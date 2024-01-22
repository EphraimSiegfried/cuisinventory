#include <DB.h>

DBClass::DBClass() {
    currentID = 0;  // reserved for product not found in get
}

bool DBClass::getJsonFromID(uint32_t id, StaticJsonDocument<JSONSIZE>& doc) {
    return loadJson(doc, String(id));
}

bool DBClass::getIDs(String barcode, std::vector<uint32_t>& ids) {
    DynamicJsonDocument barIdMapJson = loadMapping(BAR_ID_MAPPINGFILE);
    if (barIdMapJson.capacity() <= 0) return false;
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
    DynamicJsonDocument idBarMapJson = loadMapping(ID_BAR_MAPPINGFILE);
    JsonObject obj = idBarMapJson.as<JsonObject>();
    for (JsonPair kv : obj) {
        const char* keyStr = kv.key().c_str();
        uint32_t id = static_cast<uint32_t>(atol(keyStr));
        ids.push_back(id);
    }
    idBarMapJson.clear();
    return ids;
}

bool DBClass::add(StaticJsonDocument<JSONSIZE>& doc, uint32_t weight,
                  uint32_t time) {
    this->currentID++;
    StaticJsonDocument<JSONSIZE> formattedJson;
    JsonObject information = formattedJson.to<JsonObject>();
    information[UNIQUE_ID] = currentID;
    String barcode = doc["code"];
    information["code"] = barcode;
    information["date"] = time;
    information["name"] = doc["product"]["product_name"];
    information["brand"] = doc["product"]["brands"];
    JsonObject quantity = information.createNestedObject("quantity");
    quantity["initial"] = weight;
    quantity["remaining"] = weight;
    int productQuantity = doc["product"]["product_quantity"];
    quantity["packaging"] = weight - productQuantity;
    information["image_url"] = doc["product"]["image_url"];
    information["categories"] = doc["product"]["categories"];
    if (!saveJson(formattedJson, String(currentID))) {
        return false;
    }
    if (!addMappings(currentID, barcode)) {
        return false;
    }
    return true;
}

bool DBClass::set(uint32_t id, String key, String value) {
    StaticJsonDocument<JSONSIZE> jsonDoc;
    if (!loadJson(jsonDoc, String(id))) {
        return false;
    }
    jsonDoc[key] = value;
    if (!saveJson(jsonDoc, String(id))) {
        return false;
    }
    return true;
}

