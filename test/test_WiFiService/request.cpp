
#include <Arduino.h>
#include <WiFiService.h>
#include <unity.h>

void setUp(void) {}

void tearDown(void) {}

void test_barcode_get_request(void) {}

void test_json_put_request(void) {}

void setup() {}

void loop() {
    UNITY_BEGIN();
    RUN_TEST(test_barcode_get_request);
    RUN_TEST(test_json_put_request);
    UNITY_END();
}
