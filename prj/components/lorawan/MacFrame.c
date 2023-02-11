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

static unsigned char block_b0[BLOCK_B0_SIZE] = {
	0x49, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00
};

////////////////////////////////////////////////////////////////////////////////

void Frame_extract(Frame* frame)
{
}

short int Frame_validate(Frame* frame)
{
	if (frame->size < MINIMUM_PHYPAYLOAD_SIZE)
	{
		return INVALID_DATA_SIZE;
	}
	return 0;
}

short int Frame_get_version(Frame* frame)
{
	return GET_BITS((*frame->mhdr), FRAME_VERSION_BITS, FRAME_VERSION_OFFSET);
}

Frame* Frame_create_by_data(short int size, unsigned char* data)
{
	Frame* frame = malloc(sizeof(Frame));
	frame->instance = frame;

	frame->mhdr = malloc(BYTE_SIZE(MHDR_SIZE));

	frame->_iframe = malloc(sizeof(IFrame));
	frame->_iframe->extract = &Frame_extract;
	frame->_iframe->validate = &Frame_validate;

	ESP_ERROR_CHECK(size > MAXIMUM_PHYPAYLOAD_SIZE);
	frame->size = size;
	memcpy(frame->data, data, BYTE_SIZE(size));

	ESP_ERROR_CHECK(frame->size < MINIMUM_PHYPAYLOAD_SIZE);

	memcpy(frame->mhdr, frame->data, BYTE_SIZE(MHDR_SIZE));
	frame->type = (FrameType)GET_BITS((*frame->mhdr), FRAME_TYPE_BITS, FRAME_TYPE_OFFSET);

	return frame;
}

Frame* Frame_create(FrameType frame_type)
{
	Frame* frame = malloc(sizeof(Frame));
	frame->instance = frame;

	frame->mhdr = malloc(BYTE_SIZE(MHDR_SIZE));
	frame->mic = malloc(BYTE_SIZE(MIC_SIZE));

	frame->_iframe = malloc(sizeof(IFrame));
	frame->_iframe->extract = &Frame_extract;
	frame->_iframe->validate = &Frame_validate;

	frame->type = frame_type;
	memset(frame->mhdr, (frame_type << 5) | LORA_WAN_R1, MHDR_SIZE);
	frame->size = 0;

	return frame;
}

void Frame_destroy(Frame* frame)
{
	free(frame->mhdr);
	free(frame->mic);
	
	free(frame->_iframe);
	free(frame);
}

////////////////////////////////////////////////////////////////////////////////

short int MacFrame_validate(
	MacFrame* frame,
	short int dir,
	unsigned char* dev_addr, 
	short int frame_counter,
	unsigned char* nwk_skey)
{
	short int size_no_mic = frame->_frame->size - MIC_SIZE;
	memset(
		&block_b0[BLOCK_B_DIR_BYTE], 
		BIT_MASK(dir, 1), 
		BYTE_SIZE(BLOCK_B_DIR_SIZE)
	); 
	memcpy(
		&block_b0[BLOCK_B_DEV_ADDR_BYTE], 
		dev_addr, 
		BYTE_SIZE(BLOCK_B_DEV_ADDR_SIZE)
	); 
	memset(
		&block_b0[BLOCK_B_FCNT_BYTE], 
		frame_counter + 1, 
		BYTE_SIZE(BLOCK_B_FCNT_SIZE)
	); 
	memset(
		&block_b0[BLOCK_B_MESSAGE_BYTE], 
		size_no_mic, 
		BYTE_SIZE(BLOCK_B_MESSAGE_SIZE)
	); 

	// TODO: check size < MAXIMUM_MACPAYLOAD_SIZE
	unsigned char* b0_msg = malloc(BYTE_SIZE(BLOCK_B0_SIZE + size_no_mic));
	memcpy(b0_msg, block_b0, BYTE_SIZE(BLOCK_B0_SIZE));
	memcpy(b0_msg + BLOCK_B0_SIZE, frame->_frame->data, BYTE_SIZE(size_no_mic));

	unsigned char mic[MIC_SIZE];
	calculate_mic(nwk_skey, b0_msg, BLOCK_B0_SIZE + size_no_mic, mic);
	free(b0_msg);

	if (memcmp(mic, frame->_frame->mic, MIC_SIZE) != 0) return INVALID_MIC;

	Payload* payload = Payload_create();

	return 0;
}

