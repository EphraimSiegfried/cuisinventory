#pragma once

#include <Arduino.h>
#include <ArduinoJson.h>
#include <Constants.h>
#include <Debug.h>
#include <SD.h>
#include <WiFiService.h>

#include <vector>

class DBClass {
   public:
    DBClass();  // Constructor
    bool getJsonFromID(uint32_t id, StaticJsonDocument<JSONSIZE>& doc);
    bool getIDs(String barcode, std::vector<uint32_t>& ids);
    bool add(StaticJsonDocument<JSONSIZE>& doc, uint32_t weight, uint32_t time);
    bool set(uint32_t id, String key, String value);
    bool setWeight(uint32_t id, uint32_t value);
    uint32_t getLeastWeightID(String bardcode);
    bool remove(uint32_t id, String barcode);
    bool syncDB();
    bool initDatabase();
    void clearFiles(String file);

   private:
    uint32_t currentID;
    bool getCurrentID();
    bool addMappings(uint32_t currentID, String barcode);
    bool removeMappings(uint32_t currentID, String barcode);
    bool loadJson(StaticJsonDocument<JSONSIZE>& jsonDoc, String name);
    bool saveJson(StaticJsonDocument<JSONSIZE>& jsonDoc, String name);
    bool loadStateMapping(StaticJsonDocument<STATEFILESIZE>& stateJson);
    bool saveStateMapping(StaticJsonDocument<STATEFILESIZE>& stateJson);
    DynamicJsonDocument loadMapping(String mappingfile);
    bool saveMapping(DynamicJsonDocument doc, String mappingName);
    bool initializeStateFile();
    bool initializeKeyBarMapping();
    bool initializeBarKeyMapping();
    bool checkInitialized(String filename);
};

extern DBClass DB;
