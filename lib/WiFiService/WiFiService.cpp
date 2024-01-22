#include <WiFiService.h>

#define SPIWIFI      SPI  // The SPI port
#define SPIWIFI_SS   13   // Chip select pin
#define ESP32_RESETN 12   // Reset pin
#define SPIWIFI_ACK  11   // a.k.a BUSY or READY pin
#define ESP32_GPIO0  -1

WiFiServiceClass::WiFiServiceClass() {
    this->wifiStatus = WL_IDLE_STATUS;
    WiFi.setPins(SPIWIFI_SS, SPIWIFI_ACK, ESP32_RESETN, ESP32_GPIO0, &SPIWIFI);
}

bool WiFiServiceClass::connect(String ssid, String pw) {
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
        this->wifiStatus = WiFi.begin(ssid.c_str(), pw.c_str());
        delay(100);
        amountOfTries++;
    } while (this->wifiStatus != WL_CONNECTED && amountOfTries != 30);
    if (this->wifiStatus != WL_CONNECTED) {
        return false;
    }
    return true;
}

bool WiFiServiceClass::get(String barcode, DynamicJsonDocument &jsonDoc) {
    int port = 443;
    HttpClient httpClient =
        HttpClient(this->wifiClient, BARCODE_ENDPOINT, port);
    // if you get a connection, report back via serial:
    if (!this->wifiClient.connect(BARCODE_ENDPOINT, port)) {
        LOG("Connection to server failed");
        return false;
    }
    LOG("Connected to server");
    {
        char request[300];
        char code[220];
        barcode.toCharArray(code, 220);
        strcpy(request, "/api/v3/product/");
        LOG(strlen(code));
        char fields[] = BARCODE_FIELDS;
        LOG(code);
        strcat(request, code);
        for (int i = 0; fields[i] != '\0'; i++) {
            request[15 + strlen(code) + i] = fields[i];
            request[15 + strlen(code) + i + 1] = '\0';
        }
        LOG(request);
        httpClient.get(request);
    }
    //               " HTTP/1.1\r\n" + "Host: " + BARCODE_ENDPOINT + "\r\n" +
    //               "User-Agent: " + USER_AGENT);
    // Check HTTP status
    int statusCode = httpClient.responseStatusCode();
    LOG(statusCode);
    if (statusCode != 200) {
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

bool WiFiServiceClass::put(StaticJsonDocument<JSONSIZE> &jsonDoc) {
    int port = 443;
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
    LOG(serializedJson);
    String contentType = "application/json";
    String putData =
        "{\"key\":\"" + DEVICE_KEY + "\", \"data\":" + serializedJson + "}";
    LOG(putData);
    httpClient.put(PYTHONANYWHERE_PATH, contentType, putData);
    // Check HTTP status
    int statusCode = httpClient.responseStatusCode();
    if (statusCode != 200) {
        LOG("Unexpected response:");
        LOG(statusCode);
        return false;
    }
    return true;
}

WiFiServiceClass WiFiService;
