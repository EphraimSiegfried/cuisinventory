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

bool WiFiServiceClass::get(String barcode,
                           StaticJsonDocument<JSONSIZE> &jsonDoc) {
    int port = 443;
    HttpClient httpClient =
        HttpClient(this->wifiClient, BARCODE_ENDPOINT, port);
    // if you get a connection, report back via serial:
    if (!this->wifiClient.connect(BARCODE_ENDPOINT, port)) {
        LOG("Connection to server failed");
        return false;
    }
    LOG("Connected to server");
    // send request
    String request = "";
    LOG("barcode path:" + String(BARCODE_PATH));
    request = request + String(BARCODE_PATH);
    request = request + barcode;
    LOG(request);
    char request2[request.length() + 1]; // +1 for the null terminator
    request.toCharArray(request2, sizeof(request2));
    char finalRequest[300] = "";
    strcat(finalRequest,request2);
        LOG(String(finalRequest));
    strcat(finalRequest,"?fields=");
        LOG(String(finalRequest));
    strcat(finalRequest,BARCODE_FIELDS);
    LOG(String(finalRequest));
    httpClient.get("/api/v3/product/7610238272310?fields=empty,product_name,generic_name,allergens,conservation_conditions,nutriscore_grade,ingredients_text,customer_service,product_quantity,brands,image_url,categories,empty");
    //               " HTTP/1.1\r\n" + "Host: " + BARCODE_ENDPOINT + "\r\n" +
    //               "User-Agent: " + USER_AGENT);
    // Check HTTP status
    int statusCode = httpClient.responseStatusCode();
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
