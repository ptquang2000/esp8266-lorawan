#ifndef LORA_DEVICE_H
#define LORA_DEVICE_H

#include "LoraUtil.h"

#define DEV_ADDR_SIZE	4
#define NWK_SKEY_SIZE	16
#define APP_SKEY_SIZE	16
#define APP_KEY_SIZE	16
#define JOIN_EUI_SIZE	8
#define DEV_EUI_SIZE	8
#define DEV_NONCE_SIZE	2

typedef struct JoinRequestFrame_struct JoinRequestFrame;

typedef struct LoraDevice_struct
{
	uint8_t dev_addr[DEV_ADDR_SIZE];
	uint8_t nwk_skey[NWK_SKEY_SIZE];
	uint8_t app_skey[APP_SKEY_SIZE];
	uint8_t app_key[APP_KEY_SIZE];
	uint8_t join_eui[JOIN_EUI_SIZE];
	uint8_t dev_eui[DEV_EUI_SIZE];
	uint8_t dev_nonce[DEV_NONCE_SIZE];

	void* instance;
} LoraDevice;

JoinRequestFrame* LoraDevice_join_request(LoraDevice* device);
void LoraDevice_incr_dev_nonce(LoraDevice* device);
LoraDevice* LoraDevice_create(
	uint8_t* app_key, 
	uint8_t* join_eui, 
	uint8_t* dev_eui, 
	uint16_t dev_nonce);
void LoraDevice_destroy(LoraDevice*);


#endif // LORA_DEVICE_H
