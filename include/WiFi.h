#pragma once

#include <Arduino.h>
#include <ArduinoHttpClient.h>
#include <ArduinoJson.h>
#include <Constants.h>
#include <Debug.h>
#include <SPI.h>
#include <WiFiNINA.h>

class WIFI {
   public:
    WIFI(String ssid, String pw);
    bool put(StaticJsonDocument<JSONSIZE> &doc);
    bool get(String barcode, StaticJsonDocument<JSONSIZE> &doc);

   private:
    bool connect();
    const char *ssid;
    const char *pw;
    int wifiStatus;
    WiFiSSLClient wifiClient;
};
