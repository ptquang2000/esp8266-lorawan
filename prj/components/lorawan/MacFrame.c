#include "MacFrame.h"
#include "LoraUtil.h"
#include "stdlib.h"


////////////////////////////////////////////////////////////////////////////////

MacFrame* MacFrame_extract(MacFrame* frame)
{
	
}

void MacFrame_destroy(MacFrame* frame)
{
	free(frame->mhdr);
	Payload_destroy(frame->payload);
	free(frame->mic);
	free(frame->_iframe);
	free(frame);
}

MacFrame* MacFrame_create(FrameType frame_type)
{
	MacFrame* frame = malloc(sizeof(MacFrame));
	frame->instance = frame;

	frame->mhdr = malloc(sizeof(unsigned char) * MHDR_SIZE);
	frame->payload = Payload_create();
	frame->mic = malloc(sizeof(unsigned char) * MIC_SIZE);

	frame->_iframe = malloc(sizeof(IMacFrame));
	frame->_iframe->extract = &MacFrame_extract;

	memset(frame->mhdr, (frame_type << 5) | LORA_WAN_R1, MHDR_SIZE);
	frame->size = 0;

	return frame;
}

MacFrame* MacFrame_create_with_payload(FrameType frame_type, Payload* payload)
{
	MacFrame* frame = malloc(sizeof(MacFrame));
	frame->instance = frame;

	frame->mhdr = malloc(sizeof(unsigned char) * MHDR_SIZE);
	frame->payload = payload;
	frame->mic = malloc(sizeof(unsigned char) * MIC_SIZE);

	frame->_iframe = malloc(sizeof(IMacFrame));
	frame->_iframe->extract = &MacFrame_extract;

	memset(frame->mhdr, (frame_type << 5) | LORA_WAN_R1, MHDR_SIZE);
	frame->size = 0;

	return frame;
}

////////////////////////////////////////////////////////////////////////////////

void JoinRequestFrame_extract(MacFrame* frame, unsigned char* app_key)
{
	unsigned char* pdata = frame->data;

	memcpy(pdata, frame->mhdr, sizeof(unsigned char) * MHDR_SIZE);
	frame->size += MHDR_SIZE;
	pdata += MHDR_SIZE;
	free(frame->mhdr);

	frame->payload->_ipayload->extract(frame->payload);
	memcpy(pdata, frame->payload->data, sizeof(unsigned char) * frame->payload->size);
	frame->size += frame->payload->size;
	pdata += frame->payload->size;
	JoinRequestPayload_destroy(frame->payload);

	// TODO: check size < MAXIMUM_MACPAYLOAD_SIZE
	calculate_mic(app_key, frame->data, frame->size, frame->mic);
	memcpy(pdata, frame->mic, sizeof(unsigned char) * MIC_SIZE);
	frame->size += MIC_SIZE;
	pdata += MIC_SIZE;
	free(frame->mic);
}

void JoinRequestFrame_destroy(JoinRequestFrame* frame)
{
	JoinRequestPayload_destroy(frame->payload);
	MacFrame_destroy(frame->_frame);
	free(frame);
}

JoinRequestFrame* JoinRequestFrame_create(LoraDevice* device)
{
	JoinRequestFrame* frame = malloc(sizeof(JoinRequestFrame));
	frame->payload = JoinRequestPayload_create(device);
	frame->instance = frame;

	frame->_frame = MacFrame_create_with_payload(frame->payload->_payload);
	frame->_frame->instance = frame->instance;

	frame->_iframe = frame->_frame->_iframe;
	frame->_iframe->extract = &JoinRequestFrame_extract;

	return frame;
}

////////////////////////////////////////////////////////////////////////////////

void JoinAcceptFrame_extract(MacFrame* frame, unsigned char* app_key)
{
	unsigned char* data = malloc(sizeof(unsigned char) * MAXIMUM_MACPAYLOAD_SIZE);
	memset(data, 0x0, sizeof(unsigned char) * MAXIMUM_MACPAYLOAD_SIZE);
	unsigned char* pdata = data;

	memcpy(pdata, frame->mhdr, sizeof(unsigned char) * MHDR_SIZE);
	frame->size += MHDR_SIZE;
	pdata += MHDR_SIZE;
	free(frame->mhdr);

	frame->payload->_ipayload->extract(frame->payload);
	memcpy(pdata, frame->payload->data, sizeof(unsigned char) * frame->payload->size);
	frame->size += frame->payload->size;
	pdata += frame->payload->size;
	JoinRequestPayload_destroy(frame->payload);

	// TODO: check size < MAXIMUM_MACPAYLOAD_SIZE
	calculate_mic(app_key, data, frame->size, frame->mic);
	memcpy(pdata, frame->mic, sizeof(unsigned char) * MIC_SIZE);
	frame->size += MIC_SIZE;
	pdata += MIC_SIZE;
	free(frame->mic);

	memcpy(frame->data, data, sizeof(unsigned char) * MHDR_SIZE);
	aes128_decrypt(app_key, data + MHDR_SIZE, frame->data + MHDR_SIZE, frame->size - MHDR_SIZE);
	free(data);
}

void JoinAcceptFrame_destroy(JoinAcceptFrame* frame)
{
	JoinAcceptPayload_destroy(frame->payload);
	MacFrame_destroy(frame->_frame);
	free(frame);
}

JoinAcceptFrame* JoinAcceptFrame_create(
	unsigned int join_nonce, 
	unsigned char* net_id, 
	unsigned char* dev_addr, 
	DLSettings* setting, 
	short int rx_delay, 
	CFList* cf_list)
{
	JoinAcceptFrame* frame = malloc(sizeof(JoinAcceptFrame));
	frame->payload = JoinAcceptPayload_create(join_nonce, net_id, dev_addr, setting, rx_delay, cf_list);
	frame->instance = frame;

	frame->_frame = MacFrame_create_with_payload(frame->payload->_payload);
	frame->_frame->instance = frame->instance;

	frame->_iframe = frame->_frame->_iframe;
	frame->_iframe->extract = &JoinAcceptFrame_extract;
	
	return frame;
}