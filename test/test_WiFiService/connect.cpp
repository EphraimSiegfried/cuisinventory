#include <Constants.h>
#include <WiFiService.h>
#include <unity.h>

void setUp(void) {}

void tearDown(void) {}

void test_connect(void) {
    TEST_ASSERT_TRUE(WiFiService.connect(SSID, PASSWORD));
}

void setup() {
    UNITY_BEGIN();
    RUN_TEST(test_connect);
    UNITY_END();
}

void loop() {}
