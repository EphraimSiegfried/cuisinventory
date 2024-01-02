
#include <Arduino.h>
#include <Constants.h>
#include <DB.h>
#include <unity.h>

StaticJsonDocument<JSONSIZE> apiJsonDoc;
uint32_t weight;
String sample_json_barcode;
uint32_t sample_json_id;

void setUp(void) {
    deserializeJson(apiJsonDoc, SAMPLE_PRODUCT_JSON);
    weight = 1000;
    sample_json_barcode = "3017620422003";
    sample_json_id = 1;  // DB id count starts at 1
    DB.add(apiJsonDoc, weight);
}

void tearDown(void) { DB.remove(sample_json_id, sample_json_barcode); }

void test_exists_returns_true(void) {
    TEST_ASSERT_TRUE(DB.exists(sample_json_barcode));
}

void test_exists_returns_false(void) { TEST_ASSERT_FALSE(DB.exists("000000")); }

void test_get_json_from_id_returns_correctly(void) {
    StaticJsonDocument<JSONSIZE> databaseJsonDoc;
    TEST_ASSERT_TRUE(DB.getJsonFrom(id, databaseJsonDoc));
    TEST_ASSERT_EQUAL_STRING(apiJsonDoc["product_name"],
                             databaseJsonDoc["name"]);
}

void test_get_json_from_id_returns_false(void) {
    StaticJsonDocument<JSONSIZE> databaseJsonDoc;
    uint32_t id = 900;
    TEST_ASSERT_FALSE(DB.getJsonFrom(id, databaseJsonDoc));
}

void test_sets_correctly(void) {
    String key = "quantity";
    String value = "{\"initial\": 1010,\"remaining\": 50,\"packaging\": 10}";
    TEST_ASSERT_TRUE(DB.set(sample_json_id, key, value));
    StaticJsonDocument<JSONSIZE> databaseJsonDoc;
    getJsonFromID(sample_json_id, databaseJsonDoc);
    TEST_ASSERT_EQUAL_STRING(value,
                             databaseJsonDoc["quantity"].as<const char*>());
}

void test_set_returns_false(void) {
    String key = "invalid_key";
    String value = "{}";
    TEST_ASSERT_FALSE(DB.set(sample_json_id, key, value));
}

void test_get_ids_returns_correctly(void) {
    std::vector<uint32_t>& ids;
    // Add same product twice with different weights
    DB.add(apiJsonDoc, 500);
    TEST_ASSERT_TRUE(DB.getIDs(sample_json_barcode, ids));
    TEST_ASSERT_EQUAL_INT(1, ids[0]);
    TEST_ASSERT_EQUAL_INT(2, ids[1]);
    DB.remove(2, sample_json_barcode);
}

void test_get_ids_returns_false(void) {
    std::vector<uint32_t>& ids;
    // Add same product twice with same weights
    DB.add(apiJsonDoc, 1000);
    TEST_ASSERT_FALSE(DB.getIDs(sample_json_barcode, ids));
    DB.remove(2, sample_json_barcode);
}
void test_removes_correctly(void) {
    TEST_ASSERT_TRUE(DB.remove(sample_json_id, sample_json_barcode));
    TEST_ASSERT_FALSE(DB.exists(sample_json_barcode));
}

void test_remove_returns_false(void) {
    TEST_ASSERT_FALSE(DB.remove(sample_json_id, "000"));
    TEST_ASSERT_FALSE(DB.remove("900", sample_json_barcode));
    TEST_ASSERT_FALSE(DB.remove("900", "000"));
    TEST_ASSERT_TRUE(DB.exists(sample_json_barcode));
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
