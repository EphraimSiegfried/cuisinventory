#include "DBCuisinventory.h"

DBCuisinventory::DBCuisinventory() {
    currentID = 0;
    getCurrentID();
}

bool DBCuisinventory::getJsonFromID(uint32_t id,StaticJsonDocument<1024>& doc){
  //DynamicJsonDocument* keyBarMap = loadKeyMapping();
  String filename = String(id);
  File file = SD.open(filename.c_str(), FILE_WRITE);
  if(!file){
    Serial.println("Failed to open keyBar file");
    return false;
  }
  return true;
}

uint32_t DBCuisinventory::getID(String barcode){
  return 0;
}

bool DBCuisinventory::add(StaticJsonDocument<1024>& doc){
  return true;
}

bool DBCuisinventory::set(uint32_t id, String key, String value){
  return true;
}

bool DBCuisinventory::removeJson(uint32_t id){
  return true;
}

bool DBCuisinventory::getCurrentID(){
  File stateFile;
  //open state
  stateFile = SD.open(STATEFILE, FILE_WRITE);
  if(!stateFile){
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
DynamicJsonDocument* DBCuisinventory::loadKeyMapping(){
    File keyBarMap;
    keyBarMap = SD.open(KEY_BAR_MAPPINGFILE, FILE_WRITE);
    long keyBarMapSize = keyBarMap.size();
    keyBarMap.close();
    int estimateMembers = (int) keyBarMapSize/12;
    DynamicJsonDocument* keyBarMapJson = new DynamicJsonDocument(JSON_OBJECT_SIZE(estimateMembers));
    return keyBarMapJson;
}

/*bool storeJson(DynamicJsonDocument& doc){
  const char* filename = doc["code"].as<const char*>();
  if(SD.exists(filename)){
    return true;
  }
  doc.remove("errors");
  doc.remove("result");
  doc.remove("status");
  doc.remove("warnings");
  doc["weight"] = 0;
  File file = SD.open("test.json", FILE_WRITE);
  serializeJsonPretty(doc,file);
  Serial.write(file.read());
  file.close();
  return true;
}*/

