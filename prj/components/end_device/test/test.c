#include "unity.h"
#include "ClassADevice.h"

void app_main(void)
{
    ClassADevice_intialize();
    ClassADevice_register_event();
    
    unity_run_menu();
}

TEST_CASE("Class A connect", "[Class A]")
{
    ClassADevice_connect();
}