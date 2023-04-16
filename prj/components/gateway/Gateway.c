#include "Gateway.h"
#include "MetaData.h"

#include "sx1278.h"

#include "mqtt_client.h"

#include "esp_wifi.h"
#include "nvs_flash.h"
#include "esp_event.h"
#include "esp_netif.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"
#include "freertos/event_groups.h"

#include "lwip/sockets.h"
#include "lwip/dns.h"
#include "lwip/netdb.h"

#include "esp_log.h"

#define GATEWAY_ID                          2

#define CONFIG_MQTT_USERNAME               "gateway1"
#define CONFIG_MQTT_PASSWORD               "123456?aD"
#define CONFIG_MQTT_SUBTOPIC_JA            "frames/joinaccept/gateway1"
#define CONFIG_MQTT_SUBTOPIC_CFG           "gateways/gateway1"
#define CONFIG_MQTT_PUBTOPIC_JR            "frames/joinrequest"
#define CONFIG_MQTT_BROKER_URL              BROKER_URL
#define CONFIG_MQTT_BROKER_PORT             1883

#define CONFIG_LORAWAN_NETID                0

#define CONFIG_ESP_WIFI_SSID                WIFI_SSID
#define CONFIG_ESP_WIFI_PASSWORD            WIFI_PASSWORD
#define CONFIG_ESP_MAXIMUM_RETRY            5
#define WIFI_CONNECTED_BIT                  BIT0
#define WIFI_FAIL_BIT                       BIT1
#define ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD   WIFI_AUTH_OPEN

#define CONFIG_MAX_FRAME_QUEUE              5
#define CONFIG_READING_TICKS                5


static const char *TAG = "GATEWAY";

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
static SemaphoreHandle_t s_lora_tx_mutex = NULL;
static SemaphoreHandle_t s_lora_rx_mutex = NULL;

static TaskHandle_t s_lora_tx_handle = NULL;
static QueueHandle_t s_lora_tx_queue = NULL;
static TaskHandle_t s_lora_rx_handle = NULL;
static QueueHandle_t s_lora_rx_queue = NULL;

static EventGroupHandle_t s_wifi_event_group;
static int s_retry_num = 0;

static esp_mqtt_client_handle_t s_mqtt_client;

static void event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        if (s_retry_num < CONFIG_ESP_MAXIMUM_RETRY) {
            esp_wifi_connect();
            s_retry_num++;
        } else {
            xEventGroupSetBits(s_wifi_event_group, WIFI_FAIL_BIT);
        }
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        (void)event;
        // ESP_LOGI(TAG, "got ip:" IPSTR, IP2STR(&event->ip_info.ip));
        s_retry_num = 0;
        xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
    }
}

static void init_wifi() {
    ESP_ERROR_CHECK(nvs_flash_init());

    s_wifi_event_group = xEventGroupCreate();

    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &event_handler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &event_handler, NULL));

    wifi_config_t wifi_config = {
        .sta = {
            .ssid = CONFIG_ESP_WIFI_SSID,
            .password = CONFIG_ESP_WIFI_PASSWORD,
            .threshold.authmode = ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD,
        },
    };

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA) );
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config) );
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(TAG, "wifi_init_sta finished.");

    EventBits_t bits = xEventGroupWaitBits(s_wifi_event_group,
        WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
        pdFALSE,
        pdFALSE,
        portMAX_DELAY);
    if (bits & WIFI_CONNECTED_BIT) {
        ESP_LOGI(TAG, "connected to ap SSID:%s password:%s", CONFIG_ESP_WIFI_SSID, CONFIG_ESP_WIFI_PASSWORD);
    } else if (bits & WIFI_FAIL_BIT) {
        ESP_LOGI(TAG, "Failed to connect to SSID:%s, password:%s", CONFIG_ESP_WIFI_SSID, CONFIG_ESP_WIFI_PASSWORD);
    } else {
        ESP_LOGE(TAG, "UNEXPECTED EVENT");
    }

    ESP_ERROR_CHECK(esp_event_handler_unregister(IP_EVENT, IP_EVENT_STA_GOT_IP, &event_handler));
    ESP_ERROR_CHECK(esp_event_handler_unregister(WIFI_EVENT, ESP_EVENT_ANY_ID, &event_handler));
    vEventGroupDelete(s_wifi_event_group);
}

