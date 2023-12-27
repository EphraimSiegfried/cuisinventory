#pragma once

#include <WiFiNINA.h>
#include <Arduino.h>
#include <ArduinoJson.h>
#include "Constants.h"

class WIFI {

public:
  WIFI(String ssid, String pw);
 // bool put( StaticJsonDocument<JSONSIZE>& doc);
  bool get(String barcode, StaticJsonDocument<JSONSIZE>& doc);
  int getStatus();


private:
  bool connect();
  const char *ssid;
  const char *pw;
  int wifiStatus;
  WiFiSSLClient client;

};
