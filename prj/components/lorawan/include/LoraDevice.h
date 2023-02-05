#ifndef LORA_DEVICE_H
#define LORA_DEVICE_H

#define DEV_ADDR_SIZE	4
#define NWK_KEY_SIZE	2
#define APP_SKEY_SIZE	2
#define APP_KEY_SIZE	16
#define JOIN_EUI_SIZE	8
#define DEV_EUI_SIZE	8
#define DEV_NONCE_SIZE	2

typedef struct LoraDevice_struct
{
	unsigned char dev_addr[DEV_ADDR_SIZE];
	unsigned char nwk_skey[NWK_KEY_SIZE];
	unsigned char app_skey[APP_SKEY_SIZE];
	unsigned char app_key[APP_KEY_SIZE];
	unsigned char join_eui[JOIN_EUI_SIZE];
	unsigned char dev_eui[DEV_EUI_SIZE];
	unsigned char dev_nonce[DEV_NONCE_SIZE];

	void* instance;
} LoraDevice;

LoraDevice* LoraDevice_create(
	unsigned char* app_key, 
	unsigned char* join_eui, 
	unsigned char* dev_eui, 
	short int dev_nonce);
void LoraDevice_destroy(LoraDevice*);


#endif // LORA_DEVICE_H
