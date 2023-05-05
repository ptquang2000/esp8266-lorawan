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

#define DISABLE_DUTY_CYCLE

#define CONFIG_JOIN_ACCEPT_DELAY    6
#define CONFIG_ADR_ACK_LIMIT        64
#define CONFIG_ADR_ACK_DELAY        32
#define CONFIG_RECEIVE_DELAY        2
#define CONFIG_RX1_DR_OFFSET        0
#define CONFIG_NB_TRANS             3

#define CONFIG_MAX_FRAME_QUEUE      128
#define CONFIG_READING_TICKS        5

#define CONNECT_START_STATE         0
#define CONNECT_MISMATCH_STATE      1
#define CONNECT_FINISH_STATE        2
#define CONNECT_TIMEOUT_STATE       3


static const char *TAG = "CLASS A";
static uint16_t s_frame_counter = 0; // for tracking downlink frame counter (aka Du increasment)
static uint16_t s_nb_trans = CONFIG_NB_TRANS; // for tracking retransmission (prevent Cu from increasment)

typedef struct DataBuffer_struct
{
    uint8_t fport;
    uint8_t buffer[256];
    uint8_t len;
} DataBuffer;
static DataBuffer s_data_buffer;

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

static SemaphoreHandle_t s_joinaccept_mutex = NULL;
static SemaphoreHandle_t s_downlink_mutex = NULL;

static void print_data(uint8_t* data, uint8_t data_len)
{
    printf("Data: [");
    for (int i = 0; i < data_len; i++)
    {
        printf("%02x", data[i]);
        if (i != data_len - 1)
        {
            printf(", ");
        }
    }
    printf("]\n");
}

static int process_join_accept(JoinAcceptFrame* frame)
{
    int result = frame->_iframe->validate(frame->instance, s_device->app_key);
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
        s_frame_counter = 0;
        s_nb_trans = CONFIG_NB_TRANS;

        if (uxSemaphoreGetCount(s_downlink_mutex) == 0 && s_data_buffer.len > 0)
        {
            MacFrame* mc_frame = LoraDevice_confirmed_uplink(s_device, s_data_buffer.fport, s_data_buffer.len, s_data_buffer.buffer);
            xQueueSend(s_tx_frame_queue, &mc_frame->_frame, (TickType_t)0);
        }
    }
    ESP_LOGI(TAG, "Processed join accept result %d", result);
    return result;
}

static int process_unconfirmed_dl(MacFrame* frame)
{
    int result = frame->_iframe->validate(
        frame->instance,
        s_device->nwk_skey,
        s_device->app_skey,
        1,
        s_device->dev_addr,
        s_frame_counter);
    if (result == 0)
    {
        result = frame->payload->fhdr->is_ack == 0 ? ACK_IS_UNSET : result;
        result = frame->payload->fhdr->frame_counter != s_frame_counter ? INVALID_FCNT : result;
        s_frame_counter += 1;
    }
    ESP_LOGI(TAG, "Processed unconfirmed downlink result %d", result);
    return result;
}

static int process_confirmed_dl(MacFrame* frame)
{
    int result = frame->_iframe->validate(
        frame->instance, 
        s_device->nwk_skey, 
        s_device->app_skey, 
        1, 
        s_device->dev_addr, 
        s_frame_counter);
    if (result == 0)
    {
        result = frame->payload->fhdr->is_ack == 0 ? ACK_IS_UNSET : result;
        result = frame->payload->fhdr->frame_counter != s_frame_counter ? INVALID_FCNT : result;
        s_frame_counter += 1;
    }
    ESP_LOGI(TAG, "Processed confirmed downlink result %d", result);
    return result;
}

