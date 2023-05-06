#ifndef GATEWAY_H
#define GATEWAY_H

#include "stdint.h"
#include "string.h"

// #define DISABLE_DUTY_CYCLE

#define STRINGTIFY(x)                       #x
#define GATEWAY_ID_STR(x)                   STRINGTIFY(x) 

#define GATEWAY_ID                          3

#define CONFIG_MQTT_USERNAME               "gateway" GATEWAY_ID_STR(GATEWAY_ID)
#define CONFIG_MQTT_PASSWORD               "123456?aD"
#define CONFIG_MQTT_SUBTOPIC_JA            "frames/joinaccept/gateway" GATEWAY_ID_STR(GATEWAY_ID) 
#define CONFIG_MQTT_SUBTOPIC_DL            "frames/downlink/gateway" GATEWAY_ID_STR(GATEWAY_ID) 
#define CONFIG_MQTT_SUBTOPIC_CFG           "gateways/gateway" GATEWAY_ID_STR(GATEWAY_ID) 
#define CONFIG_MQTT_PUBTOPIC_JR            "frames/joinrequest"
#define CONFIG_MQTT_PUBTOPIC_UL            "frames/uplink"
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


typedef struct SX1278_struct SX1278;

void Gateway_register_event();
void Gateway_initialize();
SX1278* Gateway_get_lora();

#endif
