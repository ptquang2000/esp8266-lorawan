#include "SX1278.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"


void on_tx_done(void* p)
{
    printf("Sender:\n");
    uint8_t data[] = {'s', 'x', '1', '2', '7', '8'};
    uint8_t on_done;
    SX1278* dev = p;
    while (1)
    {
        on_done = ulTaskNotifyTake(pdTRUE, (TickType_t) portMAX_DELAY);
        if (on_done > 0)
        {
            printf("Sent %d bytes: ", sizeof(data));
            for (uint8_t i = 0; i < sizeof(data); i++)
            {
                printf("%d ", data[i]);
            }
            printf("\n");
            dev->fifo.size = 0;
            SX1278_fill_fifo(dev, data, sizeof(data));
            SX1278_start_tx(dev);
            vTaskDelay(3000 / portTICK_PERIOD_MS);
        }
    }
}

void on_rx_done(void* p)
{
    printf("Receiver:\n");
    uint8_t on_done;
    SX1278* dev = p;
    long start = xTaskGetTickCount();
    while (1)
    {
        on_done = ulTaskNotifyTake(pdTRUE, (TickType_t) portMAX_DELAY);
        if (on_done > 0)
        {
            if (dev->fifo.size == 0)
            {
                long stop = (xTaskGetTickCount() - start) * portTICK_PERIOD_MS;
                printf("Timeout for %lu\n", stop);
                printf("Try again\n");
                start = xTaskGetTickCount();
                SX1278_start_rx(dev, RxSingle, ExplicitHeaderMode);
            }
            else
            {
                printf("Received %d bytes: ", dev->fifo.size);
                for (uint8_t i = 0; i < dev->fifo.size; i++)
                {
                    printf("%c ", dev->fifo.buffer[i]);
                }
                printf("\n");
            }
        }
    }
}

void app_main(void)
{
    SX1278Settings settings = {
        .channel_freq = DEFAULT_SX1278_FREQUENCY,
        .pa_config.val = DEFAULT_PA_CONFIG,
        .preamble_len = DEFAULT_PREAMBLE_LENGTH,
        .modem_config1.val = DEFAULT_MODEM_CONFIG1,
        .modem_config2.val = DEFAULT_MODEM_CONFIG2,
        .sync_word = DEFAULT_SYNC_WORD,
        .invert_iq.val = DEFAULT_NORMAL_IQ,
    };
    SX1278* dev = SX1278_create();
    SX1278_initialize(dev, &settings);

#if 0
    xTaskCreate(on_tx_done, "tx_done", 1024, (void*)dev, tskIDLE_PRIORITY, &dev->tx_done_handle);
    xTaskNotifyGive(dev->tx_done_handle);
#else
    xTaskCreate(on_rx_done, "rx_done", 1024, (void*)dev, tskIDLE_PRIORITY, &dev->rx_done_handle);
    SX1278_start_rx(dev, RxSingle, ExplicitHeaderMode);
#endif

    while (1) {
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}