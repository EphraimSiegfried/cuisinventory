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
    bool put(JsonDocument& doc);
    bool get(const char barcode[], JsonDocument& doc);
    bool get2(const char barcode[], JsonDocument& jsonDoc);

   private:
    int wifiStatus;
    WiFiSSLClient wifiClient;
};

extern WiFiServiceClass WiFiService;
