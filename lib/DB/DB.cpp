#include <DB.h>
#ifdef __arm__
// should use uinstd.h to define sbrk but Due causes a conflict
extern "C" char* sbrk(int incr);
#else   // __ARM__
extern char* __brkval;
#endif  // __arm__

int freeMemory() {
    char top;
#ifdef __arm__
    return &top - reinterpret_cast<char*>(sbrk(0));
#elif defined(CORE_TEENSY) || (ARDUINO > 103 && ARDUINO != 151)
    return &top - __brkval;
#else   // __arm__
    return __brkval ? &top - __brkval : &top - __malloc_heap_start;
#endif  // __arm__
}

bool fileExists(String path) {
    File file;
    bool exists = SD.open(path, O_RDONLY);
    if (exists) file.close();
    return exists;
}

DBClass::DBClass() {
    currentID = 0;  // reserved for product not found in get
}

bool DBClass::getJsonFromID(uint32_t id, JsonDocument& doc) {
    return loadJson(doc,INTERNAL_FOLDER, String(id).c_str());
}

bool DBClass::getIDs(String barcode, std::vector<uint32_t>& ids) {
    JsonDocument barIdMapJson;
    if(!loadJson(barIdMapJson,INTERNAL_FOLDER,BAR_ID_MAPPINGFILE)){
        return false;
    }
    // read out field and add to vector
    JsonArray productIds = barIdMapJson[barcode];
    for (JsonVariant id : productIds) {
        ids.push_back(id.as<uint32_t>());
    }
    barIdMapJson.clear();
    return true;
}

std::vector<uint32_t> DBClass::getAllIDs() {
    std::vector<uint32_t> ids;
    JsonDocument idBarMapJson;
    if(!loadJson(idBarMapJson,INTERNAL_FOLDER,ID_BAR_MAPPINGFILE)){
        return ids;
    }
    JsonObject obj = idBarMapJson.as<JsonObject>();
    for (JsonPair kv : obj) {
        const char* keyStr = kv.key().c_str();
        uint32_t id = static_cast<uint32_t>(atol(keyStr));
        ids.push_back(id);
    }
    idBarMapJson.clear();
    return ids;
}

bool DBClass::add(JsonDocument& doc, uint32_t weight, uint32_t time) {
    this->currentID++;
    JsonDocument formattedJson;
    JsonObject information = formattedJson.to<JsonObject>();
    information[UNIQUE_ID] = currentID;
    String barcode = doc["code"];
    information["code"] = barcode;
    information["date"] = time;
    information["name"] = doc["product"]["product_name"];
    information["brand"] = doc["product"]["brands"];
    //JsonObject quantity = information.createNestedObject("quantity");
    JsonObject quantity = information["quantity"].to<JsonObject>();
    quantity["initial"] = weight;
    quantity["remaining"] = weight;
    int productQuantity = doc["product"]["product_quantity"];
    quantity["packaging"] = weight - productQuantity;
    information["image_url"] = doc["product"]["image_url"];
    information["categories"] = doc["product"]["categories"];
    LOG("free mem:");
    LOG(String(freeMemory()));
    doc.clear();
    if (!saveJson(formattedJson, DATA_FOLDER,String(currentID).c_str())) {
        return false;
    }
    LOG(F("free mem:"));
    LOG(String(freeMemory()));
    if (!addMappings(currentID, barcode)) {
        return false;
    }
    return true;
}

bool DBClass::set(uint32_t id, String key, String value) {
    JsonDocument jsonDoc;
    if (!loadJson(jsonDoc, DATA_FOLDER, String(id).c_str())) {
        return false;
    }
    jsonDoc[key] = value;
    if (!saveJson(jsonDoc, DATA_FOLDER, String(id).c_str())) {
        return false;
    }
    return true;
}

bool DBClass::setWeight(uint32_t id, uint32_t value) {
    JsonDocument jsonDoc;
    if (!loadJson(jsonDoc, DATA_FOLDER, String(id).c_str())) {
        return false;
    }
    jsonDoc["quantity"]["remaining"] = String(value);
    if (!saveJson(jsonDoc, DATA_FOLDER, String(id).c_str())) {
        return false;
    }
    return true;
}

bool DBClass::remove(uint32_t id, String barcode) {
    String path = "";
    path = path + DATA_FOLDER + "/" + String(id);
    if (!SD.remove(path)) {
        LOG(F("Failed to remove file from SD card"));
        return false;
    }
    if (!removeMappings(id, barcode)) {
        return false;
    }
    return true;
}

