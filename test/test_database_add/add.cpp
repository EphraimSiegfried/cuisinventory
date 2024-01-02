#include <Constants.h>
#include <DB.h>
#include <unity.h>

StaticJsonDocument<JSONSIZE> apiJsonDoc;
void setUp(void) {
    deserializeJson(apiJsonDoc, SAMPLE_PRODUCT_JSON);
}

void tearDown(void) {
      DB.remove(0, "3017620422003");  // TODO:Find better solution since DB.remove
                                    // might not work
}

void test_add_returns_true(void) {
    uint32_t weight = 1000;
    uint32_t time = 1000;
    TEST_ASSERT_TRUE(DB.add(apiJsonDoc, weight, time));
}

void setup() {
    UNITY_BEGIN();
    RUN_TEST(test_add_returns_true);
    UNITY_END();
}

void loop() {}
