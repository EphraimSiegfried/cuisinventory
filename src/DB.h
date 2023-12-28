#pragma once

#include <Arduino.h>
#include <ArduinoJson.h>
#include <SD.h>

#include <vector>

#include "Constants.h"

class DB {
   public:
    DB();  // Constructor
    bool getJsonFromID(uint32_t id, StaticJsonDocument<JSONSIZE>& doc);
    bool getIDs(String barcode, std::vector<uint32_t>& ids);
    bool add(StaticJsonDocument<1024>& doc);
    bool set(uint32_t id, String key, String value);
    bool removeJson(uint32_t id);

   private:
    uint32_t currentID;
    bool getCurrentID();
    bool addMappings(u_int32_t currentID, String barcode);
    bool loadStateMapping(StaticJsonDocument<STATEFILESIZE>& stateJson);
    bool saveStateMapping(StaticJsonDocument<STATEFILESIZE>& stateJson);
    DynamicJsonDocument* loadBarKeyMapping();
    bool saveMapping(DynamicJsonDocument* doc,String mappingName);
};