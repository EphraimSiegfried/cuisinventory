#include <DB.h>

DBClass::DBClass() {
    currentID = 0;  // reserved for product not found in get
    if (!initDatabase()) {
        LOG("Initialize Database failed");
    }
    getCurrentID();
}

bool DBClass::getJsonFromID(uint32_t id, StaticJsonDocument<JSONSIZE>& doc) {
    String filename = String(id);
    File jsonFile = SD.open(filename, FILE_READ);
    if (!jsonFile) {
        LOG("Failed to open keyBar file");
        return false;
    }
    auto error = deserializeJson(doc, jsonFile);
    if (error) {
        LOG(F("failed to get json from id: "));
        LOG(error);
        return false;
    }
    jsonFile.close();
    return true;
}

bool DBClass::getIDs(String barcode, std::vector<uint32_t>& ids) {
    DynamicJsonDocument* barKeyMapJson = loadMapping(KEY_BAR_MAPPINGFILE);
    // read out field and add to vector
    JsonArray keys = (*barKeyMapJson)[barcode];
    for (JsonVariant key : keys) {
        ids.push_back(key.as<uint32_t>());
    }
    (*barKeyMapJson).clear();
    delete barKeyMapJson;
    return true;
}

bool DBClass::add(StaticJsonDocument<JSONSIZE>& doc, uint32_t weight) {
    this->currentID++;
    String barcode = doc["code"];
    doc[UNIQUE_ID] = currentID;
    doc.remove("errors");
    doc.remove("result");
    doc.remove("status");
    doc.remove("warnings");
    doc["weight"] = weight;
    if (saveJson(doc, String(currentID))) {
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

bool DBClass::remove(uint32_t id, String barcode) {
    if (!SD.remove(String(id))) {
        LOG(F("Failed to remove file from SD card"));
        return false;
    }
    if (!removeMappings(id, barcode)) {
        return false;
    }
    return false;
}

bool DBClass::getCurrentID() {
    StaticJsonDocument<1024> stateJson;
    loadStateMapping(stateJson);
    this->currentID = stateJson["currentID"].as<const uint32_t>();
    return true;
}

bool DBClass::addMappings(u_int32_t currentID, String barcode) {
    // update statefile
    StaticJsonDocument<STATEFILESIZE> stateJson;
    if (!loadStateMapping(stateJson)) {
        return false;
    }
    stateJson["currentID"] = String(currentID);
    if (!saveStateMapping(stateJson)) {
        return false;
    }
    // update bar key mapping
    DynamicJsonDocument* barKeyMapJson = loadMapping(BAR_KEYS_MAPPINGFILE);
    if (barKeyMapJson == nullptr) {
        return false;
    }
    JsonArray keys = (*barKeyMapJson)[barcode];
    keys.add(currentID);
    if (!saveMapping(barKeyMapJson, BAR_KEYS_MAPPINGFILE)) {
        (*barKeyMapJson).clear();
        delete barKeyMapJson;
        return false;
    }
    (*barKeyMapJson).clear();
    delete barKeyMapJson;
    // update key bar mapping
    DynamicJsonDocument* keyBarMapJson = loadMapping(KEY_BAR_MAPPINGFILE);
    if (keyBarMapJson == nullptr) {
        return false;
    }
    (*keyBarMapJson)[String(currentID)] = barcode;
    if (!saveMapping(keyBarMapJson, KEY_BAR_MAPPINGFILE)) {
        (*keyBarMapJson).clear();
        delete keyBarMapJson;
        return false;
    }
    (*keyBarMapJson).clear();
    delete keyBarMapJson;
    return true;
}

bool DBClass::removeMappings(u_int32_t currentID, String barcode) {
    // update bar key mapping
    DynamicJsonDocument* barKeyMapJson = loadMapping(BAR_KEYS_MAPPINGFILE);
    if (barKeyMapJson == nullptr) {
        return false;
    }
    JsonArray keys = (*barKeyMapJson)[barcode];
    for (JsonArray::iterator it = keys.begin(); it != keys.end(); ++it) {
        if ((*it).as<u_int32_t>() == currentID) {
            keys.remove(it);
        }
    }
    if (!saveMapping(barKeyMapJson, BAR_KEYS_MAPPINGFILE)) {
        (*barKeyMapJson).clear();
        delete barKeyMapJson;
        return false;
    }
    (*barKeyMapJson).clear();
    delete barKeyMapJson;
    // update key bar mapping
    DynamicJsonDocument* keyBarMapJson = loadMapping(KEY_BAR_MAPPINGFILE);
    if (keyBarMapJson == nullptr) {
        return false;
    }
    (*keyBarMapJson).remove(String(currentID));
    if (!saveMapping(keyBarMapJson, KEY_BAR_MAPPINGFILE)) {
        (*keyBarMapJson).clear();
        delete keyBarMapJson;
        return false;
    }
    (*keyBarMapJson).clear();
    delete keyBarMapJson;
    return true;
}

bool DBClass::loadJson(StaticJsonDocument<JSONSIZE>& jsonDoc, String name) {
    File jsonFile = SD.open(name, FILE_READ);
    if (!jsonFile) {
        LOG("Failed to open json file");
        return false;
    }
    auto error = deserializeJson(jsonDoc, jsonFile);
    if (error) {
        LOG(F("deserializeJson() failed with code "));
        LOG(error);
        return false;
    }
    jsonFile.close();
    return true;
}

bool DBClass::saveJson(StaticJsonDocument<JSONSIZE>& jsonDoc, String name) {
    SD.remove(name);
    File jsonFile = SD.open(name, FILE_WRITE);
    if (!jsonFile) {
        LOG("Failed to open json file");
        return false;
    }
    serializeJson(jsonDoc, jsonFile);
    jsonFile.close();
    return true;
}

bool DBClass::loadStateMapping(StaticJsonDocument<STATEFILESIZE>& stateJson) {
    File stateFile;
    stateFile = SD.open(STATEFILE, FILE_READ);
    if (!stateFile) {
        LOG("Failed to open state file");
        return false;
    }
    auto error = deserializeJson(stateJson, stateFile);
    if (error) {
        LOG(F("deserializeJson() failed with code "));
        LOG(error);
        return false;
    }
    stateFile.close();
    return true;
}

bool DBClass::saveStateMapping(StaticJsonDocument<STATEFILESIZE>& stateJson) {
    SD.remove(STATEFILE);
    File stateFile = SD.open(STATEFILE, FILE_WRITE);
    if (!stateFile) {
        LOG("Failed to open state file");
        return false;
    }
    serializeJson(stateJson, stateFile);
    stateFile.close();
    return true;
}

DynamicJsonDocument* DBClass::loadMapping(String mappingfile) {
    File mappingFile = SD.open(mappingfile, FILE_READ);
    if (!mappingFile) {
        LOG("Failed to open mapping file");
        return nullptr;
    }
    // estimate required filesize
    long mapSize = mappingFile.size();
    int estimateMembers = (int)mapSize / 12;
    DynamicJsonDocument* mapJson =
        new DynamicJsonDocument(JSON_OBJECT_SIZE(1 + estimateMembers));
    auto error = deserializeJson(*mapJson, mappingFile);
    if (error) {
        LOG(F("failed to deserialize mapping File: "));
        LOG(error);
        return nullptr;
    }
    mappingFile.close();
    return mapJson;
}

bool DBClass::saveMapping(DynamicJsonDocument* doc, String mappingName) {
    SD.remove(mappingName);
    File mappingFile = SD.open(mappingName, FILE_WRITE);
    if (!mappingFile) {
        LOG("Failed to open mapping file for saving");
        return false;
    }
    serializeJson(*doc, mappingFile);
    mappingFile.close();
    return true;
}
bool DBClass::initDatabase() {
    if (!checkInitialized(STATEFILE)) {
        if (!initializeStateFile()) {
            return false;
        }
    }
    if (!checkInitialized(KEY_BAR_MAPPINGFILE)) {
        if (!initializeKeyBarMapping()) {
            return false;
        }
    }
    if (!checkInitialized(BAR_KEYS_MAPPINGFILE)) {
        if (!initializeBarKeyMapping()) {
            return false;
        }
    }
    return true;
}

bool DBClass::checkInitialized(String filename) {
    File file;
    file = SD.open(filename, FILE_READ);
    if (!file) {
        LOG("Failed to open file for exist check");
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
        LOG("Initialize sate mapping failed");
        return false;
    }
    return true;
}

bool DBClass::initializeKeyBarMapping() {
    StaticJsonDocument<JSONSIZE> keyBarJson;
    // create empty json
    keyBarJson.to<JsonObject>();
    if (!saveJson(keyBarJson, KEY_BAR_MAPPINGFILE)) {
        LOG("Initialize key bar mapping failed");
        return false;
    }
    return true;
}

bool DBClass::initializeBarKeyMapping() {
    StaticJsonDocument<JSONSIZE> barKeyJson;
    // create empty json
    barKeyJson.to<JsonObject>();
    if (!saveJson(barKeyJson, BAR_KEYS_MAPPINGFILE)) {
        LOG("Initialize bar key mapping failed");
        return false;
    }
    return true;
}

DBClass DB;