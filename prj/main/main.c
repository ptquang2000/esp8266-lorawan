#include "Gateway.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"


void app_main(void)
{
    Gateway_initialize();
    Gateway_register_event();

    while (1)
    {
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}
