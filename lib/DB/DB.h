#pragma once

#include <Arduino.h>
#include <ArduinoJson.h>
#include <Constants.h>
#include <Debug.h>
#include <SdFat.h>
#include <WiFiService.h>

#include <vector>

class DBClass {
   public:
    DBClass();  // Constructor
    bool getJsonFromID(uint32_t id, JsonDocument& doc);
    bool getIDs(String barcode, std::vector<uint32_t>& ids);
    std::vector<uint32_t> getAllIDs();
    bool add(JsonDocument& doc, uint32_t weight, uint32_t time);
    bool set(uint32_t id, String key, String value);
    bool setWeight(uint32_t id, uint32_t value);
    uint32_t getLeastWeightID(String bardcode);
    bool remove(uint32_t id, String barcode);
    bool syncDB();
    bool initDatabase();
    void clear();

   private:
    uint32_t currentID;
    bool getCurrentID();
    bool addMappings(uint32_t currentID, String barcode);
    bool removeMappings(uint32_t currentID, String barcode);
    bool loadJson(JsonDocument& jsonDoc, const char folder[],
                  const char name[]);
    bool saveJson(JsonDocument& jsonDoc, const char folder[],
                  const char name[]);
    bool initializeStateFile();
    bool initializeIdBarMapping();
    bool initializeBarIdMapping();
    bool checkInitialized(String filename);
};

extern DBClass DB;
extern SdFat SD;