void MacFrame_extract(
	MacFrame* frame,
	unsigned char* nwk_skey,
	unsigned char* app_skey)
{
	unsigned char* pdata = frame->_frame->data;

	memcpy(pdata, frame->_frame->mhdr, BYTE_SIZE(MHDR_SIZE));
	frame->_frame->size += MHDR_SIZE;
	pdata += MHDR_SIZE;

	short int dir = BIT_MASK(frame->_frame->type, 1);
	frame->payload->_ipayload->extract(frame->payload->instance, nwk_skey, app_skey, &dir);
	memcpy(pdata, frame->payload->_payload->data, BYTE_SIZE(frame->payload->_payload->size));
	frame->_frame->size += frame->payload->_payload->size;
	pdata += frame->payload->_payload->size;
	
	memset(
		&block_b0[BLOCK_B_DIR_BYTE], 
		dir, 
		BYTE_SIZE(BLOCK_B_DIR_SIZE)
	); 
	memcpy(
		&block_b0[BLOCK_B_DEV_ADDR_BYTE], 
		frame->payload->fhdr->dev_addr, 
		BYTE_SIZE(BLOCK_B_DEV_ADDR_SIZE)
	); 
	memset(
		&block_b0[BLOCK_B_FCNT_BYTE], 
		frame->payload->fhdr->frame_counter, 
		BYTE_SIZE(BLOCK_B_FCNT_SIZE)
	); 
	memset(
		&block_b0[BLOCK_B_MESSAGE_BYTE], 
		frame->_frame->size, 
		BYTE_SIZE(BLOCK_B_MESSAGE_SIZE)
	); 

	// TODO: check size < MAXIMUM_MACPAYLOAD_SIZE
	unsigned char* b0_msg = malloc(BYTE_SIZE(BLOCK_B0_SIZE + frame->_frame->size));
	memcpy(b0_msg, block_b0, BYTE_SIZE(BLOCK_B0_SIZE));
	memcpy(b0_msg + BLOCK_B0_SIZE, frame->_frame->data, BYTE_SIZE(frame->_frame->size));

	calculate_mic(nwk_skey, b0_msg, BLOCK_B0_SIZE + frame->_frame->size, frame->_frame->mic);
	free(b0_msg);
	
	memcpy(pdata, frame->_frame->mic, BYTE_SIZE(MIC_SIZE));
	frame->_frame->size += MIC_SIZE;
	pdata += MIC_SIZE;
}

MacFrame* MacFrame_create(FrameType frame_type, FrameHeader* fhdr)
{
	MacFrame* frame = malloc(sizeof(MacFrame));
	frame->instance = frame;

	frame->_frame = Frame_create(frame_type);
	frame->_frame->instance = frame->instance;

	frame->_iframe = frame->_frame->_iframe;
	frame->_iframe->extract = &MacFrame_extract;

	frame->payload = MacPayload_create(fhdr);

	return frame;
}

void MacFrame_destroy(MacFrame* frame)
{
	MacPayload_destroy(frame->payload);
	
	Frame_destroy(frame->_frame);
	free(frame);
}

////////////////////////////////////////////////////////////////////////////////

void JoinRequestFrame_extract(JoinRequestFrame* frame, unsigned char* app_key)
{
	unsigned char* pdata = frame->_frame->data;

	memcpy(pdata, frame->_frame->mhdr, BYTE_SIZE(MHDR_SIZE));
	frame->_frame->size += MHDR_SIZE;
	pdata += MHDR_SIZE;

	frame->payload->_ipayload->extract(frame->payload->instance);
	memcpy(pdata, frame->payload->_payload->data, BYTE_SIZE(frame->payload->_payload->size));
	frame->_frame->size += frame->payload->_payload->size;
	pdata += frame->payload->_payload->size;

	// TODO: check size < MAXIMUM_MACPAYLOAD_SIZE
	calculate_mic(app_key, frame->_frame->data, frame->_frame->size, frame->_frame->mic);
	memcpy(pdata, frame->_frame->mic, BYTE_SIZE(MIC_SIZE));
	frame->_frame->size += MIC_SIZE;
	pdata += MIC_SIZE;
}

void JoinRequestFrame_destroy(JoinRequestFrame* frame)
{
	JoinRequestPayload_destroy(frame->payload);

	Frame_destroy(frame->_frame);
	free(frame);
}

JoinRequestFrame* JoinRequestFrame_create(
	unsigned char* join_eui,
	unsigned char* dev_eui,
	unsigned char* dev_nonce)
{
	JoinRequestFrame* frame = malloc(sizeof(JoinRequestFrame));
	frame->instance = frame;

	frame->_frame = Frame_create(JoinRequest);
	frame->_frame->instance = frame->instance;

	frame->_iframe = frame->_frame->_iframe;
	frame->_iframe->extract = &JoinRequestFrame_extract;

	frame->payload = JoinRequestPayload_create(join_eui, dev_eui, dev_nonce);
	
	return frame;
}

