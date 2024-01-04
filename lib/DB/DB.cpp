#include <DB.h>

DBClass::DBClass() {
    currentID = 0;  // reserved for product not found in get
}

bool DBClass::getJsonFromID(uint32_t id, StaticJsonDocument<JSONSIZE>& doc) {
    return loadJson(doc, String(id));
}

bool DBClass::getIDs(String barcode, std::vector<uint32_t>& ids) {
    DynamicJsonDocument barKeyMapJson = loadMapping(KEY_BAR_MAPPINGFILE);
    if (barKeyMapJson.capacity() <= 0) return false;
    // read out field and add to vector
    JsonArray keys = barKeyMapJson[barcode];
    for (JsonVariant key : keys) {
        ids.push_back(key.as<uint32_t>());
    }
    barKeyMapJson.clear();
    return true;
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
    information["name"] = doc["product_name"];
    information["brand"] = doc["brands"];
    JsonObject quantity = information.createNestedObject("quantity");
    quantity["initial"] = weight;
    quantity["remaining"] = weight;
    int productQuantity = doc["product_quantity"];
    quantity["packaging"] = weight - productQuantity;
    information["image_url"] = doc["image_url"];
    information["categories"] = doc["categories"];
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
    String path = DATA_FOLDER + "/" + String(id);
    if (!SD.remove(DATA_FOLDER + "/" + String(id))) {
        LOG(F("Failed to remove file from SD card"));
        return false;
    }
    if (!removeMappings(id, barcode)) {
        return false;
    }
    return false;
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
    DynamicJsonDocument keyMapJson = loadMapping(KEY_BAR_MAPPINGFILE);
    for (JsonPair keyValue : keyMapJson.as<JsonObject>()) {
        StaticJsonDocument<JSONSIZE> jsonSend;
        loadJson(jsonSend, String(keyValue.key().c_str()));
        WiFiService.put(jsonSend);
    }
    startEnd["sync"] = "END";
    WiFiService.put(startEnd);
    return true;
}

bool DBClass::addMappings(uint32_t pCurrentID, String barcode) {
    // update statefile
    StaticJsonDocument<STATEFILESIZE> stateJson;
    if (!loadStateMapping(stateJson)) {
        return false;
    }
    stateJson[UNIQUE_ID] = String(pCurrentID);
    if (!saveStateMapping(stateJson)) {
        return false;
    }
    // update bar key mapping
    DynamicJsonDocument barKeyMapJson = loadMapping(BAR_KEYS_MAPPINGFILE);
    if (barKeyMapJson.capacity() <= 0) {
        return false;
    }
    JsonArray keys = barKeyMapJson[barcode];
    keys.add(pCurrentID);
    if (!saveMapping(barKeyMapJson, BAR_KEYS_MAPPINGFILE)) {
        return false;
    }
    // update key bar mapping
    DynamicJsonDocument keyBarMapJson = loadMapping(KEY_BAR_MAPPINGFILE);
    if (keyBarMapJson == nullptr) {
        return false;
    }
    keyBarMapJson[String(pCurrentID)] = barcode;
    if (!saveMapping(keyBarMapJson, KEY_BAR_MAPPINGFILE)) {
        return false;
    }
    return true;
}

bool DBClass::removeMappings(uint32_t pCurrentID, String barcode) {
    // update bar key mapping
    DynamicJsonDocument barKeyMapJson = loadMapping(BAR_KEYS_MAPPINGFILE);
    if (barKeyMapJson.capacity() <= 0) {
        return false;
    }
    JsonArray keys = barKeyMapJson[barcode];
    for (JsonArray::iterator it = keys.begin(); it != keys.end(); ++it) {
        if ((*it).as<uint32_t>() == pCurrentID) {
            keys.remove(it);
        }
    }
    if (!saveMapping(barKeyMapJson, BAR_KEYS_MAPPINGFILE)) {
        return false;
    }
    // update key bar mapping
    DynamicJsonDocument keyBarMapJson = loadMapping(KEY_BAR_MAPPINGFILE);
    if (keyBarMapJson.capacity() <= 0) {
        return false;
    }
    keyBarMapJson.remove(String(pCurrentID));
    if (!saveMapping(keyBarMapJson, KEY_BAR_MAPPINGFILE)) {
        return false;
    }
    return true;
}

bool DBClass::loadJson(StaticJsonDocument<JSONSIZE>& jsonDoc, String name) {
    String path = DATA_FOLDER + "/" + name;
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
    String path = DATA_FOLDER + "/" + name;
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
    String path = STATE_FOLDER + "/" + STATEFILE;
    File stateFile = SD.open(path, FILE_READ);
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
    String path = STATE_FOLDER + "/" + STATEFILE;
    SD.remove(path);
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
    String path = INTERNAL_FOLDER + "/" + mappingfile;
    File mappingFile = SD.open(path, FILE_READ);
    if (!mappingFile) {
        LOG("loadMapping(): Failed to open mapping file");
        return DynamicJsonDocument(0);
    }
    // estimate required filesize
    uint64_t estimateMembers = mappingFile.size() / 12;
    DynamicJsonDocument mapJson(JSON_OBJECT_SIZE(1 + estimateMembers));
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
    SD.remove(mappingName);
    File mappingFile = SD.open(mappingName, FILE_WRITE);
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
    if (!checkInitialized(STATE_FOLDER+"/"+STATEFILE)) {
        if (!initializeStateFile()) {
            return false;
        }
    }
    if (!checkInitialized(INTERNAL_FOLDER + "/" + KEY_BAR_MAPPINGFILE)) {
        if (!initializeKeyBarMapping()) {
            return false;
        }
    }
    if (!checkInitialized(INTERNAL_FOLDER + "/" + BAR_KEYS_MAPPINGFILE)) {
        if (!initializeBarKeyMapping()) {
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
    if (size <= 1) {
        return false;
    }
    return true;
}

bool DBClass::initializeStateFile() {
    StaticJsonDocument<STATEFILESIZE> stateJson;
    stateJson[UNIQUE_ID] = currentID;
    if (!saveStateMapping(stateJson)) {
        LOG("Initialize state mapping failed");
        return false;
    }
    return true;
}

bool DBClass::initializeKeyBarMapping() {
    DynamicJsonDocument keyBarJson(100);
    // create empty json
    keyBarJson.to<JsonObject>();
    if (!saveMapping(keyBarJson, KEY_BAR_MAPPINGFILE)) {
        LOG("Initialize key bar mapping failed");
        return false;
    }
    return true;
}

bool DBClass::initializeBarKeyMapping() {
    DynamicJsonDocument barKeyJson(100);
    // create empty json
    barKeyJson.to<JsonObject>();
    if (!saveMapping(barKeyJson, BAR_KEYS_MAPPINGFILE)) {
        LOG("Initialize bar key mapping failed");
        return false;
    }
    return true;
}

DBClass DB;
