#include <Constants.h>
#include <DB.h>
#include <unity.h>

void setUp(void) {}

void tearDown(void) {}

void test_add_returns_true(void) {
    deserializeJson(jsonDoc, SAMPLE_PRODUCT_JSON);
    uint32_t weight = 1000;
    TEST_ASSERT_TRUE(DB.add(jsonDoc, weight));
    DB.remove(0,"3017620422003"); // TODO:Find better solution since DB.remove might not work
}

void setup() {
    UNITY_BEGIN();
    RUN_TEST(test_add_returns_true);
    UNITY_END();
}

void loop() {}