////////////////////////////////////////////////////////////////////////////////

short int JoinAcceptFrame_validate(
	JoinAcceptFrame* frame,
	unsigned char* app_key)
{
	short int result = 0;
	unsigned char* data = malloc(BYTE_SIZE(frame->_frame->size));
	memcpy(data, frame->_frame->data, MHDR_SIZE);
	aes128_encrypt(
		app_key, 
		frame->_frame->data + MHDR_SIZE, 
		data + MHDR_SIZE, 
		frame->_frame->size - MHDR_SIZE);

	unsigned char mic[MIC_SIZE];
	calculate_mic(app_key, data, frame->_frame->size - MIC_SIZE, mic);
	if (memcmp(data + frame->_frame->size - MIC_SIZE, mic, MIC_SIZE) != 0) 
	{
		result = INVALID_MIC;
	}

	if (result == 0)
	{
		Payload* _payload = Payload_create_by_data(
			frame->_frame->size - MHDR_SIZE - MIC_SIZE, 
			data + MHDR_SIZE);
		frame->payload = JoinAcceptPayload_create_by_payload(_payload);
		result = frame->payload->_ipayload->validate(frame->payload);
	}

	free(data);
	return result;
}

JoinAcceptFrame* JoinAcceptFrame_create_by_frame(Frame* i_frame)
{
	JoinAcceptFrame* frame = malloc(sizeof(JoinAcceptFrame));
	frame->instance = frame;

	frame->_frame = i_frame;
	frame->_frame->instance = frame->instance;

	frame->_iframe = frame->_frame->_iframe;
	frame->_iframe->extract = &JoinAcceptFrame_extract;
	frame->_iframe->validate = &JoinAcceptFrame_validate;

	return frame;
}

void JoinAcceptFrame_extract(JoinAcceptFrame* frame, unsigned char* app_key)
{
	unsigned char* data = malloc(BYTE_SIZE(MAXIMUM_MACPAYLOAD_SIZE));
	memset(data, 0x0, BYTE_SIZE(MAXIMUM_MACPAYLOAD_SIZE));
	unsigned char* pdata = data;

	memcpy(pdata, frame->_frame->mhdr, BYTE_SIZE(MHDR_SIZE));
	frame->_frame->size += MHDR_SIZE;
	pdata += MHDR_SIZE;

	frame->payload->_ipayload->extract(frame->payload->instance);
	memcpy(pdata, frame->payload->_payload->data, BYTE_SIZE(frame->payload->_payload->size));
	frame->_frame->size += frame->payload->_payload->size;
	pdata += frame->payload->_payload->size;

	// TODO: check size < MAXIMUM_MACPAYLOAD_SIZE
	calculate_mic(app_key, data, frame->_frame->size, frame->_frame->mic);
	memcpy(pdata, frame->_frame->mic, BYTE_SIZE(MIC_SIZE));
	frame->_frame->size += MIC_SIZE;
	pdata += MIC_SIZE;

	memcpy(frame->_frame->data, data, BYTE_SIZE(MHDR_SIZE));
	aes128_decrypt(app_key, data + MHDR_SIZE, frame->_frame->data + MHDR_SIZE, frame->_frame->size - MHDR_SIZE);
	free(data);
}

JoinAcceptFrame* JoinAcceptFrame_create(
	unsigned int join_nonce, 
	unsigned char* net_id, 
	unsigned char* dev_addr, 
	DLSettings* dl_settings, 
	short int rx_delay, 
	CFList* cf_list)
{
	JoinAcceptFrame* frame = malloc(sizeof(JoinAcceptFrame));
	frame->instance = frame;

	frame->_frame = Frame_create(JoinAccept);
	frame->_frame->instance = frame->instance;

	frame->_iframe = frame->_frame->_iframe;
	frame->_iframe->extract = &JoinAcceptFrame_extract;
	frame->_iframe->validate = &JoinAcceptFrame_validate;
	
	frame->payload = JoinAcceptPayload_create(join_nonce, net_id, dev_addr, dl_settings, rx_delay, cf_list);

	return frame;
}

void JoinAcceptFrame_destroy(JoinAcceptFrame* frame)
{
	JoinAcceptPayload_destroy(frame->payload);
	
	Frame_destroy(frame->_frame);
	free(frame);
}