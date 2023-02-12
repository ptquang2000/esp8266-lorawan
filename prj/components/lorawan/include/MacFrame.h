#ifndef MAC_FRAME_H
#define MAC_FRAME_H

#include "string.h"
#include "MacPayload.h"
#include "MacCommand.h"

#define LORA_WAN_R1							0x00
#define FRAME_VERSION_BITS					3
#define FRAME_VERSION_OFFSET				5
#define FRAME_TYPE_BITS						3
#define FRAME_TYPE_OFFSET					5

#define MAXIMUM_JOIN_ACCEPT_PAYLOAD_SIZE	(JOIN_NONCE_SIZE + NET_ID_SIZE +\
											DEV_ADDR_SIZE + DLSETTINGS_SIZE +\
											RX_DELAY_SIZE + CFLIST_SIZE)

typedef enum FrameType_enum
{
	MIN_FRAMETYPE			= 0b000,
	JoinRequest				= 0b000,
	JoinAccept				= 0b001,
	UnconfirmedDataUplink	= 0b010,
	UnconfirmDataDownlink	= 0b011,
	ConfirmedDataUplink		= 0b100,
	ConfirmedDataDownlink	= 0b101,
	Proprietary				= 0b111,
	MAX_FRAMETYPE			= 0b111,
} FrameType;

typedef struct IFrame_struct
{
	void (*extract)();
	uint16_t (*validate)();
} IFrame;

typedef struct Frame_struct
{
	FrameType type;
	uint8_t* mhdr;
	uint8_t* mic;

	uint16_t size;
	uint8_t data[MAXIMUM_PHYPAYLOAD_SIZE];

	IFrame* _iframe;
	void* instance;
} Frame;

uint16_t Frame_get_version(Frame* frame);
int Frame_validate(Frame* frame);
void Frame_extract(Frame* frame);
void Frame_destroy(Frame* frame);
Frame* Frame_create_by_data(uint16_t size, uint8_t* data);
Frame* Frame_create(FrameType frame_type);

typedef struct MacFrame_struct
{
	MacPayload* payload;

	IFrame* _iframe;
	Frame* _frame;
	void* instance;
} MacFrame;

int MacFrame_validate(
	MacFrame* frame,
	uint8_t* nwk_skey,
	uint8_t* app_skey,
	uint16_t direction,
	uint8_t* dev_addr, 
	uint16_t frame_counter);
MacFrame* MacFrame_create_by_frame(Frame* _frame);
void MacFrame_extract(
	MacFrame* frame,
	uint8_t* nwk_skey,
	uint8_t* app_skey,
	uint16_t direction);
void MacFrame_destroy(MacFrame* frame);
MacFrame* MacFrame_create(FrameType frame_type, FrameHeader* fhdr);

typedef struct JoinRequestFrame_struct
{
	JoinRequestPayload* payload;

	IFrame* _iframe;
	Frame* _frame;
	void* instance;
} JoinRequestFrame;

void JoinRequestFrame_extract(JoinRequestFrame* frame, uint8_t* app_key);
void JoinRequestFrame_destroy(JoinRequestFrame* frame);
JoinRequestFrame* JoinRequestFrame_create(
	uint8_t* join_eui,
	uint8_t* dev_eui,
	uint8_t* dev_nonce);

typedef struct JoinAcceptFrame_struct
{
	JoinAcceptPayload* payload;

	IFrame* _iframe;
	Frame* _frame;
	void* instance;
} JoinAcceptFrame;

int JoinAcceptFrame_validate(JoinAcceptFrame* frame, uint8_t* app_key);
void JoinAcceptFrame_extract(JoinAcceptFrame* frame, uint8_t* app_key);
void JoinAcceptFrame_destroy(JoinAcceptFrame* frame);
JoinAcceptFrame* JoinAcceptFrame_create_by_frame(Frame* i_frame);
JoinAcceptFrame* JoinAcceptFrame_create(
	uint32_t join_nonce, 
	uint8_t* net_id, 
	uint8_t* dev_addr, 
	DLSettings* setting, 
	uint16_t rx_delay, 
	CFList* cf_list);


#endif // MAC_FRAME_H
