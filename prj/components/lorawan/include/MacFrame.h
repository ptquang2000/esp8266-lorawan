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
	short int (*validate)();
} IFrame;

typedef struct Frame_struct
{
	FrameType type;
	unsigned char* mhdr;
	unsigned char* mic;

	short int size;
	unsigned char data[MAXIMUM_PHYPAYLOAD_SIZE];

	IFrame* _iframe;
	void* instance;
} Frame;

short int Frame_get_version(Frame* frame);
short int Frame_validate(Frame* frame);
void Frame_extract(Frame* frame);
void Frame_destroy(Frame* frame);
Frame* Frame_create_by_data(short int size, unsigned char* data);
Frame* Frame_create(FrameType frame_type);

typedef struct MacFrame_struct
{
	MacPayload* payload;

	IFrame* _iframe;
	Frame* _frame;
	void* instance;
} MacFrame;

short int MacFrame_validate(
	MacFrame* frame,
	short int dir,
	unsigned char* dev_addr, 
	short int frame_counter,
	unsigned char* nwk_skey);
void MacFrame_extract(
	MacFrame* frame,
	unsigned char* nwk_skey,
	unsigned char* app_skey);
void MacFrame_destroy(MacFrame* frame);
MacFrame* MacFrame_create(FrameType frame_type, FrameHeader* fhdr);

typedef struct JoinRequestFrame_struct
{
	JoinRequestPayload* payload;

	IFrame* _iframe;
	Frame* _frame;
	void* instance;
} JoinRequestFrame;

void JoinRequestFrame_extract(JoinRequestFrame* frame, unsigned char* app_key);
void JoinRequestFrame_destroy(JoinRequestFrame* frame);
JoinRequestFrame* JoinRequestFrame_create(
	unsigned char* join_eui,
	unsigned char* dev_eui,
	unsigned char* dev_nonce);

typedef struct JoinAcceptFrame_struct
{
	JoinAcceptPayload* payload;

	IFrame* _iframe;
	Frame* _frame;
	void* instance;
} JoinAcceptFrame;

short int JoinAcceptFrame_validate(JoinAcceptFrame* frame, unsigned char* app_key);
void JoinAcceptFrame_extract(JoinAcceptFrame* frame, unsigned char* app_key);
void JoinAcceptFrame_destroy(JoinAcceptFrame* frame);
JoinAcceptFrame* JoinAcceptFrame_create_by_frame(Frame* i_frame);
JoinAcceptFrame* JoinAcceptFrame_create(
	unsigned int join_nonce, 
	unsigned char* net_id, 
	unsigned char* dev_addr, 
	DLSettings* setting, 
	short int rx_delay, 
	CFList* cf_list);


#endif // MAC_FRAME_H
