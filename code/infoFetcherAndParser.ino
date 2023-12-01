#include <WiFiNINA.h>
#include <SPI.h>
#include <ArduinoJson.h>
#include <SD.h>
  //Initialize serial and wait for port to open:

  void setup() {
    Serial.begin(9600);
    while (!Serial) {
      ; // wait for serial port to connect. Needed for native USB port only
    }
    SD.begin(10);
    if(!connectWlan("S10","Internet5")){
      Serial.println("wlan connection failed");
    }
    Serial.println("wlan connection successfull");
    fetchData("world.openfoodfacts.org","/api/v3/product/","3017620422003","product_name,generic_name,allergens,conservation_conditions,nutriscore_grade,ingredients_text,customer_service");
  
  }

  void loop() {
  
  }