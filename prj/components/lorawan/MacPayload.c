#include "MacPayload.h"
#include "LoraDevice.h"
#include "LoraUtil.h"

#define BLOCK_A_SIZE			16
#define BLOCK_A_DIR_BYTE		5
#define BLOCK_A_DIR_SIZE		1
#define BLOCK_A_DEV_ADDR_BYTE	6
#define BLOCK_A_DEV_ADDR_SIZE	4
#define BLOCK_A_FCNT_BYTE		10
#define BLOCK_A_FCNT_SIZE		4
#define BLOCK_A_INDEX_BYTE		15
#define BLOCK_A_INDEX_SIZE		1

static unsigned char block_a[BLOCK_A_SIZE] = {0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

////////////////////////////////////////////////////////////////////////////////

void Payload_extract(Payload* payload)
{
    
}

Payload* Payload_create()
{
	Payload* payload = malloc(sizeof(Payload));
    payload->instance = payload;
	payload->size = 0;

	payload->_ipayload = malloc(sizeof(IPayload));
    payload->_ipayload->extract = &Payload_extract;

	return payload;
}

void Payload_destroy(Payload* payload)
{
	if (payload == NULL) return;
	free(payload->_ipayload);
	free(payload);
	payload = NULL;
}

////////////////////////////////////////////////////////////////////////////////

void FrameHeader_insert_cmd(FrameHeader* fhdr, MacCommand* cmd)
{
	cmd->_icmd->extract(cmd->instance);
	memcpy(fhdr->fopts + fhdr->fopts_len, cmd->data, BYTE_SIZE(cmd->size));
	fhdr->fopts_len += cmd->size;
	ESP_ERROR_CHECK(fhdr->fopts_len < 0 || fhdr->fopts_len > 15);
}

////////////////////////////////////////////////////////////////////////////////

void MacPayload_extract(
	MacPayload* payload, 
	LoraDevice* device,
	short int* pdir)
{
	memset(&block_a[BLOCK_A_DIR_BYTE], *pdir, BYTE_SIZE(BLOCK_A_DIR_SIZE)); 
	memcpy(&block_a[BLOCK_A_DEV_ADDR_BYTE], payload->fhdr->dev_addr, BYTE_SIZE(DEV_ADDR_SIZE)); 
	memset(&block_a[BLOCK_A_FCNT_BYTE], payload->fhdr->frame_counter, BYTE_SIZE(FRAME_COUNTER_SIZE)); 

	unsigned char* pdata = payload->_payload->data;
	memcpy(pdata, payload->fhdr->dev_addr, BYTE_SIZE(DEV_ADDR_SIZE));
	pdata += DEV_ADDR_SIZE;
	payload->_payload->size += DEV_ADDR_SIZE;
	short int fctrl = 	SET_BITS(payload->fhdr->is_adr, 1, FCTRL_ADR_BIT) | 
						SET_BITS(payload->fhdr->is_adr_ack_req, 1, FCTRL_ADR_ACK_REQ_BIT) | 
						SET_BITS(payload->fhdr->is_ack, 1, FCTRL_ACK_BIT) |
						SET_BITS(payload->fhdr->is_classB, 1, FCTRL_CLASSB_BIT ) |
						SET_BITS(payload->fhdr->fopts_len, FCTRL_FOPTS_LEN_BITS, FCTRL_FOPTS_LEN_OFFSET);
	memset(pdata, fctrl, BYTE_SIZE(FRAME_CONTROL_SIZE));
	pdata += FRAME_CONTROL_SIZE;
	payload->_payload->size += FRAME_CONTROL_SIZE;
	memset(pdata, payload->fhdr->frame_counter, BYTE_SIZE(FRAME_COUNTER_SIZE));
	pdata += FRAME_COUNTER_SIZE;
	payload->_payload->size += FRAME_COUNTER_SIZE;

	if (payload->fhdr->fopts_len != 0)
	{
		memcpy(pdata, payload->fhdr->fopts, BYTE_SIZE(payload->fhdr->fopts_len));
		pdata += payload->fhdr->fopts_len;
		payload->_payload->size += payload->fhdr->fopts_len;
	}
	free(payload->fhdr);

	if (payload->fport)
	{
		short int fport = payload->fport[0];
		memcpy(pdata, payload->fport, BYTE_SIZE(FRAME_PORT_SIZE));
		pdata += FRAME_PORT_SIZE;
		payload->_payload->size += FRAME_PORT_SIZE;
		free(payload->fport);

		unsigned char* key = fport == 0 ? device->nwk_skey : device->app_skey;
		short int k = payload->frm_payload_len / 16 + 1;
		for (int i = 1; i <= k; i++)
		{
			block_a[BLOCK_A_INDEX_BYTE] = k;
			unsigned char block_s[BLOCK_A_SIZE];
			aes128_encrypt(key, block_a, block_s, BLOCK_A_SIZE);
			for (int j = 0; j < BLOCK_A_SIZE; j++)
			{
				pdata[j] = payload->frm_payload[j] ^ block_s[j];
			}
			pdata += BLOCK_A_SIZE;
		}
		payload->_payload->size += payload->frm_payload_len;
	}
}

void MacPayload_set_app_payload(
	MacPayload* payload, 
	short int fport,
	int len,
	unsigned char* app_payload)
{
	ESP_ERROR_CHECK(fport == 0);
	ESP_ERROR_CHECK(payload->fport != NULL && (short int)(*payload->fport) == 0);

	payload->frm_payload = malloc(sizeof(unsigned char) * len);
	memcpy(payload->frm_payload, app_payload, sizeof(unsigned char) * len);
	payload->frm_payload_len = len;

	payload->fport = malloc(BYTE_SIZE(FRAME_PORT_SIZE));
	memset(payload->fport, fport, BYTE_SIZE(FRAME_PORT_SIZE));
}

void MacPayload_set_commands_to_payload(
	MacPayload* payload, 
	int len, 
	unsigned char* mac_commands)
{
	ESP_ERROR_CHECK(payload->fhdr->fopts_len > 0);

	payload->frm_payload = malloc(sizeof(unsigned char) * len);
	memcpy(payload->frm_payload, mac_commands, sizeof(unsigned char) * len);
	payload->frm_payload_len = len;
	
	payload->fport = malloc(BYTE_SIZE(FRAME_PORT_SIZE));
	memset(payload->fport, 0x0, BYTE_SIZE(FRAME_PORT_SIZE));
}

void MacPayload_destroy(MacPayload* payload)
{
	if (payload == NULL) return;
	free(payload->fhdr);
	free(payload->fport);
	free(payload->frm_payload);
	Payload_destroy(payload->_payload);
	free(payload);
	payload = NULL;
}

MacPayload* MacPayload_create(FrameHeader* fhdr)
{
	ESP_ERROR_CHECK(fhdr->fopts_len < 0 || fhdr->fopts_len > MAXIMUM_FRAME_OPTIONS_SIZE);
	
	MacPayload* payload = malloc(sizeof(MacPayload));
	payload->instance = payload;

	payload->fhdr = malloc(sizeof(FrameHeader));
	memcpy(payload->fhdr, fhdr, sizeof(FrameHeader));
	payload->fport = NULL;
	payload->frm_payload = NULL;

	payload->_payload = Payload_create();
	payload->_payload->instance = payload->instance;

	payload->_ipayload = payload->_payload->_ipayload;
	payload->_ipayload->extract = &MacPayload_extract;

	return payload;
}

////////////////////////////////////////////////////////////////////////////////

void JoinRequestPayload_extract(JoinRequestPayload* payload)
{
	unsigned char* pdata = payload->_payload->data;

	memcpy(pdata, payload->join_eui, BYTE_SIZE(JOIN_EUI_SIZE));
	pdata += JOIN_EUI_SIZE;
	payload->_payload->size += JOIN_EUI_SIZE;
	free(payload->join_eui);

	memcpy(pdata, payload->dev_eui, BYTE_SIZE(DEV_EUI_SIZE));
	pdata += DEV_EUI_SIZE;
	payload->_payload->size += DEV_EUI_SIZE;
	free(payload->dev_eui);
	
	memcpy(pdata, payload->dev_nonce, BYTE_SIZE(DEV_NONCE_SIZE));
	pdata += DEV_NONCE_SIZE;
	payload->_payload->size += DEV_NONCE_SIZE;
	free(payload->dev_nonce);
}

void JoinRequestPayload_destroy(JoinRequestPayload* payload)
{
	if (payload == NULL) return;
	free(payload->join_eui);
	free(payload->dev_eui);
	free(payload->dev_nonce);
    Payload_destroy(payload->_payload);
    free(payload);
}

JoinRequestPayload* JoinRequestPayload_create(LoraDevice* device)
{
	JoinRequestPayload* payload = malloc(sizeof(JoinRequestPayload));
	payload->instance = payload;

	payload->join_eui = malloc(BYTE_SIZE(JOIN_EUI_SIZE));
	payload->dev_eui = malloc(BYTE_SIZE(DEV_EUI_SIZE));
	payload->dev_nonce = malloc(BYTE_SIZE(DEV_NONCE_SIZE));

	memcpy(payload->join_eui, device->join_eui, BYTE_SIZE(JOIN_EUI_SIZE));
	memcpy(payload->dev_eui, device->dev_eui, BYTE_SIZE(DEV_EUI_SIZE));
	memcpy(payload->dev_nonce, device->dev_nonce, BYTE_SIZE(DEV_NONCE_SIZE));

	payload->_payload = Payload_create();
    payload->_payload->instance = payload->instance;

	payload->_ipayload = payload->_payload->_ipayload;
	payload->_ipayload->extract = &JoinRequestPayload_extract;

	return payload;
}

////////////////////////////////////////////////////////////////////////////////

void JoinAcceptPayload_set_join_nonce(JoinAcceptPayload* payload, unsigned int join_nonce)
{
	payload->join_nonce[2] = (unsigned char)(join_nonce >> 16);
	payload->join_nonce[1] = (unsigned char)(join_nonce >> 8);
	payload->join_nonce[0] = (unsigned char)(join_nonce);
}

void JoinAcceptPayload_extract(JoinAcceptPayload* payload)
{
	unsigned char* pdata = payload->_payload->data;

	memcpy(pdata, payload->join_nonce, BYTE_SIZE(JOIN_NONCE_SIZE));
	pdata += JOIN_NONCE_SIZE;
	payload->_payload->size += JOIN_NONCE_SIZE;
	free(payload->join_nonce);

	memcpy(pdata, payload->net_id, BYTE_SIZE(NET_ID_SIZE));
	pdata += NET_ID_SIZE;
	payload->_payload->size += NET_ID_SIZE;
	free(payload->net_id);
	
	memcpy(pdata, payload->dev_addr, BYTE_SIZE(DEV_ADDR_SIZE));
	pdata += DEV_ADDR_SIZE;
	payload->_payload->size += DEV_ADDR_SIZE;
	free(payload->dev_addr);

	memcpy(pdata, payload->dl_settings, BYTE_SIZE(DLSETTINGS_SIZE));
	pdata += DLSETTINGS_SIZE;
	payload->_payload->size += DLSETTINGS_SIZE;
	free(payload->dl_settings);

	if (payload->cf_list)
	{
		memcpy(pdata, payload->cf_list, BYTE_SIZE(CFLIST_SIZE));
		pdata += CFLIST_SIZE;
		payload->_payload->size += CFLIST_SIZE;
		free(payload->cf_list);
	}

	memcpy(pdata, payload->rx_delay, BYTE_SIZE(RX_DELAY_SIZE));
	pdata += RX_DELAY_SIZE;
	payload->_payload->size += RX_DELAY_SIZE;
	free(payload->rx_delay);
}

void JoinAcceptPayload_destroy(JoinAcceptPayload* payload)
{
	if (payload == NULL) return;
	free(payload->join_nonce);
	free(payload->net_id);
	free(payload->dev_addr);
	free(payload->rx_delay);
	free(payload->dl_settings);
	free(payload->cf_list);
    Payload_destroy(payload->_payload);
    free(payload);
	payload = NULL;
}

JoinAcceptPayload* JoinAcceptPayload_create(
	unsigned int join_nonce, 
	unsigned char* net_id, 
	unsigned char* dev_addr, 
	DLSettings* dl_settings, 
	short int rx_delay, 
	CFList* cf_list)
{
	JoinAcceptPayload* payload = malloc(sizeof(JoinAcceptPayload));
    payload->instance = payload;

	payload->join_nonce = malloc(BYTE_SIZE(JOIN_NONCE_SIZE));
	payload->net_id = malloc(BYTE_SIZE(NET_ID_SIZE));
	payload->dev_addr = malloc(BYTE_SIZE(DEV_ADDR_SIZE));
	payload->dl_settings = malloc(BYTE_SIZE(DLSETTINGS_SIZE));
	
    JoinAcceptPayload_set_join_nonce(payload, join_nonce);
	memcpy(payload->net_id, net_id, sizeof(unsigned char)* NET_ID_SIZE);
	memcpy(payload->dev_addr, dev_addr, sizeof(unsigned char)* DEV_ADDR_SIZE);
	payload->dl_settings[0] = SET_BITS(dl_settings->rx1_dr_offset, RX1DROFFSET_BITS, RX1DROFFSET_OFFSET) |
						SET_BITS(dl_settings->rx2_data_rate, RX2DATARATE_BITS, RX2DATARATE_OFFSET);
	if (cf_list)
	{
		payload->cf_list = malloc(BYTE_SIZE(CFLIST_SIZE));
		memset(&payload->cf_list[FRE_CH3_OFFSET], BIT_MASK(cf_list->fre_ch3, 12), BYTE_SIZE(FRE_CHX_SIZE));
		memset(&payload->cf_list[FRE_CH4_OFFSET], BIT_MASK(cf_list->fre_ch4, 12), BYTE_SIZE(FRE_CHX_SIZE));
		memset(&payload->cf_list[FRE_CH5_OFFSET], BIT_MASK(cf_list->fre_ch5, 12), BYTE_SIZE(FRE_CHX_SIZE));
		memset(&payload->cf_list[FRE_CH6_OFFSET], BIT_MASK(cf_list->fre_ch6, 12), BYTE_SIZE(FRE_CHX_SIZE));
		memset(&payload->cf_list[FRE_CH7_OFFSET], BIT_MASK(cf_list->fre_ch7, 12), BYTE_SIZE(FRE_CHX_SIZE));
		memset(&payload->cf_list[CFLIST_TYPE_OFFSET], 0x0, BYTE_SIZE(FRE_CHX_SIZE));
	}
	else
	{
        payload->cf_list = NULL;
	}
	payload->rx_delay = malloc(BYTE_SIZE(RX_DELAY_SIZE));
	memset(payload->rx_delay, rx_delay, BYTE_SIZE(RX_DELAY_SIZE));

	payload->_payload = Payload_create();
	payload->_payload->size = 0;
	payload->_payload->instance = payload->instance;

	payload->_ipayload = payload->_payload->_ipayload;
	payload->_ipayload->extract = &JoinAcceptPayload_extract;

	return payload;
}

////////////////////////////////////////////////////////////////////////////////