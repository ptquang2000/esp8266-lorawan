#include "unity.h"


TEST_CASE("Running all the registered tests", "[All]")
{
    UNITY_BEGIN();
    unity_run_all_tests();
    UNITY_END();
}

void app_main(void);
