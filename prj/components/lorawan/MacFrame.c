#include "MacFrame.h"
#include "LoraUtil.h"

#define BLOCK_B_DIR_BYTE		5
#define BLOCK_B_DIR_SIZE		1
#define BLOCK_B_DEV_ADDR_BYTE	6
#define BLOCK_B_DEV_ADDR_SIZE	4
#define BLOCK_B_FCNT_BYTE		10
#define BLOCK_B_FCNT_SIZE		4
#define BLOCK_B_MESSAGE_BYTE	15
#define BLOCK_B_MESSAGE_SIZE	1

#define BLOCK_B0_SIZE			16

static unsigned char block_b0[BLOCK_B0_SIZE] = {0x49, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

////////////////////////////////////////////////////////////////////////////////

void MacFrame_extract(
	MacFrame* frame,
	LoraDevice* device)
{
	unsigned char* pdata = frame->data;
	MacPayload* payload = (MacPayload*)frame->payload->instance;
	
	short int dir = BIT_MASK(frame->type, 1);
	memset(&block_b0[BLOCK_B_DIR_BYTE], dir, BYTE_SIZE(BLOCK_B_DIR_SIZE)); 
	memcpy(&block_b0[BLOCK_B_DEV_ADDR_BYTE], payload->fhdr->dev_addr, BYTE_SIZE(BLOCK_B_DEV_ADDR_SIZE)); 
	memset(&block_b0[BLOCK_B_FCNT_BYTE], payload->fhdr->frame_counter, BYTE_SIZE(BLOCK_B_FCNT_SIZE)); 

	memcpy(pdata, frame->mhdr, BYTE_SIZE(MHDR_SIZE));
	frame->size += MHDR_SIZE;
	pdata += MHDR_SIZE;

	payload->_ipayload->extract(payload->instance, device, &dir);
	memcpy(pdata, frame->payload->data, BYTE_SIZE(frame->payload->size));
	frame->size += frame->payload->size;
	pdata += frame->payload->size;

	memset(&block_b0[BLOCK_B_MESSAGE_BYTE], frame->size, BYTE_SIZE(BLOCK_B_MESSAGE_SIZE)); 

	// TODO: check size < MAXIMUM_MACPAYLOAD_SIZE
	unsigned char* b0_msg = malloc(BYTE_SIZE(BLOCK_B0_SIZE + frame->size));
	memcpy(b0_msg, block_b0, BYTE_SIZE(BLOCK_B0_SIZE));
	memcpy(b0_msg + BLOCK_B0_SIZE, frame->data, BYTE_SIZE(frame->size));

	calculate_mic(device->nwk_skey, b0_msg, BLOCK_B0_SIZE + frame->size, frame->mic);
	free(b0_msg);
	
	memcpy(pdata, frame->mic, BYTE_SIZE(MIC_SIZE));
	frame->size += MIC_SIZE;
	pdata += MIC_SIZE;
}

void MacFrame_destroy(MacFrame* frame)
{
	free(frame->mhdr);
	Payload_destroy(frame->payload);
	free(frame->mic);
	free(frame->_iframe);
	free(frame);
}

MacFrame* MacFrame_create(FrameType frame_type, FrameHeader* fhdr)
{
	MacFrame* frame = malloc(sizeof(MacFrame));
	frame->instance = frame;

	frame->mhdr = malloc(BYTE_SIZE(MHDR_SIZE));
	frame->payload = MacPayload_create(fhdr)->_payload;
	frame->mic = malloc(BYTE_SIZE(MIC_SIZE));

	frame->_iframe = malloc(sizeof(IMacFrame));
	frame->_iframe->extract = &MacFrame_extract;

	frame->type = frame_type;
	memset(frame->mhdr, (frame_type << 5) | LORA_WAN_R1, MHDR_SIZE);
	frame->size = 0;

	return frame;
}

MacFrame* MacFrame_create_with_payload(FrameType frame_type, Payload* payload)
{
	MacFrame* frame = malloc(sizeof(MacFrame));
	frame->instance = frame;

	frame->mhdr = malloc(BYTE_SIZE(MHDR_SIZE));
	frame->payload = payload;
	frame->mic = malloc(BYTE_SIZE(MIC_SIZE));

	frame->_iframe = malloc(sizeof(IMacFrame));
	frame->_iframe->extract = &MacFrame_extract;

	frame->type = frame_type;
	memset(frame->mhdr, (frame_type << 5) | LORA_WAN_R1, MHDR_SIZE);
	frame->size = 0;

	return frame;
}

////////////////////////////////////////////////////////////////////////////////

void JoinRequestFrame_extract(JoinRequestFrame* frame, unsigned char* app_key)
{
	unsigned char* pdata = frame->_frame->data;

	memcpy(pdata, frame->_frame->mhdr, BYTE_SIZE(MHDR_SIZE));
	frame->_frame->size += MHDR_SIZE;
	pdata += MHDR_SIZE;

	frame->_frame->payload->_ipayload->extract(frame->_frame->payload->instance);
	memcpy(pdata, frame->_frame->payload->data, BYTE_SIZE(frame->_frame->payload->size));
	frame->_frame->size += frame->_frame->payload->size;
	pdata += frame->_frame->payload->size;

	// TODO: check size < MAXIMUM_MACPAYLOAD_SIZE
	calculate_mic(app_key, frame->_frame->data, frame->_frame->size, frame->_frame->mic);
	memcpy(pdata, frame->_frame->mic, BYTE_SIZE(MIC_SIZE));
	frame->_frame->size += MIC_SIZE;
	pdata += MIC_SIZE;
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

	frame->_frame = MacFrame_create_with_payload(JoinRequest, frame->payload->_payload);
	frame->_frame->instance = frame->instance;
	frame->_iframe = frame->_frame->_iframe;
	frame->_iframe->extract = &JoinRequestFrame_extract;

	return frame;
}

////////////////////////////////////////////////////////////////////////////////

void JoinAcceptFrame_extract(JoinAcceptFrame* frame, unsigned char* app_key)
{
	unsigned char* data = malloc(BYTE_SIZE(MAXIMUM_MACPAYLOAD_SIZE));
	memset(data, 0x0, BYTE_SIZE(MAXIMUM_MACPAYLOAD_SIZE));
	unsigned char* pdata = data;

	memcpy(pdata, frame->_frame->mhdr, BYTE_SIZE(MHDR_SIZE));
	frame->_frame->size += MHDR_SIZE;
	pdata += MHDR_SIZE;

	frame->_frame->payload->_ipayload->extract(frame->_frame->payload->instance);
	memcpy(pdata, frame->_frame->payload->data, BYTE_SIZE(frame->_frame->payload->size));
	frame->_frame->size += frame->_frame->payload->size;
	pdata += frame->_frame->payload->size;

	// TODO: check size < MAXIMUM_MACPAYLOAD_SIZE
	calculate_mic(app_key, data, frame->_frame->size, frame->_frame->mic);
	memcpy(pdata, frame->_frame->mic, BYTE_SIZE(MIC_SIZE));
	frame->_frame->size += MIC_SIZE;
	pdata += MIC_SIZE;

	memcpy(frame->_frame->data, data, BYTE_SIZE(MHDR_SIZE));
	aes128_decrypt(app_key, data + MHDR_SIZE, frame->_frame->data + MHDR_SIZE, frame->_frame->size - MHDR_SIZE);
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

	frame->_frame = MacFrame_create_with_payload(JoinAccept, frame->payload->_payload);
	frame->_frame->instance = frame->instance;
	frame->_iframe = frame->_frame->_iframe;
	frame->_iframe->extract = &JoinAcceptFrame_extract;
	
	return frame;
}