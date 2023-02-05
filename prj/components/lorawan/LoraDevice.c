#include "LoraDevice.h"
#include "LoraUtil.h"
#include "MacFrame.h"
#include "stdlib.h"


////////////////////////////////////////////////////////////////////////////////

void LoraDevice_set_dev_nonce(LoraDevice* device, short int dev_nonce)
{
	device->dev_nonce[1] = (unsigned char)(dev_nonce >> 8);
	device->dev_nonce[0] = (unsigned char)(dev_nonce);
}

void LoraDevice_increase_dev_nonce(LoraDevice* device)
{
	short int dev_nonce = device->dev_nonce[1] << 8 | device->dev_nonce[0];
	dev_nonce++;
	LoraDevice_set_dev_nonce(device, dev_nonce);
}

void LoraDevice_send_join_request(LoraDevice* device)
{
	JoinRequestFrame* frame = JoinRequestFrame_create(device);
	frame->_iframe->extract(frame->instance, device->app_key);
	JoinRequestFrame_destroy(frame);
	LoraDevice_increase_dev_nonce(device);
}

void LoraDevice_destroy(LoraDevice* device)
{
	if (device == NULL) return;
	free(device);
	device = NULL;
}

LoraDevice* LoraDevice_create(
	unsigned char* dev_addr,
	unsigned char* nwk_skey,
	unsigned char* app_skey,
	unsigned char* app_key,
	unsigned char* join_eui,
	unsigned char* dev_eui,
	short int dev_nonce)
{
	LoraDevice* device = malloc(sizeof(LoraDevice));
	device->instance = device;

	memcpy(device->dev_addr, dev_addr, BYTE_SIZE(DEV_ADDR_SIZE));
	memcpy(device->nwk_skey, nwk_skey, BYTE_SIZE(NWK_SKEY_SIZE));
	memcpy(device->app_skey, app_skey, BYTE_SIZE(APP_SKEY_SIZE));

	memcpy(device->app_key, app_key, BYTE_SIZE(APP_KEY_SIZE));
	memcpy(device->join_eui, join_eui, BYTE_SIZE(JOIN_EUI_SIZE));
	memcpy(device->dev_eui, dev_eui, BYTE_SIZE(DEV_EUI_SIZE));

	LoraDevice_set_dev_nonce(device, dev_nonce);
	
	return device;
}

////////////////////////////////////////////////////////////////////////////////