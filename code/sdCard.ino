
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
  File file = SD.open(filename, FILE_WRITE);
  serializeJsonPretty(doc,file);
  Serial.write(file.read());
  file.close();
  return true;
}