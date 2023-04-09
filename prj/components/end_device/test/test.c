#include "unity.h"
#include "ClassADevice.h"

SX1278* s_lora = NULL;

void app_main(void)
{
    ClassADevice_intialize();
    ClassADevice_register_event();
    
    s_lora = ClassADevice_get_lora();

    unity_run_menu();
}

TEST_CASE("Class A connect", "[Class A]")
{
    ClassADevice_connect();
}