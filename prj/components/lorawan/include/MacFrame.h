#ifndef MAC_FRAME_H
#define MAC_FRAME_H

#include "string.h"
#include "LoraDevice.h"
#include "MacPayload.h"
#include "MacCommand.h"

#define LORA_WAN_R1					0x00


typedef enum FrameType_enum
{
	JOIN_REQUEST				= 0x000,
	JOIN_ACCEPT					= 0x001,
	UNCONFIRMED_DATA_UPLINK		= 0x010,
	UNCONFIRMED_DATA_DOWNLINK	= 0x011,
	CONFIRMED_DATA_UPLINK		= 0x100,
	CONFIRMED_DATA_DOWNLINK		= 0x101,
	PROPRIETARY					= 0x111
} FrameType;

typedef struct IMacFrame_struct
{
	void (*extract)(void*, unsigned char*);
} IMacFrame;

typedef struct MacFrame_struct
{
	unsigned char* mhdr;
	Payload* payload;
	unsigned char* mic;

	short int size;
	unsigned char data[MAXIMUM_PHYPAYLOAD_SIZE];

	IMacFrame* _iframe;
	void* instance;
} MacFrame;

MacFrame* MacFrame_create(FrameType);
void MacFrame_destroy(MacFrame*);

typedef struct JoinRequestFrame_struct
{
	JoinRequestPayload* payload;

	IMacFrame* _iframe;
	MacFrame* _frame;
	void* instance;
} JoinRequestFrame;

JoinRequestFrame* JoinRequestFrame_create(
	LoraDevice*, 
	short int);
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


#endif // MAC_FRAME_H
