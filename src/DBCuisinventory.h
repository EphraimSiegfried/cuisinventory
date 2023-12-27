#pragma once

#include "Constants.h"
#include <Arduino.h>
#include <ArduinoJson.h>
#include <SD.h>

class DBCuisinventory {
public:
  DBCuisinventory(); // Constructor
  bool getJsonFromID(uint32_t id,StaticJsonDocument<1024>& doc);
  uint32_t getID(String barcode);
  bool add(StaticJsonDocument<1024>& doc);
  bool set(uint32_t id, String key, String value);
  bool removeJson(uint32_t id);

private:
  uint32_t currentID;
  bool getCurrentID();
  DynamicJsonDocument* loadKeyMapping();

};