static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
{
    ESP_LOGD(TAG, "Event dispatched from event loop base=%s, event_id=%" PRIi32 "", base, event_id);
    esp_mqtt_event_handle_t event = event_data;
    esp_mqtt_client_handle_t client = event->client;
    int msg_id;
    switch ((esp_mqtt_event_id_t)event_id) {
    case MQTT_EVENT_CONNECTED:
        ESP_LOGI(TAG, "MQTT_EVENT_CONNECTED");

        // msg_id = esp_mqtt_client_subscribe(client, CONFIG_MQTT_SUBTOPIC_CFG, 0);
        // ESP_LOGI(TAG, "sent subscribe successful, msg_id=%d", msg_id);

        msg_id = esp_mqtt_client_subscribe(client, CONFIG_MQTT_SUBTOPIC_JA, 0);
        ESP_LOGI(TAG, "sent subscribe successful, msg_id=%d", msg_id);
        break;
    case MQTT_EVENT_DISCONNECTED:
        ESP_LOGI(TAG, "MQTT_EVENT_DISCONNECTED");
        break;

    case MQTT_EVENT_SUBSCRIBED:
        ESP_LOGI(TAG, "MQTT_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);
        break;
    case MQTT_EVENT_UNSUBSCRIBED:
        ESP_LOGI(TAG, "MQTT_EVENT_UNSUBSCRIBED, msg_id=%d", event->msg_id);
        break;
    case MQTT_EVENT_PUBLISHED:
        ESP_LOGI(TAG, "MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
        break;
    case MQTT_EVENT_DATA:
        ESP_LOGI(TAG, "MQTT_EVENT_DATA");
        printf("TOPIC=%.*s\r\n", event->topic_len, event->topic);
        printf("DATA=[");
        for (size_t i = 0; i < event->data_len; i++)
        {
            printf("%d", event->data[i]);
            if (event->data_len != i + 1)
            {
                printf(", ");
            }
        }
        printf("]\n");
        if (strcmp(event->topic, CONFIG_MQTT_SUBTOPIC_JA))
        {
            Frame* frame = Frame_create_by_data(event->data_len, (uint8_t*)event->data);
            xQueueSend(s_lora_tx_queue, (void*)&frame, (TickType_t) 0);
        }
        break;
    case MQTT_EVENT_ERROR:
        ESP_LOGI(TAG, "MQTT_EVENT_ERROR");
        break;
    default:
        ESP_LOGI(TAG, "Other event id:%d", event->event_id);
        break;
    }
}

static void init_mqtt(void)
{
    esp_mqtt_client_config_t mqtt_cfg = {
        .host = CONFIG_MQTT_BROKER_URL,
        .port = CONFIG_MQTT_BROKER_PORT,
        .client_id = CONFIG_MQTT_USERNAME,
        .username = CONFIG_MQTT_USERNAME,
        .password = CONFIG_MQTT_PASSWORD,
    };

    s_mqtt_client = esp_mqtt_client_init(&mqtt_cfg);
    /* The last argument may be used to pass data to the event handler, in this example mqtt_event_handler */
    esp_mqtt_client_register_event(s_mqtt_client, ESP_EVENT_ANY_ID, mqtt_event_handler, NULL);
    esp_mqtt_client_start(s_mqtt_client);
}

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
    s_settings.modem_config2.bits.rx_payload_crc_on = 1;
    s_settings.invert_iq.val = DEFAULT_NORMAL_IQ;
    s_settings.invert_iq.bits.rx = 0;
    SX1278_initialize(s_lora, &s_settings);
    SX1278_start_rx(s_lora, RxSingle, ExplicitHeaderMode);
}

static void start_tx()
{
    ESP_LOGI(TAG, "Start tx");
    s_settings.modem_config2.bits.rx_payload_crc_on = 0;
    s_settings.invert_iq.val = DEFAULT_NORMAL_IQ;
    s_settings.invert_iq.bits.tx = 0;
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
        xSemaphoreGive(s_lora_rx_mutex);
    }
}

