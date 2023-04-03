#ifndef CLASS_A_DEVICE_H
#define CLASS_A_DEVICE_H 

#include "stdint.h"
#include "LoraDevice.h"

extern uint8_t g_app_key[APP_KEY_SIZE];
extern uint8_t g_join_eui[JOIN_EUI_SIZE];
extern uint8_t g_dev_eui[DEV_EUI_SIZE];
extern uint32_t g_dev_addr;
extern uint16_t g_dev_nonce;
extern uint32_t g_join_nonce;

void ClassADevice_connect();
void ClassADevice_send_data(uint8_t* data, uint8_t len);
void ClassADevice_register_event();
void ClassADevice_intialize();

#endif
