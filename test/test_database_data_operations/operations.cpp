#include <Arduino.h>
#include <Constants.h>
#include <DB.h>
#include <unity.h>

StaticJsonDocument<JSONSIZE> apiJsonDoc;
uint32_t weight;
uint32_t time1;
uint32_t sampleJsonId;
String sampleJsonBarcode;

void setUp(void) {
    DB.clear();
    DB.initDatabase();
    deserializeJson(apiJsonDoc, SAMPLE_PRODUCT_JSON);
    weight = 1000;
    time1 = 1000;
    sampleJsonBarcode = "3017620422003";
    sampleJsonId = 1;  // DB id count starts at 1
    DB.add(apiJsonDoc, weight, time1);
}

void tearDown(void) {
    DB.clear();
}

bool exists(String barcode) {
    std::vector<uint32_t> ids;
    DB.getIDs(barcode, ids);
    return ids.size() >= 1;
}

void test_get_json_from_id_returns_correctly(void) {
    StaticJsonDocument<JSONSIZE> databaseJsonDoc;
    TEST_ASSERT_TRUE(DB.getJsonFromID(sampleJsonId, databaseJsonDoc));
    char output[JSONSIZE];
    serializeJson(databaseJsonDoc, output);
    LOG("Output: " + String(output));
    TEST_ASSERT_EQUAL_STRING(apiJsonDoc["product"]["product_name"],
                             databaseJsonDoc["name"]);
}

void test_get_json_from_id_returns_false(void) {
    StaticJsonDocument<JSONSIZE> databaseJsonDoc;
    uint32_t id = 900;
    TEST_ASSERT_FALSE(DB.getJsonFromID(id, databaseJsonDoc));
}

void test_sets_weight_correctly(void) {
    uint32_t weight = 50;
    TEST_ASSERT_TRUE(DB.setWeight(sampleJsonId, weight));
    StaticJsonDocument<JSONSIZE> databaseJsonDoc;
    DB.getJsonFromID(sampleJsonId, databaseJsonDoc);
    char output[JSONSIZE];
    serializeJson(databaseJsonDoc, output);
    LOG("Output: " + String(output));
    TEST_ASSERT_EQUAL_INT(
        50, databaseJsonDoc["quantity"]["remaining"].as<const int32_t>());
}

void test_get_ids_returns_correctly(void) {
    std::vector<uint32_t> ids;
    // Add same product twice with different weights
    DB.add(apiJsonDoc, 500, time1);
    TEST_ASSERT_TRUE(DB.getIDs(sampleJsonBarcode, ids));
    TEST_ASSERT_EQUAL_INT(1, ids[0]);
    TEST_ASSERT_EQUAL_INT(2, ids[1]);
}

void test_removes_correctly(void) {
    TEST_ASSERT_TRUE(DB.remove(sampleJsonId, sampleJsonBarcode));
    TEST_ASSERT_FALSE(exists(sampleJsonBarcode));
}

void test_remove_returns_false(void) {
    TEST_ASSERT_FALSE(DB.remove(sampleJsonId, "000"));
    TEST_ASSERT_FALSE(DB.remove(900, sampleJsonBarcode));
    TEST_ASSERT_FALSE(DB.remove(900, "000"));
    TEST_ASSERT_TRUE(exists(sampleJsonBarcode));
}
void setup() {
    UNITY_BEGIN();
    Serial.begin(9600);
    while (!Serial) {
        ;  // wait for serial port to connect. Needed for native USB port
           // only
    }
    pinMode(SD_PIN, OUTPUT);  // set SD pin mode
    if (!SD.begin(SD_PIN)) {
        LOG("Failed to find SD");
    } else {
        RUN_TEST(test_get_json_from_id_returns_correctly);
        RUN_TEST(test_get_json_from_id_returns_false);
        RUN_TEST(test_sets_weight_correctly);
        RUN_TEST(test_get_ids_returns_correctly);
        RUN_TEST(test_removes_correctly);
        RUN_TEST(test_remove_returns_false);
    }
    UNITY_END();
}

void loop() {}
