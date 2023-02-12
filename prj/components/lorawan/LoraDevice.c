#include "LoraDevice.h"
#include "MacFrame.h"
#include "stdlib.h"


////////////////////////////////////////////////////////////////////////////////

void LoraDevice_set_dev_nonce(LoraDevice* device, uint16_t dev_nonce)
{
	device->dev_nonce[1] = (dev_nonce >> 8) & 0xff;
	device->dev_nonce[0] = (dev_nonce >> 0) & 0xff;
}

void LoraDevice_incr_dev_nonce(LoraDevice* device)
{
	uint16_t dev_nonce = device->dev_nonce[1] << 8 | device->dev_nonce[0];
	dev_nonce++;
	LoraDevice_set_dev_nonce(device, dev_nonce);
}

void LoraDevice_send_join_request(LoraDevice* device)
{
	JoinRequestFrame* frame = JoinRequestFrame_create(
		device->join_eui,
		device->dev_eui,
		device->dev_nonce);
	frame->_iframe->extract(frame->instance, device->app_key);
	JoinRequestFrame_destroy(frame);
	LoraDevice_incr_dev_nonce(device);
}

void LoraDevice_set_dev_addr(LoraDevice* device, uint32_t dev_addr)
{
	device->dev_addr[3] = (dev_addr >> 0) & 0xff;
	device->dev_addr[2] = (dev_addr >> 8) & 0xff;
	device->dev_addr[1] = (dev_addr >> 16) & 0xff;
	device->dev_addr[0] = (dev_addr >> 24) & 0xff;
}

void LoraDevice_destroy(LoraDevice* device)
{
	free(device);
}

LoraDevice* LoraDevice_create(
	uint32_t dev_addr,
	uint8_t* app_key,
	uint8_t* join_eui,
	uint8_t* dev_eui,
	uint16_t dev_nonce)
{
	LoraDevice* device = malloc(sizeof(LoraDevice));
	device->instance = device;

	memcpy(device->app_key, app_key, BYTE_SIZE(APP_KEY_SIZE));
	memcpy(device->join_eui, join_eui, BYTE_SIZE(JOIN_EUI_SIZE));
	memcpy(device->dev_eui, dev_eui, BYTE_SIZE(DEV_EUI_SIZE));

	LoraDevice_set_dev_addr(device, dev_addr);
	LoraDevice_set_dev_nonce(device, dev_nonce);
	
	return device;
}

////////////////////////////////////////////////////////////////////////////////