#include <Constants.h>
#include <DB.h>
#include <unity.h>

StaticJsonDocument<JSONSIZE> apiJsonDoc;

void setUp(void) {
    DB.clear();
    deserializeJson(apiJsonDoc, SAMPLE_PRODUCT_JSON);
}

void tearDown(void) { DB.clear(); }

void test_initialize_database_returns_true(void) {
    TEST_ASSERT_TRUE(DB.initDatabase());
}

void test_add_returns_true(void) {
    DB.initDatabase();
    uint32_t weight = 1000;
    uint32_t time = 1000;
    TEST_ASSERT_TRUE(DB.add(apiJsonDoc, weight, time));
}

void setup() {
    UNITY_BEGIN();
    Serial.begin(9600);
    while (!Serial) {
        ;  // wait for serial port to connect. Needed for native USB port only
    }
    pinMode(SD_PIN, OUTPUT);  // set SD pin mode
    if (!SD.begin(SD_PIN)) {
        LOG("Failed to find SD");
    } else {
        RUN_TEST(test_initialize_database_returns_true);
        RUN_TEST(test_add_returns_true);
    }
    UNITY_END();
}

void loop() {}
