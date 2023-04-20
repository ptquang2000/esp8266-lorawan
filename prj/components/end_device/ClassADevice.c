#include "ClassADevice.h"
#include "string.h"
#include "esp_system.h"
#include "esp_log.h"

#include "SX1278.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

#include "MacFrame.h"

#define CONFIG_JOIN_ACCEPT_DELAY    6
#define CONFIG_ADR_ACK_LIMIT        64
#define CONFIG_ADR_ACK_DELAY        32
#define CONFIG_RECEIVE_DELAY        2
#define CONFIG_RX1_DR_OFFSET        0

#define CONFIG_MAX_FRAME_QUEUE      5
#define CONFIG_READING_TICKS        5

#define CONNECT_START_STATE         0
#define CONNECT_MISMATCH_STATE      1
#define CONNECT_FINISH_STATE        2
#define CONNECT_TIMEOUT_STATE       3


static const char *TAG = "CLASS A";

static SX1278Settings s_settings = {
    .channel_freq = DEFAULT_SX1278_FREQUENCY,
    .pa_config.val = DEFAULT_PA_CONFIG,
    .preamble_len = DEFAULT_PREAMBLE_LENGTH,
    .modem_config1.val = DEFAULT_MODEM_CONFIG1,
    .modem_config2.val = DEFAULT_MODEM_CONFIG2,
    .sync_word = DEFAULT_SYNC_WORD,
    .invert_iq.val = DEFAULT_NORMAL_IQ,
};

static SX1278* s_lora = NULL;
static LoraDevice* s_device = NULL;

static SemaphoreHandle_t s_tx_done_mutex = NULL;
static SemaphoreHandle_t s_rx_done_mutex = NULL;

static TaskHandle_t s_event_loop_handle = NULL;
static QueueHandle_t s_tx_frame_queue = NULL;

static TaskHandle_t s_downlink_handle = NULL;
static SemaphoreHandle_t s_joinaccept_mutex = NULL;

static void print_data(uint8_t* data, uint8_t data_len)
{
    printf("Data: [");
    for (int i = 0; i < data_len; i++)
    {
        printf("%d", data[i]);
        if (i != data_len - 1)
        {
            printf(", ");
        }
    }
    printf("]\n");
}

static int process_join_accept(JoinAcceptFrame* frame)
{
    int result = frame->_iframe->validate(frame->instance);
    if (result == 0) 
    {
        static uint8_t nwk_block[] = {
            0x1, 0x0, 0x0, 0x0,
            0x0, 0x0, 0x0, 0x0,
            0x0, 0x0, 0x0, 0x0,
            0x0, 0x0, 0x0, 0x0,
        };
        memcpy(&nwk_block[1], frame->payload->join_nonce, JOIN_NONCE_SIZE);
        memcpy(&nwk_block[4], frame->payload->net_id, NET_ID_SIZE);
        memcpy(&nwk_block[7], s_device->dev_nonce, DEV_NONCE_SIZE);
        aes128_encrypt(s_device->app_key, nwk_block, s_device->nwk_skey, sizeof(nwk_block));
        static uint8_t app_block[] = {
            0x2, 0x0, 0x0, 0x0,
            0x0, 0x0, 0x0, 0x0,
            0x0, 0x0, 0x0, 0x0,
            0x0, 0x0, 0x0, 0x0,
        };
        memcpy(&app_block[1], frame->payload->join_nonce, JOIN_NONCE_SIZE);
        memcpy(&app_block[4], frame->payload->net_id, NET_ID_SIZE);
        memcpy(&app_block[7], s_device->dev_nonce, DEV_NONCE_SIZE);
        aes128_encrypt(s_device->app_key, app_block, s_device->app_skey, sizeof(app_block));

        memcpy(s_device->dev_addr, frame->payload->dev_addr, DEV_ADDR_SIZE);
    }
    return result;
}

static void process_unconfirmed_dl(MacFrame* frame)
{
    int result = frame->_iframe->validate(frame->instance);
    if (result != 0) return;
}

static void process_confirmed_dl(MacFrame* frame)
{
    int result = frame->_iframe->validate(frame->instance);
    if (result != 0) return;
}

static void on_rx_done(void* p)
{
    uint8_t on_done;
    long start = xTaskGetTickCount();
    
    while (1)
    {
        on_done = ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
        if (on_done <= 0) continue;

        xSemaphoreGive(s_rx_done_mutex);
        vTaskDelete(s_lora->rx_done_handle);
    }
}

