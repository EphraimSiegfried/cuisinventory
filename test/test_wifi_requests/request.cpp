#include <Arduino.h>
#include <Constants.h>
#include <WiFiService.h>
#include <unity.h>

String barcode;
StaticJsonDocument<JSONSIZE> jsonDoc;
void setUp(void) { barcode = "4104420021884"; }

void tearDown(void) { jsonDoc.clear(); }

void test_barcode_get_request_returns_true(void) {
    TEST_ASSERT_TRUE(WiFiService.get(barcode, jsonDoc));
}

void test_barcode_get_request_sets_json_correctly(void) {
    WiFiService.get(barcode, jsonDoc);
    TEST_ASSERT_EQUAL_STRING(
        "Mehl Dinkel Mehl Type 630",
        jsonDoc["product"]["product_name"].as<const char*>());
    TEST_ASSERT_EQUAL_STRING(
        "1000", jsonDoc["product"]["product_quantity"].as<const char*>());
}
void test_json_put_request(void) {
    jsonDoc["product"] = "Mehl";
    TEST_ASSERT_TRUE(WiFiService.put(jsonDoc));
}

void setup() {
    WiFiService.connect(SSID, PASSWORD);
    UNITY_BEGIN();
    // RUN_TEST(test_barcode_get_request_returns_true);
    //  RUN_TEST(test_barcode_get_request_sets_json_correctly);
    RUN_TEST(test_json_put_request);
    UNITY_END();
}

void loop() {}
