#include <WiFi.h>

#define SPIWIFI      SPI  // The SPI port
#define SPIWIFI_SS   13   // Chip select pin
#define ESP32_RESETN 12   // Reset pin
#define SPIWIFI_ACK  11   // a.k.a BUSY or READY pin
#define ESP32_GPIO0  -1

WIFI::WIFI(String ssid, String pw) {
    this->ssid = ssid.c_str();
    this->pw = pw.c_str();
    this->wifiStatus = WL_IDLE_STATUS;
    WiFi.setPins(SPIWIFI_SS, SPIWIFI_ACK, ESP32_RESETN, ESP32_GPIO0, &SPIWIFI);
    connect();
}

bool WIFI::connect() {
    // check for the WiFi module:
    if (WiFi.status() == WL_NO_MODULE) {
        LOG("Communication with WiFi module failed!");
        return false;
    }
    LOG("Attempting to connect to WPA SSID: \r\n");
    LOG(ssid);
    // Connect to WPA/WPA2 network:
    int amountOfTries = 0;
    do {
        this->wifiStatus = WiFi.begin(ssid, pw);
        delay(100);
        amountOfTries++;
    } while (this->wifiStatus != WL_CONNECTED && amountOfTries != 30);
    if (this->wifiStatus != WL_CONNECTED) {
        return false;
    }
    return true;
}

bool WIFI::get(String barcode, StaticJsonDocument<JSONSIZE> &jsonDoc) {
    int port = 443;
    HttpClient httpClient =
        HttpClient(this->wifiClient, BARCODE_ENDPOINT.c_str(), port);
    // if you get a connection, report back via serial:
    if (!this->wifiClient.connect(BARCODE_ENDPOINT.c_str(), port)) {
        LOG("Connection to server failed");
        return false;
    }
    LOG("Connected to server");
    // send request
    httpClient.get(BARCODE_PATH + barcode + "?fields=" + BARCODE_FIELDS +
                   " HTTP/1.1\r\n" + "Host: " + BARCODE_ENDPOINT + "\r\n" +
                   "User-Agent: " + USER_AGENT);
    // Check HTTP status
    int statusCode = httpClient.responseStatusCode();
    if (statusCode != 0) {
        LOG("Unexpected response:");
        LOG(statusCode);
        return false;
    }
    // Get response
    String response = httpClient.responseBody();
    auto error = deserializeJson(jsonDoc, response);
    if (error) {
        LOG("deserializeJson() failed with code:");
        LOG(error.c_str());
        return false;
    }
    if (strcmp(jsonDoc["status"].as<const char *>(), "success") != 0) {
        LOG("Couldn't find product");
        return false;
    }
    LOG(jsonDoc["product"]["product_name"].as<const char *>());
    return true;
}

bool WIFI::put(StaticJsonDocument<JSONSIZE> &jsonDoc) {
    int port = 80;
    HttpClient httpClient =
        HttpClient(this->wifiClient, PYTHONANYWHERE_ENDPOINT.c_str(), port);
    // if you get a connection, report back via serial:
    if (!this->wifiClient.connect(PYTHONANYWHERE_ENDPOINT.c_str(), port)) {
        LOG("Connection to server failed");
        return false;
    }
    LOG("Connected to server");
    // send request
    char serializedJson[JSONSIZE];
    serializeJson(jsonDoc, serializedJson);
    String contentType = "application/json";
    httpClient.put(PYTHONANYWHERE_ENDPOINT + PYTHONANYWHERE_PATH, contentType,
                   "key=" + DEVICE_KEY + +"&" + "data=" + serializedJson);
    // Check HTTP status
    int statusCode = httpClient.responseStatusCode();
    if (statusCode != 0) {
        LOG("Unexpected response:");
        LOG(statusCode);
        return false;
    }
    return true;
}
