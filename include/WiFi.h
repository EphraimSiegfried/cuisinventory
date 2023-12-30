#pragma once

#include <Arduino.h>
#include <ArduinoHttpClient.h>
#include <ArduinoJson.h>
#include <Constants.h>
#include <Debug.h>
#include <SPI.h>
#include <WiFiNINA.h>

class WiFiClass2 {
   public:
    WiFiClass2();
    bool connect(String ssid, String pw);
    bool put(StaticJsonDocument<JSONSIZE> &doc);
    bool get(String barcode, StaticJsonDocument<JSONSIZE> &doc);

   private:
    int wifiStatus;
    WiFiSSLClient wifiClient;
};

extern WiFiClass2 WiFi2;
