#include "unity.h"
#include "FreeRTOS.h"
#include "task.h"
#include "Gateway.h"
#include "SX1278.h"
#include "MacFrame.h"
#include "esp_log.h"
#include "cJSON.h"

static SX1278* s_lora = NULL;
static uint8_t app_key[] = {0x0e, 0xfe, 0x82, 0x00, 0x6e, 0x16, 0x80, 0xfa, 0x90, 0x05, 0x2a, 0xce, 0x4c, 0xed, 0xe3, 0x3b};
static uint8_t join_eui[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
static uint8_t dev_eui[] = {0xFE, 0xFF, 0xFF, 0x00, 0x00, 0x0A, 0xAA, 0xAA};
static uint16_t s_dev_nonce = 1;

void app_main(void)
{
    Gateway_initialize();
    Gateway_register_event();

    vTaskDelay(1000 / portTICK_PERIOD_MS);

    s_lora = Gateway_get_lora();

    unity_run_menu();
}

static void send_join_request()
{
    uint8_t dev_nonce[] = {(uint8_t)s_dev_nonce, (uint8_t)(s_dev_nonce >> 8)};
    JoinRequestFrame* frame = JoinRequestFrame_create(join_eui, dev_eui, dev_nonce);
	frame->_iframe->extract(frame, app_key);

    s_lora->fifo.size = 0;
    s_lora->pkt_status.rssi = -30;
    s_lora->pkt_status.snr = -20;
    SX1278_fill_fifo(s_lora, frame->_frame->data, frame->_frame->size);
    xTaskNotifyGive(s_lora->rx_done_handle);
    JoinRequestFrame_destroy(frame);
}

TEST_CASE("Publish Join Request", "[Gateway][MQTT]")
{
    send_join_request();
    s_dev_nonce += 1;
}

TEST_CASE("Publish Join Request - Initial dev nonce", "[Gateway][MQTT]")
{
    s_dev_nonce = 1;
    send_join_request();
}

TEST_CASE("Publish Confirmed Data Uplink", "[Gateway][MQTT]")
{
}

TEST_CASE("Publish Unconfirmed Data Uplink", "[Gateway][MQTT]")
{
}