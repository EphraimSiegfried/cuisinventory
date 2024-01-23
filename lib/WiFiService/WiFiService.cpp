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
        LOG(F("Communication with WiFi module failed!"));
        return false;
    }
    LOG(F("Attempting to connect to WPA SSID: \r\n"));
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

bool WiFiServiceClass::get(const char barcode[], JsonDocument &jsonDoc) {
    int port = 443;
    HttpClient httpClient =
        HttpClient(this->wifiClient, BARCODE_ENDPOINT, port);
    // if you get a connection, report back via serial:
    if (!this->wifiClient.connect(BARCODE_ENDPOINT, port)) {
        LOG(F("Connection to server failed"));
        return false;
    }
    LOG(F("Connected to server"));
    {
        char request[200];
        // snprintf(request, 200, "%s%s%s%s", BARCODE_PATH, barcode,
        // "?fields=", BARCODE_FIELDS);
        // char request[200];
        // char code[30];
        // barcode.toCharArray(code, 30);
        strcpy(request, "/api/v3/product/");
        LOG(strlen(barcode));
        // char fields[] = BARCODE_FIELDS;
        LOG(barcode);
        strcat(request, barcode);
        for (int i = 0; BARCODE_FIELDS[i] != '\0'; i++) {
            request[15 + strlen(barcode) + i] = BARCODE_FIELDS[i];
            request[15 + strlen(barcode) + i + 1] = '\0';
        }
        LOG(request);
        httpClient.get(request);
    }
    //               " HTTP/1.1\r\n" + "Host: " + BARCODE_ENDPOINT +
    //               "\r\n" + "User-Agent: " + USER_AGENT);
    // Check HTTP status
    int statusCode = httpClient.responseStatusCode();
    LOG(statusCode);
    if (statusCode != 200) {
        LOG(F("Unexpected response:"));
        LOG(statusCode);
        return false;
    }
    // Get response
    auto error = deserializeJson(jsonDoc, httpClient.responseBody());
    if (error) {
        LOG(F("deserializeJson() failed with code:"));
        LOG(error.c_str());
        return false;
    }
    if (strcmp(jsonDoc["status"].as<const char *>(), "success") != 0) {
        LOG(F("Couldn't find product"));
        return false;
    }
    LOG(jsonDoc["product"]["product_name"].as<const char *>());
    return true;
}
//this get2 method is more memory efficient as it passes the stream with server response directly to deserialize json without copy
bool WiFiServiceClass::get2(const char barcode[],
                            JsonDocument &jsonDoc) {
    if (!this->wifiClient.connect(BARCODE_ENDPOINT, 443)) {
        Serial.println(F("Connection to server failed"));
        return false;
    }
    LOG(F("Connected to server"));
    char request[200];
    snprintf(request, 200, "%s%s%s%s%s%s", "GET ", BARCODE_PATH, barcode,
             "?fields=", BARCODE_FIELDS, " HTTP/1.1");
    wifiClient.println("Host: " + String(BARCODE_ENDPOINT));
    wifiClient.println(
        F("User-Agent: Cuisinventory/1.0 alexander.lutsch@stud.unibas.ch"));
    wifiClient.println("Connection: close");
    wifiClient.println();
    // Check HTTP status
    char status[32] = {0};
    wifiClient.readBytesUntil('\r', status, sizeof(status));
    if (strcmp(status, "HTTP/1.1 200 OK") != 0) {
        Serial.print(F("Unexpected response: "));
        Serial.println(status);
        return false;
    }

    char singelLine[] = "\n";
    char doubleLine[] = "\r\n\r\n";
    // Skip headers
    wifiClient.find(doubleLine, 4);
    // skip first line
    wifiClient.find(singelLine, 1);

    auto error = deserializeJson(jsonDoc, wifiClient);
    if (error) {
        LOG(F("deserializeJson() failed with code:"));
        LOG(error.c_str());
        return false;
    }
    wifiClient.stop();
    if (strcmp(jsonDoc["status"].as<const char *>(), "success") != 0) {
        LOG(F("Couldn't find product"));
        return false;
    }
    LOG(jsonDoc["product"]["product_name"].as<const char *>());
    return true;
}

bool WiFiServiceClass::put(JsonDocument &jsonDoc) {
    int port = 443;
    HttpClient httpClient =
        HttpClient(this->wifiClient, PYTHONANYWHERE_ENDPOINT, port);
    // if you get a connection, report back via serial:
    if (!this->wifiClient.connect(PYTHONANYWHERE_ENDPOINT, port)) {
        LOG(F("Connection to server failed"));
        return false;
    }
    LOG(F("Connected to server"));
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
        LOG(F("Unexpected response:"));
        LOG(statusCode);
        return false;
    }
    return true;
}

WiFiServiceClass WiFiService;
