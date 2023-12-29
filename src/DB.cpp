#include <DB.h>

DB::DB() {
    currentID = 0;
    getCurrentID();
}

bool DB::getJsonFromID(uint32_t id, StaticJsonDocument<JSONSIZE>& doc) {
    String filename = String(id);
    File jsonFile = SD.open(filename.c_str(), FILE_WRITE);
    if (!jsonFile) {
        Serial.println("Failed to open keyBar file");
        return false;
    }
    auto error = deserializeJson(doc, jsonFile);
    if (error) {
        Serial.print(F("failed to get json from id: "));
        Serial.println(error.c_str());
        return false;
    }
    jsonFile.close();
    return true;
}

bool DB::getIDs(String barcode, std::vector<uint32_t>& ids) {
    DynamicJsonDocument* barKeyMapJson = loadMapping(KEY_BAR_MAPPINGFILE);
    // read out field and add to vector
    JsonArray keys = (*barKeyMapJson)[barcode.c_str()];
    for (JsonVariant key : keys) {
        ids.push_back(key.as<uint32_t>());
    }
    (*barKeyMapJson).clear();
    delete barKeyMapJson;
    return true;
}

bool DB::add(StaticJsonDocument<1024>& doc, float weight) {
    this->currentID++;
    String barcode = doc["code"];
    doc["uniqueID"] = currentID;
    doc.remove("errors");
    doc.remove("result");
    doc.remove("status");
    doc.remove("warnings");
    doc["weight"] = weight;
    File file = SD.open(String(currentID).c_str(), FILE_WRITE);
    serializeJson(doc, file);
    file.close();
    if(!addMappings(currentID,barcode)){
        return false;
    }
    return true;
}

bool DB::set(uint32_t id, String key, String value) {
    StaticJsonDocument<JSONSIZE> jsonDoc;
    if(!loadJson(jsonDoc,String(id))){
        return false;
    }
    jsonDoc[key] = value;
    if(!saveJson(jsonDoc,String(id))){
        return false;
    }
    return true;
}

bool DB::remove(uint32_t id, String barcode) { 
    if(!SD.remove(String(id).c_str())){
        Serial.println(F("Failed to remove file from SD card"));
        return false;
    }
    if(!removeMappings(id,barcode)){
        return false;
    }
    return false;
}

bool DB::getCurrentID() {
    File stateFile;
    // open state
    stateFile = SD.open(STATEFILE, FILE_WRITE);
    if (!stateFile) {
        Serial.println("Failed to open state file");
        return false;
    }
    StaticJsonDocument<1024> doc;
    auto error = deserializeJson(doc, stateFile);
    if (error) {
        Serial.print(F("deserializeJson() failed with code "));
        Serial.println(error.c_str());
        return false;
    }
    this->currentID = doc["currentID"].as<const uint32_t>();
    stateFile.close();
    return true;
}

bool DB::addMappings(u_int32_t currentID, String barcode) {
    // update statefile
    StaticJsonDocument<STATEFILESIZE> stateJson;
    if(!loadStateMapping(stateJson)){
        return false;
    }
    stateJson["currentID"] = String(currentID);
    if(!saveStateMapping(stateJson)){
        return false;
    }
    // update bar key mapping
    DynamicJsonDocument* barKeyMapJson = loadMapping(BAR_KEYS_MAPPINGFILE);
    if(barKeyMapJson == nullptr){
        return false;
    }
    JsonArray keys = (*barKeyMapJson)[barcode.c_str()];
    keys.add(currentID);
    if(!saveMapping(barKeyMapJson, BAR_KEYS_MAPPINGFILE)){
        (*barKeyMapJson).clear();
        delete barKeyMapJson;
        return false;
    }
    (*barKeyMapJson).clear();
    delete barKeyMapJson;
    // update key bar mapping
    DynamicJsonDocument* keyBarMapJson = loadMapping(KEY_BAR_MAPPINGFILE);
    if(keyBarMapJson == nullptr){
        return false;
    }
    (*keyBarMapJson)[String(currentID)] = barcode;
    if(!saveMapping(keyBarMapJson,KEY_BAR_MAPPINGFILE)){
        (*keyBarMapJson).clear();
        delete keyBarMapJson;
        return false;
    }
    (*keyBarMapJson).clear();
    delete keyBarMapJson;
    return true;
}

