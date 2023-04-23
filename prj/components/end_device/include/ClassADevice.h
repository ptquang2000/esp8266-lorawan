#ifndef CLASS_A_DEVICE_H
#define CLASS_A_DEVICE_H 

#include "stdint.h"
#include "LoraDevice.h"

typedef struct SX1278_struct SX1278;

void ClassADevice_connect();
void ClassADevice_send_data_confirmed(uint8_t* data, uint8_t len, uint8_t fport);
void ClassADevice_send_data_unconfirmed(uint8_t* data, uint8_t len, uint8_t fport);
void ClassADevice_register_event();
void ClassADevice_intialize(LoraDevice* device);
SX1278* ClassADevice_get_lora();

#endif
