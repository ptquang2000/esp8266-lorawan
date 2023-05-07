#include "unity.h"
#include "test_utils.h"
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
    ClassADevice_suspend_tasks();
    ClassADevice_connect();
    ClassADevice_wait_connect();
}

TEST_CASE("Class A unconfirmed uplink", "[Class A]")
{
    ClassADevice_suspend_tasks();
    const static uint8_t data[] = {'u','n','c','o','n','f','i','r','m','e','d'};
    ClassADevice_send_data_unconfirmed(data, sizeof(data), 10);
    ClassADevice_wait_uplink();
}

TEST_CASE("Class A unconfirmed uplink without wait", "[Class A]")
{
    ClassADevice_suspend_tasks();
    const static uint8_t data1[] = {'u','n','c','o','n','f','i','r','m','e','d', ' ', '1'};
    ClassADevice_send_data_unconfirmed(data1, sizeof(data1), 10);
    const static uint8_t data2[] = {'u','n','c','o','n','f','i','r','m','e','d', ' ', '2'};
    ClassADevice_send_data_unconfirmed(data2, sizeof(data2), 10);

    int num = 0;
    while (num < 2)
    {
        ClassADevice_wait_uplink();
        num++;
    }
}

TEST_CASE("Class A confirmed uplink", "[Class A]")
{
    ClassADevice_suspend_tasks();
    const static uint8_t data[] = {'c','o','n','f','i','r','m','e','d'};
    ClassADevice_send_data_confirmed(data, sizeof(data), 10);
    ClassADevice_wait_uplink();
}

TEST_CASE("Class A confirmed uplink without wait", "[Class A]")
{
    ClassADevice_suspend_tasks();
    const static uint8_t data1[] = {'c','o','n','f','i','r','m','e','d', ' ', '1'};
    ClassADevice_send_data_confirmed(data1, sizeof(data1), 10);
    const static uint8_t data2[] = {'c','o','n','f','i','r','m','e','d', ' ', '2'};
    ClassADevice_send_data_confirmed(data2, sizeof(data2), 10);

    int num = 0;
    while (num < 2)
    {
        ClassADevice_wait_uplink();
        num++;
    }
}
