#pragma once

#include <Arduino.h>
#include <ArduinoHttpClient.h>
#include <ArduinoJson.h>
#include <Constants.h>
#include <Debug.h>
#include <SPI.h>
#include <WiFiNINA.h>

class WiFiServiceClass {
   public:
    WiFiServiceClass();
    bool connect(String ssid, String pw);
    bool put(StaticJsonDocument<JSONSIZE> &doc);
    bool get(String barcode, DynamicJsonDocument& doc);

   private:
    int wifiStatus;
    WiFiSSLClient wifiClient;
};

extern WiFiServiceClass WiFiService;
