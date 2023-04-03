#include "unity.h"
#include "FreeRTOS.h"
#include "task.h"
#include "Gateway.h"

void app_main(void)
{
    Gateway_initialize();
    vTaskDelay(1000 / portTICK_PERIOD_MS);
    unity_run_menu();
}