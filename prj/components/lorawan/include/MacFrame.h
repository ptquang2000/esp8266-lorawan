#ifndef MAC_FRAME_H
#define MAC_FRAME_H

#include "string.h"
#include "LoraDevice.h"
#include "MacPayload.h"
#include "MacCommand.h"

#define LORA_WAN_R1					0x00


typedef enum FrameType_enum
{
	JoinRequest				= 0b000,
	JoinAccept				= 0b001,
	UnconfirmedDataUplink	= 0b010,
	UnconfirmDataDownlink	= 0b011,
	ConfirmedDataUplink		= 0b100,
	ConfirmedDataDownlink	= 0b101,
	Proprietary				= 0b111
} FrameType;

typedef struct IMacFrame_struct
{
	void (*extract)();
} IMacFrame;

typedef struct MacFrame_struct
{
	unsigned char* mhdr;
	Payload* payload;
	unsigned char* mic;

	FrameType type;

	short int size;
	unsigned char data[MAXIMUM_PHYPAYLOAD_SIZE];

	IMacFrame* _iframe;
	void* instance;
} MacFrame;

MacFrame* MacFrame_create(FrameType frame_type, FrameHeader* fhdr);
void MacFrame_destroy(MacFrame* frame);

typedef struct JoinRequestFrame_struct
{
	JoinRequestPayload* payload;

	IMacFrame* _iframe;
	MacFrame* _frame;
	void* instance;
} JoinRequestFrame;

JoinRequestFrame* JoinRequestFrame_create(LoraDevice* device);
void JoinRequestFrame_destroy(JoinRequestFrame* frame);

typedef struct JoinAcceptFrame_struct
{
	JoinAcceptPayload* payload;

	IMacFrame* _iframe;
	MacFrame* _frame;
	void* instance;
} JoinAcceptFrame;

JoinAcceptFrame* JoinAcceptFrame_create(
	unsigned int join_nonce, 
	unsigned char* net_id, 
	unsigned char* dev_addr, 
	DLSettings* setting, 
	short int rx_delay, 
	CFList* cf_list);
void JoinAcceptFrame_destroy(JoinAcceptFrame* frame);


#endif // MAC_FRAME_H