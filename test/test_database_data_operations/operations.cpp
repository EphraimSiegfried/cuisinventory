
#include <Arduino.h>
#include <Constants.h>
#include <DB.h>
#include <unity.h>

StaticJsonDocument<JSONSIZE> apiJsonDoc;
uint32_t weight;
uint32_t time;
uint32_t sampleJsonId;
String sampleJsonBarcode;

void setUp(void) {
    deserializeJson(apiJsonDoc, SAMPLE_PRODUCT_JSON);
    weight = 1000;
    time = 1000;
    sampleJsonBarcode = "3017620422003";
    sampleJsonId = 1;  // DB id count starts at 1
    DB.add(apiJsonDoc, weight, time);
}

void tearDown(void) { DB.remove(sampleJsonId, sampleJsonBarcode); }

void test_exists_returns_true(void) {
    TEST_ASSERT_TRUE(DB.exists(sampleJsonBarcode));
}

void test_exists_returns_false(void) { TEST_ASSERT_FALSE(DB.exists("000000")); }

void test_get_json_from_id_returns_correctly(void) {
    StaticJsonDocument<JSONSIZE> databaseJsonDoc;
    TEST_ASSERT_TRUE(DB.getJsonFromID(sampleJsonId, databaseJsonDoc));
    TEST_ASSERT_EQUAL_STRING(apiJsonDoc["product_name"],
                             databaseJsonDoc["name"]);
}

void test_get_json_from_id_returns_false(void) {
    StaticJsonDocument<JSONSIZE> databaseJsonDoc;
    uint32_t id = 900;
    TEST_ASSERT_FALSE(DB.getJsonFromID(id, databaseJsonDoc));
}

void test_sets_correctly(void) {
    String key = "quantity";
    String value = "{\"initial\": 1010,\"remaining\": 50,\"packaging\": 10}";
    TEST_ASSERT_TRUE(DB.set(sampleJsonId, key, value));
    StaticJsonDocument<JSONSIZE> databaseJsonDoc;
    getJsonFromID(sampleJsonId, databaseJsonDoc);
    TEST_ASSERT_EQUAL_STRING(value.c_str(),
                             databaseJsonDoc["quantity"].as<const char*>());
}

void test_set_returns_false(void) {
    String key = "invalid_key";
    String value = "{}";
    TEST_ASSERT_FALSE(DB.set(sampleJsonId, key, value));
}

void test_get_ids_returns_correctly(void) {
    std::vector<uint32_t> ids;
    // Add same product twice with different weights
    DB.add(apiJsonDoc, 500,time);
    TEST_ASSERT_TRUE(DB.getIDs(sampleJsonBarcode, ids));
    TEST_ASSERT_EQUAL_INT(1, ids[0]);
    TEST_ASSERT_EQUAL_INT(2, ids[1]);
    DB.remove(2, sampleJsonBarcode);
}

void test_get_ids_returns_false(void) {
    std::vector<uint32_t>& ids;
    // Add same product twice with same weights
    DB.add(apiJsonDoc, weight,time);
    TEST_ASSERT_FALSE(DB.getIDs(sampleJsonBarcode, ids));
    DB.remove(2, sampleJsonBarcode);
}
void test_removes_correctly(void) {
    TEST_ASSERT_TRUE(DB.remove(sampleJsonId, sampleJsonBarcode));
    TEST_ASSERT_FALSE(DB.exists(sampleJsonBarcode));
}

void test_remove_returns_false(void) {
    TEST_ASSERT_FALSE(DB.remove(sampleJsonId, "000"));
    TEST_ASSERT_FALSE(DB.remove(900, sampleJsonBarcode));
    TEST_ASSERT_FALSE(DB.remove(900, "000"));
    TEST_ASSERT_TRUE(DB.exists(sampleJsonBarcode));
}
void setup() {
    UNITY_BEGIN();
    RUN_TEST(test_exists_returns_true);
    RUN_TEST(test_exists_returns_false);
    RUN_TEST(test_get_json_from_id_returns_correctly);
    RUN_TEST(test_get_json_from_id_returns_false);
    RUN_TEST(test_sets_correctly);
    RUN_TEST(test_set_returns_false);
    RUN_TEST(test_get_ids_returns_correctly);
    RUN_TEST(test_get_ids_returns_false);
    RUN_TEST(test_removes_correctly);
    RUN_TEST(test_remove_returns_false);
    UNITY_END();
}

void loop() {}