static void start_rx()
{
    s_lora->fifo.size = 0;
    s_settings.modem_config2.bits.rx_payload_crc_on = 0;
    s_settings.invert_iq.val = DEFAULT_NORMAL_IQ;
    s_settings.invert_iq.bits.rx = 1;
    SX1278_initialize(s_lora, &s_settings);

    xTaskCreate(on_rx_done, "lora_rx_done", 1024, NULL, tskIDLE_PRIORITY, &s_lora->rx_done_handle);
    SX1278_start_rx(s_lora, RxSingle, ExplicitHeaderMode);
}

static void on_tx_done(void* p)
{
    uint8_t on_done;
    while (1)
    {
        on_done = ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
        if (on_done <= 0) continue;

        xSemaphoreGive(s_tx_done_mutex);
        vTaskDelete(s_lora->tx_done_handle);
    }
}

static void start_tx()
{
    s_settings.modem_config2.bits.rx_payload_crc_on = 1;
    s_settings.invert_iq.val = DEFAULT_NORMAL_IQ;
    s_settings.invert_iq.bits.tx = 1;
    SX1278_initialize(s_lora, &s_settings);

    xTaskCreate(on_tx_done, "lora_tx_done", 1024, NULL, tskIDLE_PRIORITY, &s_lora->tx_done_handle);
    SX1278_start_tx(s_lora);
}

static uint8_t is_resend(FrameType type)
{
    switch (type)
    {
    case JoinRequest: return 1;
    case ConfirmedDataUplink: return 1;
    case ConfirmedDataDownlink: return 1;
    default: return 0;
    }
}

static void ClassA_event_loop(void *p)
{
    Frame* tx_frame;
    double time_on_air = 0., off_duty_cycle = 0., offset = 0.;
    uint32_t rx_window = 0;
    long begin = 0, end = 0;
    uint8_t resend = 0, continue_rx = 1;

    ESP_LOGI(TAG, "Ready for new tx requests");
    while (1) 
    {
        if (s_tx_frame_queue == NULL || !xQueueReceive(s_tx_frame_queue, (void*)&tx_frame, (TickType_t) CONFIG_READING_TICKS))
        {
            vTaskDelay(20 / portTICK_PERIOD_MS);
            continue;
        }

        s_lora->fifo.size = 0;
        SX1278_fill_fifo(s_lora, tx_frame->data, tx_frame->size);
        print_data(tx_frame->data, tx_frame->size);

        time_on_air = SX1278_get_toa(s_lora);
        off_duty_cycle = time_on_air * 99 / portTICK_PERIOD_MS;
        offset = xTaskGetTickCount();
        resend = is_resend(tx_frame->type);
        if (tx_frame->type == JoinRequest)
        {
            rx_window = CONFIG_JOIN_ACCEPT_DELAY * 1000;
        }
        else
        {
            rx_window = CONFIG_RECEIVE_DELAY * 1000;
        }

        start_tx();
        while (1)
        {
            if (xSemaphoreTake(s_tx_done_mutex, portMAX_DELAY) == 1) break;   
            else vTaskDelay(20 / portTICK_PERIOD_MS);
        }
        vTaskDelay(time_on_air / portTICK_PERIOD_MS);
        ESP_LOGI(TAG, "Frame was sent");

        start_rx();
        begin = xTaskGetTickCount();
        ESP_LOGI(TAG, "Open rx window");
        while (continue_rx)
        {
            if (xSemaphoreTake(s_rx_done_mutex, portMAX_DELAY) != 1) 
            {
                vTaskDelay(20 / portTICK_PERIOD_MS);
                continue;
            };

            if (s_lora->fifo.size > 0)
            {
                continue_rx = 0;
                Frame* rx_frame = Frame_create_by_data(s_lora->fifo.size, s_lora->fifo.buffer);

                switch (rx_frame->type)
                {
                case JoinAccept:
                {
                    ESP_LOGI(TAG, "Recevie a join accept frame");
                    if (tx_frame->type != JoinRequest) 
                    {
                        continue_rx = 1;
                        Frame_destroy(rx_frame);
                    }
                    else
                    {
                        JoinAcceptFrame* join_accept = JoinAcceptFrame_create_by_frame(rx_frame);
                        int result = process_join_accept(join_accept);
                        JoinAcceptFrame_destroy(join_accept);
                        if (result != 0) 
                        {
                            continue_rx = 1;
                        }
                        else 
                        {
                            resend = 0;
                            xSemaphoreGive(s_joinaccept_mutex);
                        }
                    }
                }
                break;
                case UnconfirmDataDownlink:
                {
                    ESP_LOGI(TAG, "Recevie a unconfirmed data downlink frame");
                    if (tx_frame->type != ConfirmedDataUplink) 
                    {
                        continue_rx = 1;
                        Frame_destroy(rx_frame);
                    }
                    else 
                    {
                        MacFrame* udl = MacFrame_create_by_frame(rx_frame);
                        process_unconfirmed_dl(udl);
                        MacFrame_destroy(udl);
                    }
                }
                break;
                case ConfirmedDataDownlink:
                {
                    ESP_LOGI(TAG, "Recevie a confirmed data downlink frame");
                    if (tx_frame->type != ConfirmedDataUplink) 
                    {
                        continue_rx = 1;
                        Frame_destroy(rx_frame);
                    }
                    else 
                    {
                        MacFrame* cdl = MacFrame_create_by_frame(rx_frame);
                        process_confirmed_dl(cdl);
                        MacFrame_destroy(cdl);
                    }
                }
                break;
                default:
                {
                    ESP_LOGI(TAG, "Recevie an unexpected frame");
                    continue_rx = 1;
                    Frame_destroy(rx_frame);
                }
                break;
                }
            }

            if (continue_rx == 1)
            {
                end = (xTaskGetTickCount() - begin) * portTICK_PERIOD_MS;
                if (end > rx_window)
                {
                    ESP_LOGI(TAG, "Rx windown timeout");
                    continue_rx = 0;
                    break;
                }
                else
                {
                    start_rx();
                }
            } 
        }

        continue_rx = 1;
        if (resend)
        {
            xQueueSendToFront(s_tx_frame_queue, &tx_frame, (TickType_t)0);
        }
        else
        {
            Frame_destroy(tx_frame);
        }
        offset = xTaskGetTickCount() - offset - off_duty_cycle;
        if (offset > 0) vTaskDelay(offset);
        ESP_LOGI(TAG, "Ready for new tx requests");
    }
}

