/*
Mapping syntax:
UniqueID,Serial\n
UniqueID,Serial\n
*/

String MAPPINGFILENAME = "storMAP";
long uniqueID = 0;

bool initializeSD(){
  File mappingFile;
  //Extract uniqueID
  mappingFile = SD.open(MAPPINGFILENAME, FILE_WRITE);
  if(!mappingFile){
    Serial.println("Failed to open mapping file");
    return false;
  }
  long fileSize = mappingFile.size();
  if(fileSize <= 0){
    uniqueID = 0;
    return true;
  }
  char singleLine[] = "\n";
  while(mappingFile.available()){
    mappingFile.find(singleLine);
    uniqueID++;
  }
  mappingFile.close();
  return true;
}

bool storeJson(DynamicJsonDocument& doc){
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
}