bool DBClass::getCurrentID() {
    JsonDocument stateJson;
    if (!loadJson(stateJson,STATE_FOLDER,STATEFILE)){
        return false;
    }
    this->currentID = stateJson[UNIQUE_ID].as<const uint32_t>();
    return true;
}

uint32_t DBClass::getLeastWeightID(String barcode) {
    std::vector<uint32_t> ids;
    uint32_t leastWeightId = 0;
    int leastWeight = 999999;
    getIDs(barcode, ids);
    for (uint32_t id : ids) {
        JsonDocument doc;
        loadJson(doc, DATA_FOLDER, String(id).c_str());
        int weight = doc["quantity"]["remaining"];
        if (weight < leastWeight) {
            leastWeight = weight;
            leastWeightId = id;
        }
    }
    return leastWeightId;
}

bool DBClass::syncDB() {
    JsonDocument startEnd;
    startEnd["sync"] = "BEGIN";
    WiFiService.put(startEnd);
    JsonDocument idBarJson;
    if(!loadJson(idBarJson,INTERNAL_FOLDER,ID_BAR_MAPPINGFILE)){
        return false;
    }
    for (JsonPair keyValue : idBarJson.as<JsonObject>()) {
        JsonDocument jsonSend;
        loadJson(jsonSend, DATA_FOLDER, keyValue.key().c_str());
        WiFiService.put(jsonSend);
    }
    startEnd["sync"] = "END";
    WiFiService.put(startEnd);
    return true;
}

bool DBClass::addMappings(uint32_t id, String barcode) {
    // update statefile
    JsonDocument stateJson;
    if (!loadJson(stateJson,STATE_FOLDER,STATEFILE)) {
        return false;
    }
    stateJson[UNIQUE_ID] = String(id);
    if (!saveJson(stateJson,STATE_FOLDER,STATEFILE)) {
        return false;
    }
    // update bar id mapping
    JsonDocument barIdMapJson;
    if (!loadJson(barIdMapJson,INTERNAL_FOLDER,BAR_ID_MAPPINGFILE)) {
        return false;
    }
    if (!barIdMapJson.containsKey(barcode)) {
        //barIdMapJson.createNestedArray(barcode);
        barIdMapJson[barcode].to<JsonArray>();
    }
    JsonArray ids = barIdMapJson[barcode];
    ids.add(id);
    if (!saveJson(barIdMapJson,INTERNAL_FOLDER, BAR_ID_MAPPINGFILE)) {
        return false;
    }
    barIdMapJson.clear();
    // update id bar mapping
    JsonDocument idBarJson;
    if (!loadJson(idBarJson,INTERNAL_FOLDER,ID_BAR_MAPPINGFILE)) {
        return false;
    }
    idBarJson[String(id)] = barcode;
    if (!saveJson(idBarJson,INTERNAL_FOLDER, ID_BAR_MAPPINGFILE)) {
        return false;
    }
    return true;
}

bool DBClass::removeMappings(uint32_t id, String barcode) {
    // update bar id mapping
    JsonDocument barIdMapJson;
    if (!loadJson(barIdMapJson,INTERNAL_FOLDER,BAR_ID_MAPPINGFILE)) {
        return false;
    }
    if (!barIdMapJson.containsKey(barcode)) {
        LOG(F("Barcode doesn't exist"));
        return false;
    }
    JsonArray ids = barIdMapJson[barcode];
    for (JsonArray::iterator it = ids.begin(); it != ids.end(); ++it) {
        if ((*it).as<uint32_t>() == id) {
            ids.remove(it);
            LOG("Removed id: " + String(id));
        }
    }
    if (!saveJson(barIdMapJson,INTERNAL_FOLDER,BAR_ID_MAPPINGFILE)) {
        return false;
    }
    barIdMapJson.clear();
    // update id bar mapping
    JsonDocument idBarJson;
    if (!loadJson(idBarJson,INTERNAL_FOLDER,ID_BAR_MAPPINGFILE)) {
        return false;
    }
    idBarJson.remove(String(id));
    if (!saveJson(idBarJson, INTERNAL_FOLDER, ID_BAR_MAPPINGFILE)) {
        return false;
    }
    return true;
}