void ClassADevice_connect()
{
    JoinRequestFrame* frame = JoinRequestFrame_create(s_device->join_eui, s_device->dev_eui, s_device->dev_nonce);
	frame->_iframe->extract(frame, s_device->app_key);

    Frame* tx_frame = Frame_create_by_data(frame->_frame->size, frame->_frame->data);
    xQueueSendToFront(s_tx_frame_queue, &tx_frame, (TickType_t)0);
    xSemaphoreTake(s_joinaccept_mutex, portMAX_DELAY);
    
    while (1)
    {
        if (xSemaphoreTake(s_joinaccept_mutex, portMAX_DELAY) == 1) break;
        vTaskDelay(20 / portTICK_PERIOD_MS);
    }
	JoinRequestFrame_destroy(frame);
}

void ClassADevice_send_data(uint8_t* data, uint8_t len)
{
}

void ClassADevice_register_event()
{
    s_tx_frame_queue = xQueueCreate(CONFIG_MAX_FRAME_QUEUE, sizeof(Frame*));

    s_tx_done_mutex = xSemaphoreCreateMutex();
    s_rx_done_mutex = xSemaphoreCreateMutex();
    s_joinaccept_mutex = xSemaphoreCreateMutex();

    xTaskCreate(ClassA_event_loop, "classA_event_loop", 1024, NULL, tskIDLE_PRIORITY, &s_event_loop_handle);
}

void ClassADevice_intialize(LoraDevice* device)
{
    s_lora = SX1278_create();
    s_settings.pa_config.bits.output_power = TxPower0;
    s_settings.channel_freq = RF433_175MHZ;
    s_settings.modem_config1.bits.bandwidth = Bw125kHz;
    s_settings.modem_config2.bits.spreading_factor = SF11;
    s_settings.modem_config2.bits.rx_payload_crc_on = 1;
    SX1278_initialize(s_lora, &s_settings);

    s_device = device;
}

SX1278* ClassADevice_get_lora()
{
    return s_lora;
}