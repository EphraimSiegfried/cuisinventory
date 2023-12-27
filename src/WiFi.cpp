#include "Constants.h"
#include <Arduino.h>
#include <ArduinoJson.h>
#include <WiFiNINA.h>

#define SPIWIFI SPI     // The SPI port
#define SPIWIFI_SS 13   // Chip select pin
#define ESP32_RESETN 12 // Reset pin
#define SPIWIFI_ACK 11  // a.k.a BUSY or READY pin
#define ESP32_GPIO0 -1

class WIFI {

public:
  WIFI(String ssid, String pw) {
    WIFI::ssid = ssid.c_str();
    WIFI::pw = pw.c_str();
    WIFI::wifiStatus = WL_IDLE_STATUS;
    WiFiSSLClient client;
    connect();
  }
  bool put(StaticJsonDocument *doc);

  bool get(String barcode, StaticJsonDocument *doc) {
    // if you get a connection, report back via serial:
    if (!client.connect(BARCODE_ENDPOINT, 443)) {
      Serial.println(F("Connection to server failed"));
      return false;
    }
    Serial.println(F("Connected to server"));
    // Make a HTTP request:
    client.println("GET " + BARCODE_PATH + barcode +
                   "?fields=" + BARCODE_FIELDS + " HTTP/1.1");
    client.println("Host: " + BARCODE_ENDPOINT);
    client.println("User-Agent:" + USER_AGENT);
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
    // skip first line
    client.find(singelLine, 1);

    DynamicJsonDocument doc(1024);
    auto error = deserializeJson(doc, client);
    if (error) {
      Serial.print(F("deserializeJson() failed with code "));
      Serial.println(error.c_str());
      return false;
    }
    if (strcmp(doc["status"].as<const char *>(), "success") != 0) {
      Serial.println(F("Couldn't find product"));
      return false;
    }
    Serial.println(doc["product"]["product_name"].as<const char *>());
    if (!storeJson(doc)) {
      Serial.println(F("Storing failed"));
      return false;
    }
    client.flush();
    doc.clear();
    return true;
  }

  int getStatus() {
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

private:
  const char *ssid;
  const char *pw;
  int wifiStatus;

  bool connect() {
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
    if (wifiStatus != WL_CONNECTED) {
      return false;
    }
    return true;
  }
};
