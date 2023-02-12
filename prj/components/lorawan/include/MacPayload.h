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
	uint32_t fre_ch3;
	uint32_t fre_ch4;
	uint32_t fre_ch5;
	uint32_t fre_ch6;
	uint32_t fre_ch7;
} CFList;

typedef struct IPayload_struct
{
	void (*extract)();
	uint16_t (*validate)();
} IPayload;

typedef struct Payload_struct
{
	uint16_t size;
	uint8_t data[MAXIMUM_MACPAYLOAD_SIZE];
	
	IPayload* _ipayload;
	void* instance;
} Payload;

uint16_t Payload_validate(Payload* payload);
Payload* Payload_create_by_data(uint16_t size, uint8_t* data);
void Payload_extract(Payload* payload);
Payload* Payload_create();
void Payload_destroy(Payload* payload);

typedef struct FrameHeader_struct
{
	uint8_t dev_addr[DEV_ADDR_SIZE];
	
	uint16_t is_adr;
	uint16_t is_adr_ack_req;
	uint16_t is_ack;
	uint16_t fpending;
	uint16_t fopts_len;

	uint16_t frame_counter;

	uint8_t fopts[MAXIMUM_FRAME_OPTIONS_SIZE];
} FrameHeader;

void FrameHeader_insert_cmd(FrameHeader* fhdr, MacCommand* cmd);

typedef struct MacPayload_struct
{
	FrameHeader* fhdr;
	uint8_t* fport;
	uint8_t* frm_payload;
	uint16_t frm_payload_len;

	IPayload* _ipayload;
	Payload* _payload;
	void* instance;
} MacPayload;

uint16_t MacPayload_validate(MacPayload* payload, 
	uint8_t* nwk_skey,
	uint8_t* app_skey,
	uint16_t direction);
void MacPayload_set_fport(MacPayload* payload, uint16_t fport);
void MacPayload_set_app_payload(
	MacPayload* payload, 
	uint16_t fport, 
	int len,
	uint8_t* app_payload);
void MacPayload_set_commands_to_payload(
	MacPayload* payload, 
	int len, 
	MacCommand** mac_commands);
void MacPayload_extract(
	MacPayload* payload, 
	uint8_t* nwk_skey,
	uint8_t* app_skey,
	uint16_t direction);
void MacPayload_destroy(MacPayload* payload);
MacPayload* MacPayload_create_by_payload(Payload* payload);
MacPayload* MacPayload_create(FrameHeader* fhdr);

typedef struct JoinRequestPayload_struct
{
	uint8_t* join_eui;
	uint8_t* dev_eui;
	uint8_t* dev_nonce;

	IPayload* _ipayload;
	Payload* _payload;
	void* instance;
} JoinRequestPayload;

void JoinRequestPayload_extract(JoinRequestPayload* payload);
void JoinRequestPayload_destroy(JoinRequestPayload* payload);
JoinRequestPayload* JoinRequestPayload_create(
	uint8_t* join_eui,
	uint8_t* dev_eui,
	uint8_t* dev_nonce);

typedef struct JoinAcceptPayload_struct
{
	uint8_t* join_nonce;
	uint8_t* net_id;
	uint8_t* dev_addr;
	uint8_t* dl_settings;
	uint8_t* rx_delay;
	uint8_t* cf_list;

	IPayload* _ipayload;
	Payload* _payload;
	void* instance;
} JoinAcceptPayload;

void JoinAcceptPayload_extract(JoinAcceptPayload* payload);
uint16_t JoinAcceptPayload_validate(JoinAcceptPayload* payload);
void JoinAcceptPayload_destroy(JoinAcceptPayload* payload);
JoinAcceptPayload* JoinAcceptPayload_create_by_payload(Payload* payload);
JoinAcceptPayload* JoinAcceptPayload_create(
	uint32_t join_nonce, 
	uint8_t* net_id, 
	uint8_t* dev_addr, 
	DLSettings* dl_settings, 
	uint16_t rx_delay, 
	CFList* cf_list);


#endif // MAC_PAYLOAD_h