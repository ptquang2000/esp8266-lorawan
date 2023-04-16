#include "unity.h"
#include "FreeRTOS.h"
#include "task.h"
#include "ClassADevice.h"
#include "LoraDevice.h"

uint8_t app_key[APP_KEY_SIZE] = {0x96, 0xf4, 0x15, 0x5e, 0xfa, 0x37, 0xbe, 0xb7, 0x60, 0x5e, 0x4d, 0x5d, 0x6d, 0x65, 0x64, 0xe8};;
uint8_t join_eui[JOIN_EUI_SIZE] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
uint8_t dev_eui[DEV_EUI_SIZE] = {0xFE, 0xFF, 0xFF, 0x00, 0x00, 0x0A, 0xAA, 0xAA};
uint16_t dev_nonce = 0;
uint16_t join_nonce = 0;

void app_main(void)
{
    LoraDevice* device = LoraDevice_create(app_key, join_eui, dev_eui, dev_nonce);

    ClassADevice_intialize(device);
    ClassADevice_register_event();

    vTaskDelay(1000 / portTICK_PERIOD_MS);

    unity_run_menu();
}

TEST_CASE("Class A connect", "[Class A]")
{
    ClassADevice_connect();
}