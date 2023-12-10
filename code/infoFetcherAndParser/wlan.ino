
#if defined(ADAFRUIT_FEATHER_M4_EXPRESS) || \
  defined(ADAFRUIT_FEATHER_M0_EXPRESS) || \
  defined(ADAFRUIT_FEATHER_M0) || \
  defined(ARDUINO_AVR_FEATHER32U4) || \
  defined(ARDUINO_NRF52840_FEATHER) || \
  defined(ADAFRUIT_ITSYBITSY_M0) || \
  defined(ADAFRUIT_ITSYBITSY_M4_EXPRESS) || \
  defined(ARDUINO_AVR_ITSYBITSY32U4_3V) || \
  defined(ARDUINO_NRF52_ITSYBITSY)
  // Configure the pins used for the ESP32 connection
  #define SPIWIFI       SPI  // The SPI port
  #define SPIWIFI_SS    13   // Chip select pin
  #define ESP32_RESETN  12   // Reset pin
  #define SPIWIFI_ACK   11   // a.k.a BUSY or READY pin
  #define ESP32_GPIO0   -1
#elif defined(ARDUINO_AVR_FEATHER328P)
  #define SPIWIFI       SPI  // The SPI port
  #define SPIWIFI_SS     4   // Chip select pin
  #define ESP32_RESETN   3   // Reset pin
  #define SPIWIFI_ACK    2   // a.k.a BUSY or READY pin
  #define ESP32_GPIO0   -1
#elif defined(TEENSYDUINO)
  #define SPIWIFI       SPI  // The SPI port
  #define SPIWIFI_SS     5   // Chip select pin
  #define ESP32_RESETN   6   // Reset pin
  #define SPIWIFI_ACK    9   // a.k.a BUSY or READY pin
  #define ESP32_GPIO0   -1
#elif defined(ARDUINO_NRF52832_FEATHER )
  #define SPIWIFI       SPI  // The SPI port
  #define SPIWIFI_SS    16  // Chip select pin
  #define ESP32_RESETN  15  // Reset pin
  #define SPIWIFI_ACK    7  // a.k.a BUSY or READY pin
  #define ESP32_GPIO0   -1
#elif !defined(SPIWIFI_SS)  // if the wifi definition isnt in the board variant
  // Don't change the names of these #define's! they match the variant ones
  #define SPIWIFI       SPI
  #define SPIWIFI_SS    10   // Chip select pin
  #define SPIWIFI_ACK    7   // a.k.a BUSY or READY pin
  #define ESP32_RESETN   5   // Reset pin
  #define ESP32_GPIO0   -1   // Not connected
#endif

//Wifi Client 
WiFiSSLClient client;
 // the Wifi radio's status
int wifiStatus = WL_IDLE_STATUS;
//bool storeJson(DynamicJsonDocument& doc);
/**
Attempts to connect to wifi
*/
bool connectWlan(String p_ssid,String p_pass){
  const char* ssid = p_ssid.c_str();
  const char* pass = p_pass.c_str();
  WiFi.setPins(SPIWIFI_SS, SPIWIFI_ACK, ESP32_RESETN, ESP32_GPIO0, &SPIWIFI);
  // check for the WiFi module:
  if (WiFi.status() == WL_NO_MODULE) {
    Serial.println("Communication with WiFi module failed!");
    return false;
  }
  // attempt to connect to Wifi network:
  Serial.print("Attempting to connect to WPA SSID: ");
  Serial.println(ssid);
  // Connect to WPA/WPA2 network:
  wifiStatus = WiFi.begin(ssid, pass);
  // wait 10 seconds for connection:
  delay(3000);
  if(wifiStatus != WL_CONNECTED){
    return false;
  }
  return true;
}

void printWifiStatus() {
  // print the SSID of the network you're attached to:
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print your board's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  // print the received signal strength:
  long rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");
}

bool fetchData(String p_server,String p_apiPath, String p_barcode, String p_fields){
  const char* server = p_server.c_str();
  const char* apiPath = p_apiPath.c_str();
  const char* barcode = p_barcode.c_str();
  const char* fields = p_fields.c_str();

   // if you get a connection, report back via serial:
  if (!client.connect(server, 443)) {
    Serial.println(F("Connection to server failed"));
    return false;
  }
  Serial.println(F("Connected to server"));
  // Make a HTTP request:
  client.println("GET "+ p_apiPath + p_barcode + "?fields="+ p_fields + " HTTP/1.1");
  client.println("Host: "+ p_server);
  client.println("User-Agent: Cuisinventory/1.0 alexander.lutsch@stud.unibas.ch");
  client.println("Connection: close");
  client.println();


  // Check HTTP status
  char status[32] = {0};
  client.readBytesUntil('\r', status, sizeof(status));
  if (strcmp(status, "HTTP/1.1 200 OK") != 0) {
    Serial.print(F("Unexpected response: "));
    Serial.println(status);
    return false;
  }

  char singelLine[] = "\n";
  char doubleLine[] = "\r\n\r\n";
  // Skip headers
  client.find(doubleLine, 4);
  //skip first line
  client.find(singelLine,1);

  DynamicJsonDocument doc(1024);
  auto error = deserializeJson(doc, client);
  if (error) {
    Serial.print(F("deserializeJson() failed with code "));
    Serial.println(error.c_str());
    return false;
  }
  if(strcmp(doc["status"].as<const char*>(),"success") != 0){
    Serial.println(F("Couldn't find product"));
    return false;
  }
  Serial.println(doc["product"]["product_name"].as<const char*>());
  if(!storeJson(doc)){
    Serial.println(F("Storing failed"));
    return false;
  }
  client.flush();
  doc.clear();
  return true;
}





