#include "DB.h"

DB::DB() {
    currentID = 0;
    getCurrentID();
}

bool DB::getJsonFromID(uint32_t id, StaticJsonDocument<JSONSIZE>& doc) {
    // DynamicJsonDocument* keyBarMap = loadKeyMapping();
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
    DynamicJsonDocument* barKeyMapJson = loadBarKeyMapping();
    // read out field and add to vector
    JsonArray keys = (*barKeyMapJson)[barcode.c_str()];
    for (JsonVariant key : keys) {
        ids.push_back(key.as<uint32_t>());
    }
    (*barKeyMapJson).clear();
    delete barKeyMapJson;
    return true;
}

bool DB::add(StaticJsonDocument<1024>& doc) {
    this->currentID++;
    doc["uniqueID"] = currentID;
    String filename = String(currentID);

    doc.remove("errors");
    doc.remove("result");
    doc.remove("status");
    doc.remove("warnings");
    doc["weight"] = 0;
    File file = SD.open("test.json", FILE_WRITE);
    serializeJsonPretty(doc, file);
    Serial.write(file.read());
    file.close();
}

bool DB::set(uint32_t id, String key, String value) { return true; }

bool DB::removeJson(uint32_t id) { return true; }

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
    loadStateMapping(stateJson);
    stateJson["currentID"] = currentID;
    saveStateMapping(stateJson);
    // update bar key mapping
    DynamicJsonDocument* barKeyMapJson = loadBarKeyMapping();
    JsonArray keys = (*barKeyMapJson)[barcode.c_str()];
    //Add barcode
    keys.add(barcode);
    saveMapping(barKeyMapJson,BAR_KEYS_MAPPINGFILE);
    (*barKeyMapJson).clear();
    delete barKeyMapJson;
    // update key bar mapping
}

bool DB::loadStateMapping(StaticJsonDocument<STATEFILESIZE>& stateJson){
    File stateFile;
    stateFile = SD.open(STATEFILE, FILE_WRITE);
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

bool DB::saveStateMapping(StaticJsonDocument<STATEFILESIZE>& stateJson){
    File stateFile;
    stateFile = SD.open(STATEFILE, FILE_WRITE);
    if (!stateFile) {
        Serial.println("Failed to open state file");
        return false;
    }
    serializeJson(stateJson,stateFile);
    stateFile.close();
    return true;
}

DynamicJsonDocument* DB::loadBarKeyMapping(){
    File barKeyMap = SD.open(BAR_KEYS_MAPPINGFILE.c_str(), FILE_WRITE);
    if (!barKeyMap) {
        Serial.println("Failed to open barKeyMap file");
        return NULL;
    }
    // estimate required filesize
    long barKeyMapSize = barKeyMap.size();
    int estimateMembers = (int)barKeyMapSize / 12;
    DynamicJsonDocument* barKeyMapJson = new DynamicJsonDocument(JSON_OBJECT_SIZE(estimateMembers));
    auto error = deserializeJson(*barKeyMapJson, barKeyMap);
    if (error) {
        Serial.print(F("failed to deserialize barKeyMapping: "));
        Serial.println(error.c_str());
        return NULL;
    }
    barKeyMap.close();
    return barKeyMapJson;
}

bool DB::saveMapping(DynamicJsonDocument* doc,String mappingName){
    File mappingFile;
    mappingFile = SD.open(mappingName.c_str(), FILE_WRITE);
    if (!mappingFile) {
        Serial.println("Failed to open mapping file for saving");
        return false;
    }
    serializeJson(*doc,mappingFile);
    mappingFile.close();
    return true;
}