static void on_rx_done(void* p)
{
    uint8_t on_done;
    while (1)
    {
        on_done = ulTaskNotifyTake(pdTRUE, (TickType_t) portMAX_DELAY);
        if (on_done <= 0) continue;
        
        if (xSemaphoreTake(s_lora_rx_mutex, portMAX_DELAY ) == 1 )
        {
            if (s_lora->fifo.size != 0)
            {
                ESP_LOGI(TAG, "Append new frame to rx queue");
                Frame* frame = Frame_create_by_data(s_lora->fifo.size, s_lora->fifo.buffer);
                print_data(s_lora->fifo.buffer, s_lora->fifo.size);
                xQueueSend(s_lora_rx_queue, (void*)&frame, (TickType_t) 0);
                s_lora->fifo.size = 0;
            }

            if (uxQueueMessagesWaiting(s_lora_tx_queue) == 0)
            {
                xSemaphoreGive(s_lora_rx_mutex);
                start_rx();
            }
            else 
            {
                ESP_LOGI(TAG, "Suspending rx for tx");
                xSemaphoreGive(s_lora_tx_mutex);
            }
        }
    }
}


static void lora_tx_handler(void *p)
{
    Frame* frame;
    while (true) 
    {
        if (s_lora_tx_queue != NULL && xQueueReceive(s_lora_tx_queue, (void*)&frame, (TickType_t) CONFIG_READING_TICKS))
        {
            if (xSemaphoreTake(s_lora_tx_mutex, portMAX_DELAY) == 1 )
            {
                ESP_LOGI(TAG, "Append new frame to tx queue");
                SX1278_fill_fifo(s_lora, frame->data, frame->size);
                start_tx(s_lora);
                Frame_destroy(frame);
            }
        }
        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
}

static void lora_rx_handler()
{
    Frame* frame;
    int msg_id;
    while (true) 
    {
        if (s_lora_rx_queue != NULL && xQueueReceive(s_lora_rx_queue, (void*)&frame, (TickType_t) CONFIG_READING_TICKS))
        {
            MetaData meta_data = {
                .id = GATEWAY_ID,
                .rssi = s_lora->pkt_status.rssi,
                .snr = s_lora->pkt_status.snr,
                .frame = frame->data,
                .size = frame->size,
            };
            MetaData_create_json(&meta_data);
            msg_id = esp_mqtt_client_publish(s_mqtt_client, CONFIG_MQTT_PUBTOPIC_JR, meta_data.json, strlen(meta_data.json), 0, 0);
            ESP_LOGI(TAG, "sent subscribe successful, msg_id=%d, msg=%s", msg_id, meta_data.json);

            MetaData_free_json(&meta_data);
            Frame_destroy(frame);
        }
        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
}

void Gateway_register_event()
{
    s_lora_tx_queue = xQueueCreate(CONFIG_MAX_FRAME_QUEUE, sizeof(Frame*));
    s_lora_rx_queue = xQueueCreate(CONFIG_MAX_FRAME_QUEUE, sizeof(Frame*));

    s_lora_tx_mutex = xSemaphoreCreateMutex();
    xSemaphoreTake(s_lora_tx_mutex, portMAX_DELAY);

    s_lora_rx_mutex = xSemaphoreCreateMutex();

    xTaskCreate(lora_tx_handler, "lora_tx_handler", 2048, NULL, tskIDLE_PRIORITY, &s_lora_tx_handle);
    xTaskCreate(lora_rx_handler, "lora_rx_handler", 2048, NULL, tskIDLE_PRIORITY, &s_lora_rx_handle);
    
    xTaskCreate(on_tx_done, "lora_tx_done", 1024, NULL, tskIDLE_PRIORITY, &s_lora->tx_done_handle);
    xTaskCreate(on_rx_done, "lora_rx_done", 1024, NULL, tskIDLE_PRIORITY, &s_lora->rx_done_handle);
    
    start_rx();
}

void Gateway_initialize()
{
    ESP_LOGI(TAG, "[APP] Startup..");
    ESP_LOGI(TAG, "[APP] Free memory: %" PRIu32 " bytes", esp_get_free_heap_size());
    ESP_LOGI(TAG, "[APP] IDF version: %s", esp_get_idf_version());

    init_wifi();

    init_mqtt();

    s_lora = SX1278_create();
    s_settings.pa_config.bits.output_power = TxPower0;
    s_settings.channel_freq = RF433_175MHZ;
    s_settings.modem_config1.bits.bandwidth = Bw125kHz;
    s_settings.modem_config2.bits.spreading_factor = SF11;
    s_settings.modem_config2.bits.rx_payload_crc_on = 1;
    SX1278_initialize(s_lora, &s_settings);
}

SX1278* Gateway_get_lora()
{
    return s_lora;
}