bool DBClass::setWeight(uint32_t id, uint32_t value) {
    StaticJsonDocument<JSONSIZE> jsonDoc;
    if (!loadJson(jsonDoc, String(id))) {
        return false;
    }
    jsonDoc["quantity"]["remaining"] = String(value);
    if (!saveJson(jsonDoc, String(id))) {
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
    StaticJsonDocument<STATEFILESIZE> stateJson;
    if (loadStateMapping(stateJson))
        this->currentID = stateJson[UNIQUE_ID].as<const uint32_t>();
    return true;
}

uint32_t DBClass::getLeastWeightID(String barcode) {
    std::vector<uint32_t> ids;
    uint32_t leastWeightId = 0;
    int leastWeight = 999999;
    getIDs(barcode, ids);
    for (uint32_t id : ids) {
        StaticJsonDocument<JSONSIZE> doc;
        loadJson(doc, String(id));
        int weight = doc["quantity"]["remaining"];
        if (weight < leastWeight) {
            leastWeight = weight;
            leastWeightId = id;
        }
    }
    return leastWeightId;
}

bool DBClass::syncDB() {
    StaticJsonDocument<JSONSIZE> startEnd;
    startEnd["sync"] = "BEGIN";
    WiFiService.put(startEnd);
    DynamicJsonDocument idBarJson = loadMapping(ID_BAR_MAPPINGFILE);
    for (JsonPair keyValue : idBarJson.as<JsonObject>()) {
        StaticJsonDocument<JSONSIZE> jsonSend;
        loadJson(jsonSend, String(keyValue.key().c_str()));
        WiFiService.put(jsonSend);
    }
    startEnd["sync"] = "END";
    WiFiService.put(startEnd);
    return true;
}

bool DBClass::addMappings(uint32_t id, String barcode) {
    // update statefile
    StaticJsonDocument<STATEFILESIZE> stateJson;
    if (!loadStateMapping(stateJson)) {
        return false;
    }
    stateJson[UNIQUE_ID] = String(id);
    if (!saveStateMapping(stateJson)) {
        return false;
    }
    // update bar id mapping
    DynamicJsonDocument barIdMapJson = loadMapping(BAR_ID_MAPPINGFILE);
    if (barIdMapJson.capacity() <= 0) {
        return false;
    }
    if (!barIdMapJson.containsKey(barcode)) {
        /*StaticJsonDocument<1> doc;
        JsonArray array = doc.to<JsonArray>();*/
        barIdMapJson.createNestedArray(barcode);
    }
    JsonArray ids = barIdMapJson[barcode];
    ids.add(id);
    if (!saveMapping(barIdMapJson, BAR_ID_MAPPINGFILE)) {
        return false;
    }
    // update id bar mapping
    DynamicJsonDocument idBarJson = loadMapping(ID_BAR_MAPPINGFILE);
    if (idBarJson.capacity() <= 0) {
        return false;
    }
    idBarJson[String(id)] = barcode;
    if (!saveMapping(idBarJson, ID_BAR_MAPPINGFILE)) {
        return false;
    }
    return true;
}

bool DBClass::removeMappings(uint32_t id, String barcode) {
    // update bar id mapping
    DynamicJsonDocument barIdMapJson = loadMapping(BAR_ID_MAPPINGFILE);
    if (barIdMapJson.capacity() <= 0) {
        return false;
    }
    if (!barIdMapJson.containsKey(barcode)) {
        LOG("Barcode doesn't exist");
        return false;
    }
    JsonArray ids = barIdMapJson[barcode];
    for (JsonArray::iterator it = ids.begin(); it != ids.end(); ++it) {
        if ((*it).as<uint32_t>() == id) {
            ids.remove(it);
        }
    }
    if (!saveMapping(barIdMapJson, BAR_ID_MAPPINGFILE)) {
        return false;
    }
    // update id bar mapping
    DynamicJsonDocument idBarJson = loadMapping(ID_BAR_MAPPINGFILE);
    if (idBarJson.capacity() <= 0) {
        return false;
    }
    idBarJson.remove(String(id));
    if (!saveMapping(idBarJson, ID_BAR_MAPPINGFILE)) {
        return false;
    }
    return true;
}

bool DBClass::loadJson(StaticJsonDocument<JSONSIZE>& jsonDoc, String name) {
    String path = "";
    path = path + DATA_FOLDER + "/" + name;
    File jsonFile = SD.open(path, FILE_READ);
    if (!jsonFile) {
        LOG("loadJson(): Failed to open json file");
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

bool DBClass::saveJson(StaticJsonDocument<JSONSIZE>& jsonDoc, String name) {
    String path = "";
    path = path + DATA_FOLDER + "/" + name;
    LOG(path);
    SD.remove(path);
    File jsonFile = SD.open(path, FILE_WRITE);
    if (!jsonFile) {
        LOG("saveJson(): Failed to open json file");
        return false;
    }
    serializeJson(jsonDoc, jsonFile);
    jsonFile.close();
    return true;
}

bool DBClass::loadStateMapping(StaticJsonDocument<STATEFILESIZE>& stateJson) {
    LOG(STATEFILE);
    String path = "";
    path = path + STATE_FOLDER + "/state";
    LOG(STATE_FOLDER);
    LOG(STATEFILE);
    /*if (!SD.exists(path)) {
        LOG(path + "doesnt exist");
    }*/
    LOG("test");
    File stateFile = SD.open(path, FILE_READ);
    LOG("test2");
    if (!stateFile) {
        LOG("loadStateMapping(): Failed to open state file");
        return false;
    }
    auto error = deserializeJson(stateJson, stateFile);
    if (error) {
        LOG(F("loadStateMapping(): deserializeJson() failed with code "));
        LOG(error.c_str());
        return false;
    }
    stateFile.close();
    return true;
}

bool DBClass::saveStateMapping(StaticJsonDocument<STATEFILESIZE>& stateJson) {
    String path = "";
    path = path + STATE_FOLDER + "/state";
    LOG(STATE_FOLDER);
    LOG(STATEFILE);
    if (!SD.exists(path)) {
        LOG(path + "doesnt exist");
    }
    if (!SD.remove(path)) {
        LOG("failed remove");
    }
    File stateFile = SD.open(path, FILE_WRITE);
    if (!stateFile) {
        LOG("saveStateMapping(): Failed to open state file");
        return false;
    }
    serializeJson(stateJson, stateFile);
    stateFile.close();
    return true;
}

DynamicJsonDocument DBClass::loadMapping(String mappingfile) {
    String path = "";
    path = path + INTERNAL_FOLDER + "/" + mappingfile;
    File mappingFile = SD.open(path, FILE_READ);
    if (!mappingFile) {
        LOG("loadMapping(): Failed to open mapping file");
        return DynamicJsonDocument(0);
    }
    // estimate required filesize
    uint64_t estimateMembers = mappingFile.size() / 12;
    DynamicJsonDocument mapJson(200 + JSON_OBJECT_SIZE(1 + estimateMembers));
    auto error = deserializeJson(mapJson, mappingFile);
    if (error) {
        LOG(F("loadMapping(): deserializeJson() failed with code "));
        LOG(error.c_str());
        return DynamicJsonDocument(0);
    }
    mappingFile.close();
    return mapJson;
}

bool DBClass::saveMapping(DynamicJsonDocument doc, String mappingName) {
    String path = "";
    path = path + INTERNAL_FOLDER + "/" + mappingName;
    SD.remove(path);
    File mappingFile = SD.open(path, FILE_WRITE);
    if (!mappingFile) {
        LOG("saveMapping(): Failed to open mapping file");
        return false;
    }
    serializeJson(doc, mappingFile);
    mappingFile.close();
    return true;
}

bool DBClass::initDatabase() {
    if (!SD.exists(STATE_FOLDER)) {
        SD.mkdir(STATE_FOLDER);
    }
    if (!SD.exists(INTERNAL_FOLDER)) {
        SD.mkdir(INTERNAL_FOLDER);
    }
    if (!SD.exists(DATA_FOLDER)) {
        SD.mkdir(DATA_FOLDER);
    }
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
    StaticJsonDocument<STATEFILESIZE> stateJson;
    stateJson[UNIQUE_ID] = 0;
    if (!saveStateMapping(stateJson)) {
        LOG("Initialize state mapping failed");
        return false;
    }
    return true;
}

bool DBClass::initializeIdBarMapping() {
    DynamicJsonDocument IdBarJson(100);
    // create empty json
    IdBarJson.to<JsonObject>();
    if (!saveMapping(IdBarJson, ID_BAR_MAPPINGFILE)) {
        LOG("Initialize id bar mapping failed");
        return false;
    }
    return true;
}

bool DBClass::initializeBarIdMapping() {
    DynamicJsonDocument barIdJson(100);
    // create empty json
    barIdJson.to<JsonObject>();
    if (!saveMapping(barIdJson, BAR_ID_MAPPINGFILE)) {
        LOG("Initialize bar id mapping failed");
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
