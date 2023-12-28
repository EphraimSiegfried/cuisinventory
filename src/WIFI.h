#pragma once

#include <Arduino.h>
#include <ArduinoJson.h>
#include <SPI.h>
#include <WiFiNINA.h>

#include "Constants.h"

class WIFI {
   public:
    WIFI(String ssid, String pw);
    // bool put( StaticJsonDocument<JSONSIZE>& doc);
    bool get(String barcode, StaticJsonDocument<JSONSIZE> &doc);

   private:
    bool connect();
    const char *ssid;
    const char *pw;
    int wifiStatus;
    WiFiSSLClient client;
};
