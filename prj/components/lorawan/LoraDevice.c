#include "LoraDevice.h"
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
	frame->_iframe->extract(frame, device->app_key);
	JoinRequestFrame_destroy(frame);
	LoraDevice_increase_dev_nonce(device);
}

void LoraDevice_destroy(LoraDevice* device)
{
	free(device->dev_nonce);
	free(device);
}

LoraDevice* LoraDevice_create(unsigned char* app_key, unsigned char* join_eui, unsigned char* dev_eui, short int dev_nonce)
{
	LoraDevice* device = malloc(sizeof(LoraDevice));
	device->instance = device;

	memcpy(device->app_key, app_key, sizeof(unsigned char) * APP_KEY_SIZE);
	memcpy(device->join_eui, join_eui, sizeof(unsigned char) * JOIN_EUI_SIZE);
	memcpy(device->dev_eui, dev_eui, sizeof(unsigned char) * DEV_EUI_SIZE);
	LoraDevice_set_dev_nonce(device, dev_nonce);
	return device;
}

////////////////////////////////////////////////////////////////////////////////