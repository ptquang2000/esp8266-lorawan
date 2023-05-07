#ifndef CLASS_A_DEVICE_H
#define CLASS_A_DEVICE_H 

#include "stdint.h"
#include "LoraDevice.h"

// #define DISABLE_DUTY_CYCLE

#define CONFIG_JOIN_ACCEPT_DELAY    5000 // ms
#define CONFIG_ADR_ACK_LIMIT        64
#define CONFIG_ADR_ACK_DELAY        32
#define CONFIG_RECEIVE_DELAY        1000 // ms
#define CONFIG_RX1_DR_OFFSET        0
#define CONFIG_NB_TRANS             1
#define CONFIG_RX_WINDOW            2000 // ms

#define CONFIG_MAX_FRAME_QUEUE      128
#define CONFIG_READING_TICKS        5

#define CONNECT_START_STATE         0
#define CONNECT_MISMATCH_STATE      1
#define CONNECT_FINISH_STATE        2
#define CONNECT_TIMEOUT_STATE       3


typedef struct SX1278_struct SX1278;

void ClassADevice_connect();
void ClassADevice_send_data_confirmed(uint8_t* data, uint8_t len, uint8_t fport);
void ClassADevice_send_data_unconfirmed(uint8_t* data, uint8_t len, uint8_t fport);
void ClassADevice_suspend_tasks();
void ClassADevice_wait_connect();
void ClassADevice_wait_uplink();
void ClassADevice_register_event();
void ClassADevice_intialize(LoraDevice* device);
SX1278* ClassADevice_get_lora();

#endif
