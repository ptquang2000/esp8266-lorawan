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
#define CONFIG_ADR_AACK_LIMIT       64
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

static SemaphoreHandle_t s_lora_mutex = NULL;

static TaskHandle_t s_tx_frame_handle = NULL;
static QueueHandle_t s_tx_frame_queue = NULL;
static TaskHandle_t s_rx_frame_handle = NULL;
static QueueHandle_t s_rx_frame_queue = NULL;

static uint8_t connection_state = 0;
static uint32_t s_receive_window = 0;

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

static void start_rx()
{
    ESP_LOGI(TAG, "Start rx");
    s_lora->fifo.size = 0;
    s_settings.modem_config2.bits.rx_payload_crc_on = 0;
    s_settings.invert_iq.val = DEFAULT_NORMAL_IQ;
    s_settings.invert_iq.bits.rx = 1;
    SX1278_initialize(s_lora, &s_settings);
    SX1278_start_rx(s_lora, RxSingle, ExplicitHeaderMode);
}

static void start_tx()
{
    ESP_LOGI(TAG, "Start tx");
    s_settings.modem_config2.bits.rx_payload_crc_on = 1;
    s_settings.invert_iq.val = DEFAULT_NORMAL_IQ;
    s_settings.invert_iq.bits.tx = 1;
    SX1278_initialize(s_lora, &s_settings);
    SX1278_start_tx(s_lora);
}

static void on_tx_done(void* p)
{
    uint8_t on_done;
    while (1)
    {
        on_done = ulTaskNotifyTake(pdTRUE, (TickType_t) portMAX_DELAY);
        if (on_done <= 0) continue;

        ESP_LOGI(TAG, "On transmit done");
        start_rx();
    }
}

static void on_rx_done(void* p)
{
    uint8_t on_done;
    long start = xTaskGetTickCount();
    while (1)
    {
        on_done = ulTaskNotifyTake(pdTRUE, (TickType_t) portMAX_DELAY);
        if (on_done <= 0) continue;

        if (s_lora->fifo.size == 0)
        {
            long stop = (xTaskGetTickCount() - start) * portTICK_PERIOD_MS;
            if (stop < s_receive_window)
            {
                start_rx();
            }
            else
            {
                ESP_LOGI(TAG, "Close received window without any data");
            }
        }
        else
        {
            Frame* frame = Frame_create_by_data(s_lora->fifo.size, s_lora->fifo.buffer);
            xQueueSend(s_rx_frame_queue, &frame, (TickType_t) 0);
            xSemaphoreGive(s_lora_mutex);
        }
    }
}

static void tx_frame_handler(void *p)
{
    Frame* frame;
    while (true) 
    {
        if (s_tx_frame_queue != NULL && xQueueReceive(s_tx_frame_queue, (void*)&frame, (TickType_t) CONFIG_READING_TICKS))
        {
            if (xSemaphoreTake(s_lora_mutex, (TickType_t) 0xFFFFFFFF) == 1)
            {
                s_lora->fifo.size = 0;
                SX1278_fill_fifo(s_lora, frame->data, frame->size);
                print_data(frame->data, frame->size);
                start_tx();
                Frame_destroy(frame);
            }
        }
        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
}

static void process_join_accept(JoinAcceptFrame* frame)
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

        connection_state = CONNECT_FINISH_STATE;
    }
    else if (connection_state != CONNECT_FINISH_STATE)
    {
        connection_state = CONNECT_MISMATCH_STATE;
    }
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

static void rx_frame_handler()
{
    Frame* frame;
    while (true) 
    {
        if (s_rx_frame_queue != NULL && xQueueReceive(s_rx_frame_queue, (void*)&frame, (TickType_t) CONFIG_READING_TICKS))
        {
            switch (frame->type)
            {
            case JoinAccept:
            {
                JoinAcceptFrame* join_accept = JoinAcceptFrame_create_by_frame(frame);
                process_join_accept(join_accept);
                JoinAcceptFrame_destroy(join_accept);
            }
            break;
            case UnconfirmDataDownlink:
            {
                MacFrame* udl = MacFrame_create_by_frame(frame);
                process_unconfirmed_dl(udl);
                MacFrame_destroy(udl);
            }
            break;
            case ConfirmedDataDownlink:
            {
                MacFrame* cdl = MacFrame_create_by_frame(frame);
                process_confirmed_dl(cdl);
                MacFrame_destroy(cdl);
            }
            break;
            default:
            Frame_destroy(frame);
            break;
            }
        }
        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
}

void ClassADevice_connect()
{
    s_receive_window = CONFIG_JOIN_ACCEPT_DELAY * 1000;

    JoinRequestFrame* frame = JoinRequestFrame_create(s_device->join_eui, s_device->dev_eui, s_device->dev_nonce);
	frame->_iframe->extract(frame, s_device->app_key);

    connection_state = CONNECT_START_STATE;
    Frame* tx_frame = Frame_create_by_data(frame->_frame->size, frame->_frame->data);
    xQueueSendToFront(s_tx_frame_queue, &tx_frame, (TickType_t)0);
	JoinRequestFrame_destroy(frame);
    
    while (connection_state != CONNECT_FINISH_STATE)
    {
        if (connection_state == CONNECT_MISMATCH_STATE)
        {
            connection_state = CONNECT_START_STATE;
            start_rx();
        }
        vTaskDelay(20 / portTICK_PERIOD_MS);
    }
}

void ClassADevice_send_data(uint8_t* data, uint8_t len)
{
    s_receive_window = CONFIG_RECEIVE_DELAY * 1000;
}

void ClassADevice_register_event()
{
    s_tx_frame_queue = xQueueCreate(CONFIG_MAX_FRAME_QUEUE, sizeof(Frame*));
    s_rx_frame_queue = xQueueCreate(CONFIG_MAX_FRAME_QUEUE, sizeof(Frame*));

    s_lora_mutex = xSemaphoreCreateMutex();

    xTaskCreate(tx_frame_handler, "classA_tx_frame_handler", 1024, NULL, tskIDLE_PRIORITY, &s_tx_frame_handle);
    xTaskCreate(rx_frame_handler, "classA_rx_frame_handler", 1024, NULL, tskIDLE_PRIORITY, &s_rx_frame_handle);
    
    xTaskCreate(on_tx_done, "lora_tx_done", 1024, NULL, tskIDLE_PRIORITY, &s_lora->tx_done_handle);
    xTaskCreate(on_rx_done, "lora_rx_done", 1024, NULL, tskIDLE_PRIORITY, &s_lora->rx_done_handle);
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