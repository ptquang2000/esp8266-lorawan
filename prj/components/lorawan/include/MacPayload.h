#ifndef MAC_PAYLOAD_H
#define MAC_PAYLOAD_H

#include "MacCommand.h"
#include "LoraDevice.h"

#define MAXIMUM_MACPAYLOAD_SIZE			250
#define MHDR_SIZE						1
#define MIC_SIZE						4

#define JOIN_NONCE_SIZE					3
#define NET_ID_SIZE						3
#define CFLIST_SIZE						16
#define 	FRE_CHX_SIZE				3
#define 	FRE_CH3_OFFSET				0
#define 	FRE_CH4_OFFSET				3
#define 	FRE_CH5_OFFSET				6
#define 	FRE_CH6_OFFSET				9
#define 	FRE_CH7_OFFSET				12
#define 	CFLIST_TYPE_OFFSET			15
#define RX_DELAY_SIZE					1

#define FRAME_CONTROL_SIZE				1
#define 	FCTRL_ADR_BIT				7
#define 	FCTRL_ADR_ACK_REQ_BIT		6			
#define 	FCTRL_ACK_BIT				5
#define 	FCTRL_FPENDING_BIT			4
#define 	FCTRL_CLASSB_BIT			4
#define 	FCTRL_FOPTS_LEN_OFFSET		0
#define 		FCTRL_FOPTS_LEN_BITS	4
#define FRAME_COUNTER_SIZE				2
#define MAXIMUM_FRAME_OPTIONS_SIZE		15
#define FRAME_PORT_SIZE					1
#define MAXIMUM_FRAME_HEADER_SIZE		(DEV_ADDR_SIZE + FRAME_CONTROL_SIZE + FRAME_COUNTER_SIZE + MAXIMUM_FRAME_OPTIONS_SIZE)
#define MAXIMUM_FRAME_PAYLOAD_SIZE		(MAXIMUM_PHYPAYLOAD_SIZE - MAXIMUM_FRAME_HEADER_SIZE - MAXIMUM_FRAME_OPTIONS_SIZE)

#define MAXIMUM_PHYPAYLOAD_SIZE			(MAXIMUM_MACPAYLOAD_SIZE + MHDR_SIZE + MIC_SIZE)
#define MINIMUM_PHYPAYLOAD_SIZE			(MHDR_SIZE + DEV_ADDR_SIZE + FRAME_CONTROL_SIZE + FRAME_COUNTER_SIZE + MIC_SIZE)


typedef struct CFList_struct
{
	unsigned int fre_ch3;
	unsigned int fre_ch4;
	unsigned int fre_ch5;
	unsigned int fre_ch6;
	unsigned int fre_ch7;
} CFList;

typedef struct IPayload_struct
{
	void (*extract)();
	short int (*validate)();
} IPayload;

typedef struct Payload_struct
{
	short int size;
	unsigned char data[MAXIMUM_MACPAYLOAD_SIZE];
	
	IPayload* _ipayload;
	void* instance;
} Payload;

short int Payload_validate(Payload* payload);
Payload* Payload_create_by_data(short int size, unsigned char* data);
void Payload_extract(Payload* payload);
Payload* Payload_create();
void Payload_destroy(Payload* payload);

typedef struct FrameHeader_struct
{
	unsigned char dev_addr[DEV_ADDR_SIZE];
	
	short int is_adr;
	short int is_adr_ack_req;
	short int is_ack;
	short int fpending;
	short int fopts_len;

	short int frame_counter;

	unsigned char fopts[MAXIMUM_FRAME_OPTIONS_SIZE];
} FrameHeader;

void FrameHeader_insert_cmd(FrameHeader* fhdr, MacCommand* cmd);

typedef struct MacPayload_struct
{
	FrameHeader* fhdr;
	unsigned char* fport;
	unsigned char* frm_payload;
	short int frm_payload_len;

	IPayload* _ipayload;
	Payload* _payload;
	void* instance;
} MacPayload;

short int MacPayload_validate(MacPayload* payload, 
	unsigned char* nwk_skey,
	unsigned char* app_skey,
	short int direction);
void MacPayload_set_fport(MacPayload* payload, short int fport);
void MacPayload_set_app_payload(
	MacPayload* payload, 
	short int fport, 
	int len,
	unsigned char* app_payload);
void MacPayload_set_commands_to_payload(
	MacPayload* payload, 
	int len, 
	MacCommand** mac_commands);
void MacPayload_extract(
	MacPayload* payload, 
	unsigned char* nwk_skey,
	unsigned char* app_skey,
	short int direction);
void MacPayload_destroy(MacPayload* payload);
MacPayload* MacPayload_create_by_payload(Payload* payload);
MacPayload* MacPayload_create(FrameHeader* fhdr);

typedef struct JoinRequestPayload_struct
{
	unsigned char* join_eui;
	unsigned char* dev_eui;
	unsigned char* dev_nonce;

	IPayload* _ipayload;
	Payload* _payload;
	void* instance;
} JoinRequestPayload;

void JoinRequestPayload_extract(JoinRequestPayload* payload);
void JoinRequestPayload_destroy(JoinRequestPayload* payload);
JoinRequestPayload* JoinRequestPayload_create(
	unsigned char* join_eui,
	unsigned char* dev_eui,
	unsigned char* dev_nonce);

typedef struct JoinAcceptPayload_struct
{
	unsigned char* join_nonce;
	unsigned char* net_id;
	unsigned char* dev_addr;
	unsigned char* dl_settings;
	unsigned char* rx_delay;
	unsigned char* cf_list;

	IPayload* _ipayload;
	Payload* _payload;
	void* instance;
} JoinAcceptPayload;

void JoinAcceptPayload_extract(JoinAcceptPayload* payload);
short int JoinAcceptPayload_validate(JoinAcceptPayload* payload);
void JoinAcceptPayload_destroy(JoinAcceptPayload* payload);
JoinAcceptPayload* JoinAcceptPayload_create_by_payload(Payload* payload);
JoinAcceptPayload* JoinAcceptPayload_create(
	unsigned int join_nonce, 
	unsigned char* net_id, 
	unsigned char* dev_addr, 
	DLSettings* dl_settings, 
	short int rx_delay, 
	CFList* cf_list);


#endif // MAC_PAYLOAD_h