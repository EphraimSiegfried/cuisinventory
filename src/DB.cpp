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
    // Open BarKey Mappingfile
    File barKeyMap = SD.open(BAR_KEYS_MAPPINGFILE.c_str(), FILE_WRITE);
    if (!barKeyMap) {
        Serial.println("Failed to open barKeyMap file");
        return false;
    }
    // estimate required filesize
    long keyBarMapSize = barKeyMap.size();
    int estimateMembers = (int)keyBarMapSize / 12;
    DynamicJsonDocument barKeyMapJson(JSON_OBJECT_SIZE(estimateMembers));
    auto error = deserializeJson(barKeyMapJson, barKeyMap);
    if (error) {
        Serial.print(F("failed to deserialize barKeyMapping: "));
        Serial.println(error.c_str());
        return false;
    }
    barKeyMap.close();
    // read out field and add to vector
    JsonArray keys = barKeyMapJson[barcode.c_str()];
    for (JsonVariant key : keys) {
        ids.push_back(key.as<uint32_t>());
    }
    barKeyMapJson.clear();
    return true;
}

bool DB::add(StaticJsonDocument<1024>& doc) {
    this->currentID++;
    doc["uniqueID"] = currentID;
    String filename = String(currentID);

    if (SD.exists(filename)) {
        return true;
    }
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

bool addMappings(u_int32_t currentID, String barcode) {
    // update statefile
    File stateFile;
    stateFile = SD.open(STATEFILE, FILE_WRITE);
    if (!stateFile) {
        Serial.println("Failed to open state file");
        return false;
    }
    StaticJsonDocument<STATEFILESIZE> stateJson;
    auto error = deserializeJson(stateJson, stateFile);
    if (error) {
        Serial.print(F("deserializeJson() failed with code "));
        Serial.println(error.c_str());
        return false;
    }
    stateFile.close();
    stateJson['currentID'] = currentID;
    // Save statefile
    const char* filename = String(currentID).c_str();
    File file = SD.open(filename, FILE_WRITE);
    serializeJson(stateJson, file);
    file.close();
}

/*DynamicJsonDocument* DB::loadKeyMapping(){
    File keyBarMap = SD.open(KEY_BAR_MAPPINGFILE.c_str(), FILE_WRITE);
    long keyBarMapSize = keyBarMap.size();
    int estimateMembers = (int) keyBarMapSize/12;
    DynamicJsonDocument keyBarMapJson(JSON_OBJECT_SIZE(estimateMembers));
    auto error = deserializeJson(keyBarMapJson,keyBarMap);
    if (error) {
      Serial.print(F("failed to deserialize keyBarMapping: "));
      Serial.println(error.c_str());
      return keyBarMapJson;
    }
    keyBarMap.close();
    return keyBarMapJson;
}*/

bool storeJson(Json doc) {
    const char* filename = doc["uniqueID"].as<const char*>();
    File file = SD.open(filename, FILE_WRITE);
    serializeJson(doc, file);
    file.close();
    return true;
}