bool DBClass::loadJson(JsonDocument& jsonDoc, const char folder[], const char name[]) {
    size_t totalLength = snprintf(NULL, 0, "%s%s%s", folder,"/", name);
    char path[totalLength + 1];  // +1 for the null terminator
    snprintf(path, sizeof(path), "%s%s%s", folder,"/", name);
    LOG(path);
    LOG("free mem:");
    LOG(String(freeMemory()));
    File jsonFile = SD.open(path, FILE_READ);
    if (!jsonFile) {
        LOG(F("loadJson(): Failed to open json file with path:"));
        LOG(path);
        return false;
    }
    auto error = deserializeJson(jsonDoc, jsonFile);
    if (error) {
        LOG(F("loadJson(): deserializeJson() failed with code "));
        LOG(error.c_str());
        return false;
    }
    jsonFile.close();
    return true;
}

bool DBClass::saveJson(JsonDocument& jsonDoc, const char folder[], const char name[]) {
    size_t totalLength = snprintf(NULL, 0, "%s%s%s", folder,"/", name);
    char path[totalLength + 1];  // +1 for the null terminator
    snprintf(path, sizeof(path), "%s%s%s", folder,"/", name);
    LOG(path);
    SD.remove(path);
    File jsonFile = SD.open(path, FILE_WRITE);
    if (!jsonFile) {
        LOG(F("saveJson(): Failed to open json file with path:"));
        LOG(path);
        return false;
    }
    serializeJson(jsonDoc, jsonFile);
    jsonFile.close();
    return true;
}

bool DBClass::initDatabase() {
    LOG("free mem:");
    LOG(String(freeMemory()));
    if (!fileExists(STATE_FOLDER)) {
        SD.mkdir(STATE_FOLDER);
    }
    LOG("free mem:");
    LOG(String(freeMemory()));
    if (!fileExists(INTERNAL_FOLDER)) {
        SD.mkdir(INTERNAL_FOLDER);
    }
    LOG("free mem:");
    LOG(String(freeMemory()));
    if (!fileExists(DATA_FOLDER)) {
        SD.mkdir(DATA_FOLDER);
    }
    LOG("free mem:");
    LOG(String(freeMemory()));
    String path = "";
    path = path + STATE_FOLDER + "/" + STATEFILE;
    if (!checkInitialized(path)) {
        if (!initializeStateFile()) {
            return false;
        }
    }
    path = "";
    path = path + INTERNAL_FOLDER + "/" + ID_BAR_MAPPINGFILE;
    if (!checkInitialized(path)) {
        if (!initializeIdBarMapping()) {
            return false;
        }
    }
    path = "";
    path = path + INTERNAL_FOLDER + "/" + BAR_ID_MAPPINGFILE;
    if (!checkInitialized(path)) {
        if (!initializeBarIdMapping()) {
            return false;
        }
    }
    if (!getCurrentID()) {
        return false;
    }
    return true;
}

bool DBClass::checkInitialized(String filename) {
    File file = SD.open(filename, FILE_READ);
    if (!file) {
        return false;
    }
    long size = file.size();
    file.close();
    if (size <= 1) {
        return false;
    }
    return true;
}

bool DBClass::initializeStateFile() {
    JsonDocument stateJson;
    stateJson[UNIQUE_ID] = 0;
    if (!saveJson(stateJson,STATE_FOLDER,STATEFILE)) {
        LOG(F("Initialize state mapping failed"));
        return false;
    }
    return true;
}

bool DBClass::initializeIdBarMapping() {
    JsonDocument IdBarJson;
    // create empty json
    IdBarJson.to<JsonObject>();
    if (!saveJson(IdBarJson,INTERNAL_FOLDER, ID_BAR_MAPPINGFILE)) {
        LOG(F("Initialize id bar mapping failed"));
        return false;
    }
    return true;
}

bool DBClass::initializeBarIdMapping() {
    JsonDocument barIdJson;
    // create empty json
    barIdJson.to<JsonObject>();
    if (!saveJson(barIdJson, INTERNAL_FOLDER, BAR_ID_MAPPINGFILE)) {
        LOG(F("Initialize bar id mapping failed"));
        return false;
    }
    return true;
}

void DBClass::clear() {
    File dir = SD.open(INTERNAL_FOLDER);
    dir.rmRfStar();
    dir.close();
    dir = SD.open(DATA_FOLDER);
    dir.rmRfStar();
    dir.close();
    dir = SD.open(STATE_FOLDER);
    dir.rmRfStar();
    dir.close();
}

SdFat SD;
DBClass DB;