bool DB::removeMappings(u_int32_t currentID, String barcode){
    // update bar key mapping
    DynamicJsonDocument* barKeyMapJson = loadMapping(BAR_KEYS_MAPPINGFILE);
    if(barKeyMapJson == nullptr){
        return false;
    }
    JsonArray keys = (*barKeyMapJson)[barcode.c_str()];
    for (JsonArray::iterator it=keys.begin(); it!=keys.end(); ++it) {
        if ((*it).as<u_int32_t>() == currentID) {
            keys.remove(it);
        }
    }
    if(!saveMapping(barKeyMapJson, BAR_KEYS_MAPPINGFILE)){
        (*barKeyMapJson).clear();
        delete barKeyMapJson;
        return false;
    }
    (*barKeyMapJson).clear();
    delete barKeyMapJson;
    // update key bar mapping
    DynamicJsonDocument* keyBarMapJson = loadMapping(KEY_BAR_MAPPINGFILE);
    if(keyBarMapJson == nullptr){
        return false;
    }
    (*keyBarMapJson).remove(String(currentID));
    if(!saveMapping(keyBarMapJson,KEY_BAR_MAPPINGFILE)){
        (*keyBarMapJson).clear();
        delete keyBarMapJson;
        return false;
    }
    (*keyBarMapJson).clear();
    delete keyBarMapJson;
    return true;
}

bool DB::loadJson(StaticJsonDocument<JSONSIZE>& jsonDoc, String name) {
    File jsonFile;
    jsonFile = SD.open(name.c_str(), FILE_WRITE);
    if (!jsonFile) {
        Serial.println("Failed to open json file");
        return false;
    }
    auto error = deserializeJson(jsonDoc, jsonFile);
    if (error) {
        Serial.print(F("deserializeJson() failed with code "));
        Serial.println(error.c_str());
        return false;
    }
    jsonFile.close();
    return true;
}

bool DB::saveJson(StaticJsonDocument<JSONSIZE>& jsonDoc, String name) {
    File jsonFile;
    jsonFile = SD.open(name.c_str(), FILE_WRITE);
    if (!jsonFile) {
        Serial.println("Failed to open json file");
        return false;
    }
    serializeJson(jsonDoc, jsonFile);
    jsonFile.close();
    return true;
}

bool DB::loadStateMapping(StaticJsonDocument<STATEFILESIZE>& stateJson) {
    File stateFile;
    stateFile = SD.open(STATEFILE.c_str(), FILE_WRITE);
    if (!stateFile) {
        Serial.println("Failed to open state file");
        return false;
    }
    auto error = deserializeJson(stateJson, stateFile);
    if (error) {
        Serial.print(F("deserializeJson() failed with code "));
        Serial.println(error.c_str());
        return false;
    }
    stateFile.close();
    return true;
}

bool DB::saveStateMapping(StaticJsonDocument<STATEFILESIZE>& stateJson) {
    File stateFile;
    stateFile = SD.open(STATEFILE.c_str(), FILE_WRITE);
    if (!stateFile) {
        Serial.println("Failed to open state file");
        return false;
    }
    serializeJson(stateJson, stateFile);
    stateFile.close();
    return true;
}

DynamicJsonDocument* DB::loadMapping(String mappingfile) {
    File mappingFile = SD.open(mappingfile.c_str(), FILE_WRITE);
    if (!mappingFile) {
        Serial.println("Failed to open mapping file");
        return nullptr;
    }
    // estimate required filesize
    long mapSize = mappingFile.size();
    int estimateMembers = (int)mapSize / 12;
    DynamicJsonDocument* mapJson =
        new DynamicJsonDocument(JSON_OBJECT_SIZE(1+estimateMembers));
    auto error = deserializeJson(*mapJson, mappingFile);
    if (error) {
        Serial.print(F("failed to deserialize mapping File: "));
        Serial.println(error.c_str());
        return nullptr;
    }
    mappingFile.close();
    return mapJson;
}

bool DB::saveMapping(DynamicJsonDocument* doc, String mappingName) {
    File mappingFile;
    mappingFile = SD.open(mappingName.c_str(), FILE_WRITE);
    if (!mappingFile) {
        Serial.println("Failed to open mapping file for saving");
        return false;
    }
    serializeJson(*doc, mappingFile);
    mappingFile.close();
    return true;
}
