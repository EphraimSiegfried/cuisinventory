#pragma once

#include <Arduino.h>
#include <ArduinoJson.h>
#include <Constants.h>
#include <Debug.h>
#include <SD.h>

#include <vector>

class DBClass {
   public:
    DBClass();  // Constructor
    bool getJsonFromID(uint32_t id, StaticJsonDocument<JSONSIZE>& doc);
    bool getIDs(String barcode, std::vector<uint32_t>& ids);
    bool add(StaticJsonDocument<JSONSIZE>& doc, uint32_t weight);
    bool set(uint32_t id, String key, String value);
    bool remove(uint32_t id, String barcode);
    bool exists(String barcode);

   private:
    uint32_t currentID;
    bool getCurrentID();
    bool addMappings(u_int32_t currentID, String barcode);
    bool removeMappings(u_int32_t currentID, String barcode);
    bool loadJson(StaticJsonDocument<JSONSIZE>& jsonDoc, String name);
    bool saveJson(StaticJsonDocument<JSONSIZE>& jsonDoc, String name);
    bool loadStateMapping(StaticJsonDocument<STATEFILESIZE>& stateJson);
    bool saveStateMapping(StaticJsonDocument<STATEFILESIZE>& stateJson);
    DynamicJsonDocument* loadMapping(String mappingfile);
    bool saveMapping(DynamicJsonDocument* doc, String mappingName);
    bool initDatabase();
    bool checkInitialized(String filename);
    bool initializeStateFile();
    bool initializeKeyBarMapping();
    bool initializeBarKeyMapping();
};

extern DBClass DB;