static void on_rx_done(void* p)
{
    uint8_t on_done;
    
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
    SX1278_start_rx(s_lora, RxContinuous, ExplicitHeaderMode);
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

static void free_frame(Frame* frame)
{
    switch (frame->type)
    {
    case JoinRequest: JoinRequestFrame_destroy(frame->instance); break;
    case JoinAccept: JoinAcceptFrame_destroy(frame->instance); break;
    default: MacFrame_destroy(frame->instance); break;
    }
}

static void ClassA_event_loop(void *p)
{
    Frame* tx_frame;
    double time_on_air = 0., off_duty_cycle = 0., offset = 0.;
    uint32_t rx_window = 0;
    long begin = 0, end = 0;
    uint8_t resend = 0, continue_rx = 1;
    int result;

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
        off_duty_cycle = time_on_air * 90 / portTICK_PERIOD_MS;
        offset = xTaskGetTickCount();
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

        resend = 1;
        start_rx();
        begin = xTaskGetTickCount();
        ESP_LOGI(TAG, "Open rx window");
        while (continue_rx)
        {
            end = (xTaskGetTickCount() - begin) * portTICK_PERIOD_MS;
            if (end > rx_window || continue_rx == 0)
            {
                ESP_LOGI(TAG, "Rx window timeout");
                continue_rx = 0;
                break;
            }

            if (xSemaphoreTake(s_rx_done_mutex, (TickType_t) rx_window / portTICK_PERIOD_MS) != 1) 
            {
                vTaskDelay(20 / portTICK_PERIOD_MS);
                continue;
            }

            if (s_lora->fifo.size > 0)
            {
                continue_rx = 0;
                Frame* rx_frame = Frame_create_by_data(s_lora->fifo.size, s_lora->fifo.buffer);
                print_data(rx_frame->data, rx_frame->size);

                switch (rx_frame->type)
                {
                case JoinAccept:
                {
                    ESP_LOGI(TAG, "Recevied a join accept frame");
                    if (tx_frame->type != JoinRequest) 
                    {
                        continue_rx = 1;
                        Frame_destroy(rx_frame);
                    }
                    else
                    {
                        JoinAcceptFrame* join_accept = JoinAcceptFrame_create_by_frame(rx_frame);
                        result = process_join_accept(join_accept);
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
                    ESP_LOGI(TAG, "Recevied a unconfirmed data downlink frame");
                    if (tx_frame->type != ConfirmedDataUplink && tx_frame->type != UnconfirmedDataUplink) 
                    {
                        continue_rx = 1;
                        Frame_destroy(rx_frame);
                    }
                    else 
                    {
                        MacFrame* udl = MacFrame_create_by_frame(rx_frame);
                        result = process_unconfirmed_dl(udl);
                        MacFrame_destroy(udl);
                        if (result != 0) 
                        {
                            continue_rx = 1;
                        }
                        else 
                        {
                            resend = 0;
                            xSemaphoreGive(s_downlink_mutex);
                        }
                    }
                }
                break;
                case ConfirmedDataDownlink:
                {
                    ESP_LOGI(TAG, "Recevied a confirmed data downlink frame");
                    if (tx_frame->type != ConfirmedDataUplink || tx_frame->type != UnconfirmDataDownlink) 
                    {
                        continue_rx = 1;
                        Frame_destroy(rx_frame);
                    }
                    else 
                    {
                        MacFrame* cdl = MacFrame_create_by_frame(rx_frame);
                        result = process_confirmed_dl(cdl);
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
        }
        SX1278_switch_mode(s_lora, Standby);
        continue_rx = 1;

        if (resend)
        {
            switch (tx_frame->type)
            {
            case JoinRequest:
            {
                free_frame(tx_frame);
                JoinRequestFrame* jr_frame = LoraDevice_join_request(s_device);
                xQueueSendToFront(s_tx_frame_queue, &jr_frame->_frame, (TickType_t)0);
            }
            break;
            case UnconfirmedDataUplink:
            case ConfirmedDataUplink:
            {
                if (s_nb_trans > 0)
                {
                    xQueueSendToFront(s_tx_frame_queue, &tx_frame, (TickType_t)0);
                    s_nb_trans -= 1;
                }
                else if (tx_frame->type == ConfirmedDataUplink)
                {
                    ESP_LOGW(TAG, "Reach retransmission limit, try  reconnecting!");
                    free_frame(tx_frame);
                    while (uxQueueMessagesWaiting(s_tx_frame_queue) > 0)
                    {
                        xQueueReceive(s_tx_frame_queue, (void*)&tx_frame, (TickType_t) CONFIG_READING_TICKS);
                        free_frame(tx_frame);
                    }
                    JoinRequestFrame* jr_frame = LoraDevice_join_request(s_device);
                    xQueueSend(s_tx_frame_queue, &jr_frame->_frame, (TickType_t)0);
                }
                else 
                {
                    free_frame(tx_frame);
                    xSemaphoreGive(s_downlink_mutex);
                    s_nb_trans = CONFIG_NB_TRANS;
                }
            }
            break;
            case Proprietary: break;
            default: break;
            }
        }
        else
        {
            free_frame(tx_frame);
            s_nb_trans = CONFIG_NB_TRANS;
        }

#ifdef DISABLE_DUTY_CYCLE
        offset = 0.;
#else
        offset = off_duty_cycle - (xTaskGetTickCount() - offset);
#endif
        if (offset > 0.)
        {
            ESP_LOGI(TAG, "Tx in off duty for approximate %d ms", (int) offset * portTICK_PERIOD_MS);
            vTaskDelay(offset);
        }
        ESP_LOGI(TAG, "Frame left %d", uxQueueMessagesWaiting(s_tx_frame_queue));
        ESP_LOGI(TAG, "Ready for new tx requests");
    }
}

void ClassADevice_connect()
{
    if (s_device == NULL)
    {
        ESP_LOGE(TAG, "Required to call initialize first");
        ESP_ERROR_CHECK(ESP_FAIL);
    }

    Frame* removed_frame;
    while (uxQueueMessagesWaiting(s_tx_frame_queue) > 0)
    {
        xQueueReceive(s_tx_frame_queue, (void*)&removed_frame, (TickType_t) CONFIG_READING_TICKS);
        free_frame(removed_frame);
    }

    JoinRequestFrame* tx_frame = LoraDevice_join_request(s_device);
    xQueueSendToFront(s_tx_frame_queue, &tx_frame->_frame, (TickType_t)0);
    
    if (uxSemaphoreGetCount(s_joinaccept_mutex) == 1)
    {
        xSemaphoreTake(s_joinaccept_mutex, portMAX_DELAY);
    }

    while (1)
    {
        if (xSemaphoreTake(s_joinaccept_mutex, (TickType_t) 200 / portTICK_PERIOD_MS) == 1) break;
    }

    ESP_LOGI(TAG, "Device has connected to network server");
}

void ClassADevice_send_data_confirmed(uint8_t* data, uint8_t len, uint8_t fport)
{
    if (s_device == NULL)
    {
        ESP_LOGE(TAG, "Required to call initialize first");
        ESP_ERROR_CHECK(ESP_FAIL);
    }

    MacFrame* tx_frame = LoraDevice_confirmed_uplink(s_device, fport, len, data);
    s_data_buffer.fport = fport;
    s_data_buffer.len = len;
    memcpy(s_data_buffer.buffer, data, len);
    xQueueSendToFront(s_tx_frame_queue, &tx_frame->_frame, (TickType_t)0);

    if (uxSemaphoreGetCount(s_downlink_mutex) == 1)
    {
        xSemaphoreTake(s_downlink_mutex, portMAX_DELAY);
    }

    while (1)
    {
        if (xSemaphoreTake(s_downlink_mutex, (TickType_t) 200 / portTICK_PERIOD_MS) == 1) break;
        vTaskDelay(20 / portTICK_PERIOD_MS);
    }
    s_data_buffer.len = 0;

    ESP_LOGI(TAG, "Data have sent to network server");
}

void ClassADevice_send_data_unconfirmed(uint8_t* data, uint8_t len, uint8_t fport)
{
    if (s_device == NULL)
    {
        ESP_LOGE(TAG, "Required to call initialize first");
        ESP_ERROR_CHECK(ESP_FAIL);
    }

    MacFrame* tx_frame = LoraDevice_unconfirmed_uplink(s_device, fport, len, data);
    xQueueSendToFront(s_tx_frame_queue, &tx_frame->_frame, (TickType_t)0);

    if (uxSemaphoreGetCount(s_downlink_mutex) == 1)
    {
        xSemaphoreTake(s_downlink_mutex, portMAX_DELAY);
    }

    while (1)
    {
        if (xSemaphoreTake(s_downlink_mutex, (TickType_t) 200 / portTICK_PERIOD_MS) == 1) break;
        vTaskDelay(20 / portTICK_PERIOD_MS);
    }

    ESP_LOGI(TAG, "Data have sent to network server");
}

void ClassADevice_register_event()
{
    s_tx_frame_queue = xQueueCreate(CONFIG_MAX_FRAME_QUEUE, sizeof(Frame*));

    s_tx_done_mutex = xSemaphoreCreateMutex();
    s_rx_done_mutex = xSemaphoreCreateMutex();
    s_joinaccept_mutex = xSemaphoreCreateMutex();
    s_downlink_mutex = xSemaphoreCreateMutex();

    xSemaphoreTake(s_tx_done_mutex, portMAX_DELAY);
    xSemaphoreTake(s_rx_done_mutex, portMAX_DELAY);
    xTaskCreate(ClassA_event_loop, "classA_event_loop", 1024, NULL, tskIDLE_PRIORITY, &s_event_loop_handle);
}

void ClassADevice_intialize(LoraDevice* device)
{
    s_lora = SX1278_create();
    s_settings.pa_config.bits.output_power = TxPower0;
    s_settings.channel_freq = RF433_175MHZ;
    s_settings.modem_config1.bits.bandwidth = Bw125kHz;
    s_settings.modem_config2.bits.spreading_factor = SF11;
    SX1278_initialize(s_lora, &s_settings);

    s_device = device;
}

SX1278* ClassADevice_get_lora()
{
    return s_lora;
}
