#include "MacPayload.h"
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

static uint8_t block_a[BLOCK_A_SIZE] = {
	0x01, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00
};
static uint16_t s_max_app_payload = 242;

////////////////////////////////////////////////////////////////////////////////

void set_max_app_payload(uint16_t dr)
{
	switch (dr)
	{
	case 0: s_max_app_payload = 51;	break;
	case 1: s_max_app_payload = 51;	break;
	case 2: s_max_app_payload = 51;	break;
	case 3: s_max_app_payload = 115; break;
	case 4: s_max_app_payload = 242; break;
	case 5: s_max_app_payload = 242; break;
	case 6: s_max_app_payload = 242; break;
	case 7: s_max_app_payload = 242; break;
	default: ESP_ERROR_CHECK(1); break;
	}
}

////////////////////////////////////////////////////////////////////////////////

void Payload_extract(Payload* payload)
{
    
}

int Payload_validate(Payload* payload)
{
	return -1;
}

Payload* Payload_create_by_data(uint16_t size, uint8_t* data)
{
	Payload* payload = malloc(sizeof(Payload));
    payload->instance = payload;

	payload->size = size;
	memcpy(payload->data, data, payload->size);

	payload->_ipayload = malloc(sizeof(IPayload));
    payload->_ipayload->extract = &Payload_extract;
	payload->_ipayload->validate = &Payload_validate;

	return payload;
}

Payload* Payload_create()
{
	Payload* payload = malloc(sizeof(Payload));
    payload->instance = payload;
	payload->size = 0;

	payload->_ipayload = malloc(sizeof(IPayload));
    payload->_ipayload->extract = &Payload_extract;
	payload->_ipayload->validate = &Payload_validate;

	return payload;
}

void Payload_destroy(Payload* payload)
{
	free(payload->_ipayload);
	free(payload);
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

int MacPayload_validate(MacPayload* payload, 
	uint8_t* nwk_skey,
	uint8_t* app_skey,
	uint16_t direction)
{
	uint16_t size = payload->_payload->size;
	uint8_t* pdata = payload->_payload->data;

	if (size < DEV_ADDR_SIZE + FRAME_CONTROL_SIZE + FRAME_COUNTER_SIZE)
	{
		return INVALID_DATA_SIZE;
	}

	memcpy(payload->fhdr->dev_addr, pdata, BYTE_SIZE(DEV_ADDR_SIZE));
	pdata += DEV_ADDR_SIZE;
	size -= DEV_ADDR_SIZE;
	
	uint16_t fctrl;
	memcpy(&fctrl, pdata, BYTE_SIZE(FRAME_CONTROL_SIZE));
	payload->fhdr->is_adr = GET_BITS(fctrl, 1, FCTRL_ADR_BIT);
	payload->fhdr->is_adr_ack_req = GET_BITS(fctrl, 1, FCTRL_ADR_ACK_REQ_BIT);
	payload->fhdr->is_ack = GET_BITS(fctrl, 1, FCTRL_ACK_BIT);
	payload->fhdr->fpending = GET_BITS(fctrl, 1, FCTRL_CLASSB_BIT);
	payload->fhdr->fopts_len = GET_BITS(fctrl, FCTRL_FOPTS_LEN_BITS, FCTRL_FOPTS_LEN_OFFSET);
	pdata += FRAME_CONTROL_SIZE;
	size -= FRAME_CONTROL_SIZE;

	memcpy(&payload->fhdr->frame_counter, pdata, BYTE_SIZE(FRAME_COUNTER_SIZE));
	pdata += FRAME_COUNTER_SIZE;
	size -= FRAME_COUNTER_SIZE;

	if (size >= payload->fhdr->fopts_len && payload->fhdr->fopts_len != 0)
	{
		memcpy(payload->fhdr->fopts, pdata, BYTE_SIZE(payload->fhdr->fopts_len));
		pdata += payload->fhdr->fopts_len;
		size -= payload->fhdr->fopts_len;
	}

	if (size >= FRAME_PORT_SIZE)
	{
		payload->fport = malloc(BYTE_SIZE(FRAME_PORT_SIZE));
		memcpy(payload->fport, pdata, BYTE_SIZE(FRAME_PORT_SIZE));
		pdata += FRAME_PORT_SIZE;
		size -= FRAME_PORT_SIZE;
	}

	if (size > 0 && payload->fport != NULL)
	{

		if (size > s_max_app_payload - payload->fhdr->fopts_len)
		{
			return INVALID_DATA_SIZE;
		}
		uint16_t fport = payload->fport[0];
		uint8_t* key = fport == 0 ? nwk_skey : app_skey;
		payload->frm_payload_len = size;
		payload->frm_payload = malloc(BYTE_SIZE(size));

		memset(
			&block_a[BLOCK_A_DIR_BYTE], 
			direction & 1, 
			BYTE_SIZE(BLOCK_A_DIR_SIZE)
		); 
		memcpy(
			&block_a[BLOCK_A_DEV_ADDR_BYTE], 
			payload->fhdr->dev_addr, 
			BYTE_SIZE(DEV_ADDR_SIZE)
		);
		memcpy(
			&block_a[BLOCK_A_FCNT_BYTE], 
			&payload->fhdr->frame_counter, 
			BYTE_SIZE(FRAME_COUNTER_SIZE)
		);

		uint16_t k = size / 16 + 1;
		uint16_t len_pld = 0;
		for (int i = 1; i <= k; i++)
		{
			block_a[BLOCK_A_INDEX_BYTE] = i;
			uint8_t block_s[BLOCK_A_SIZE];
			aes128_encrypt(key, block_a, block_s, BLOCK_A_SIZE);

			for (int j = 0; j < BLOCK_A_SIZE; j++)
			{
				payload->frm_payload[j] = pdata[j] ^ block_s[j];
				if (++len_pld == payload->frm_payload_len) break;
			}
			pdata += BLOCK_A_SIZE;
		}
		size -= payload->frm_payload_len;
	}

	if (size != 0)
	{
		return INVALID_DATA_SIZE;
	}

	return 0;
}

MacPayload* MacPayload_create_by_payload(Payload* _payload)
{
	MacPayload* payload = malloc(sizeof(MacPayload));
	payload->instance = payload;

	payload->fhdr = malloc(sizeof(FrameHeader));
	payload->fport = NULL;
	payload->frm_payload = NULL;

	payload->_payload = _payload;
	payload->_payload->instance = payload->instance;

	payload->_ipayload = payload->_payload->_ipayload;
	payload->_ipayload->extract = &MacPayload_extract;
	payload->_ipayload->validate = &MacPayload_validate;

	payload->frm_payload_len = 0;

	return payload;
}

void MacPayload_extract(
	MacPayload* payload, 
	uint8_t* nwk_skey,
	uint8_t* app_skey,
	uint16_t direction)
{
	uint8_t* pdata = payload->_payload->data;
	memcpy(pdata, payload->fhdr->dev_addr, BYTE_SIZE(DEV_ADDR_SIZE));
	pdata += DEV_ADDR_SIZE;
	payload->_payload->size += DEV_ADDR_SIZE;
	
	uint16_t fctrl = 	
		SET_BITS(payload->fhdr->is_adr, 1, FCTRL_ADR_BIT) | 
		SET_BITS(payload->fhdr->is_adr_ack_req, 1, FCTRL_ADR_ACK_REQ_BIT) | 
		SET_BITS(payload->fhdr->is_ack, 1, FCTRL_ACK_BIT) |
		SET_BITS(payload->fhdr->fpending, 1, FCTRL_CLASSB_BIT ) |
		SET_BITS(payload->fhdr->fopts_len, FCTRL_FOPTS_LEN_BITS, FCTRL_FOPTS_LEN_OFFSET);
	memset(pdata, fctrl, BYTE_SIZE(FRAME_CONTROL_SIZE));
	pdata += FRAME_CONTROL_SIZE;
	payload->_payload->size += FRAME_CONTROL_SIZE;

	memcpy(pdata, &payload->fhdr->frame_counter, BYTE_SIZE(FRAME_COUNTER_SIZE));
	pdata += FRAME_COUNTER_SIZE;
	payload->_payload->size += FRAME_COUNTER_SIZE;

	if (payload->fhdr->fopts_len != 0)
	{
		memcpy(pdata, payload->fhdr->fopts, BYTE_SIZE(payload->fhdr->fopts_len));
		pdata += payload->fhdr->fopts_len;
		payload->_payload->size += payload->fhdr->fopts_len;
	}

	if (payload->fport)
	{
		memcpy(pdata, payload->fport, BYTE_SIZE(FRAME_PORT_SIZE));
		pdata += FRAME_PORT_SIZE;
		payload->_payload->size += FRAME_PORT_SIZE;
	}

	if (payload->fport && payload->frm_payload)
	{
		uint16_t fport = payload->fport[0];
		uint8_t* key = fport == 0 ? nwk_skey : app_skey;

		memset(
			&block_a[BLOCK_A_DIR_BYTE], 
			direction & 1, 
			BYTE_SIZE(BLOCK_A_DIR_SIZE)
		); 
		memcpy(
			&block_a[BLOCK_A_DEV_ADDR_BYTE], 
			payload->fhdr->dev_addr, 
			BYTE_SIZE(DEV_ADDR_SIZE)
		);
		memcpy(
			&block_a[BLOCK_A_FCNT_BYTE], 
			&payload->fhdr->frame_counter, 
			BYTE_SIZE(FRAME_COUNTER_SIZE)
		); 

		uint16_t k = payload->frm_payload_len / 16 + 1;
		uint16_t len_pld = 0;
		for (int i = 1; i <= k; i++)
		{
			block_a[BLOCK_A_INDEX_BYTE] = i;
			uint8_t block_s[BLOCK_A_SIZE];
			aes128_encrypt(key, block_a, block_s, BLOCK_A_SIZE);

			for (int j = 0; j < BLOCK_A_SIZE; j++)
			{
				pdata[j] = payload->frm_payload[j] ^ block_s[j];
				if (++len_pld == payload->frm_payload_len) break;
			}
			pdata += BLOCK_A_SIZE;
		}
		payload->_payload->size += payload->frm_payload_len;
	}
}

void MacPayload_set_fport(MacPayload* payload, uint16_t fport)
{
	ESP_ERROR_CHECK(
		payload->fport != NULL ||
		(payload->fhdr->fopts_len > 0 && fport == 0)
	);

	payload->fport = malloc(BYTE_SIZE(FRAME_PORT_SIZE));
	memset(payload->fport, fport, BYTE_SIZE(FRAME_PORT_SIZE));
}

void MacPayload_set_app_payload(
	MacPayload* payload, 
	uint16_t fport,
	int len,
	uint8_t* app_payload)
{
	ESP_ERROR_CHECK(
		fport == 0 ||
		(payload->fport != NULL && (uint16_t)(*payload->fport) == 0)
	);

	payload->frm_payload = malloc(sizeof(uint8_t) * len);
	memcpy(payload->frm_payload, app_payload, sizeof(uint8_t) * len);
	payload->frm_payload_len = len;

	payload->fport = malloc(BYTE_SIZE(FRAME_PORT_SIZE));
	memset(payload->fport, fport, BYTE_SIZE(FRAME_PORT_SIZE));
}

void MacPayload_set_commands_to_payload(
	MacPayload* payload, 
	int len, 
	MacCommand** cmds)
{
	ESP_ERROR_CHECK(
		payload->frm_payload != NULL ||
		payload->fhdr->fopts_len > 0
	);
	
	uint32_t max_len = 
		MAXIMUM_FRAME_PAYLOAD_SIZE - 
		MAXIMUM_FRAME_OPTIONS_SIZE + 
		payload->fhdr->fopts_len;
	uint8_t* frm_payload = malloc(BYTE_SIZE(max_len));
	uint8_t* pfrm_payload = frm_payload;
	
	payload->frm_payload_len = 0;
	for (int i = 0; i < len; i++)
	{
		cmds[i]->_icmd->extract(cmds[i]->instance);
		payload->frm_payload_len += cmds[i]->size;
		ESP_ERROR_CHECK(payload->frm_payload_len > max_len);

		memcpy(pfrm_payload, cmds[i]->data, BYTE_SIZE(cmds[i]->size));
		pfrm_payload += cmds[i]->size;
	}
	payload->frm_payload = malloc(BYTE_SIZE(payload->frm_payload_len));
	memcpy(payload->frm_payload, frm_payload, BYTE_SIZE(payload->frm_payload_len));
	
	payload->fport = malloc(BYTE_SIZE(FRAME_PORT_SIZE));
	memset(payload->fport, 0x0, BYTE_SIZE(FRAME_PORT_SIZE));
	
	free(frm_payload);
}

void MacPayload_destroy(MacPayload* payload)
{
	free(payload->fhdr);
	free(payload->fport);
	free(payload->frm_payload);

	if (payload->_payload != NULL) Payload_destroy(payload->_payload);
	free(payload);
}

MacPayload* MacPayload_create(FrameHeader* fhdr)
{
	ESP_ERROR_CHECK(
		fhdr->fopts_len < 0 || 
		fhdr->fopts_len > MAXIMUM_FRAME_OPTIONS_SIZE
	);
	
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
	payload->_ipayload->validate = &MacPayload_validate;

	payload->frm_payload_len = 0;

	return payload;
}

////////////////////////////////////////////////////////////////////////////////

void JoinRequestPayload_extract(JoinRequestPayload* payload)
{
	uint8_t* pdata = payload->_payload->data;

	memcpy(pdata, payload->join_eui, BYTE_SIZE(JOIN_EUI_SIZE));
	pdata += JOIN_EUI_SIZE;
	payload->_payload->size += JOIN_EUI_SIZE;

	memcpy(pdata, payload->dev_eui, BYTE_SIZE(DEV_EUI_SIZE));
	pdata += DEV_EUI_SIZE;
	payload->_payload->size += DEV_EUI_SIZE;
	
	memcpy(pdata, payload->dev_nonce, BYTE_SIZE(DEV_NONCE_SIZE));
	pdata += DEV_NONCE_SIZE;
	payload->_payload->size += DEV_NONCE_SIZE;
}

void JoinRequestPayload_destroy(JoinRequestPayload* payload)
{
	free(payload->join_eui);
	free(payload->dev_eui);
	free(payload->dev_nonce);

    if (payload->_payload != NULL) Payload_destroy(payload->_payload);
    free(payload);
}

JoinRequestPayload* JoinRequestPayload_create(
	uint8_t* join_eui,
	uint8_t* dev_eui,
	uint8_t* dev_nonce)
{
	JoinRequestPayload* payload = malloc(sizeof(JoinRequestPayload));
	payload->instance = payload;

	payload->join_eui = malloc(BYTE_SIZE(JOIN_EUI_SIZE));
	payload->dev_eui = malloc(BYTE_SIZE(DEV_EUI_SIZE));
	payload->dev_nonce = malloc(BYTE_SIZE(DEV_NONCE_SIZE));

	memcpy(payload->join_eui, join_eui, BYTE_SIZE(JOIN_EUI_SIZE));
	memcpy(payload->dev_eui, dev_eui, BYTE_SIZE(DEV_EUI_SIZE));
	memcpy(payload->dev_nonce, dev_nonce, BYTE_SIZE(DEV_NONCE_SIZE));

	payload->_payload = Payload_create();
    payload->_payload->instance = payload->instance;

	payload->_ipayload = payload->_payload->_ipayload;
	payload->_ipayload->extract = &JoinRequestPayload_extract;

	return payload;
}

////////////////////////////////////////////////////////////////////////////////

JoinAcceptPayload* JoinAcceptPayload_create_by_payload(Payload* _payload)
{
	JoinAcceptPayload* payload = malloc(sizeof(JoinAcceptPayload));
    payload->instance = payload;

	payload->_payload = _payload;
	payload->_payload->instance = payload->instance;

	payload->_ipayload = payload->_payload->_ipayload;
	payload->_ipayload->extract = &JoinAcceptPayload_extract;
	payload->_ipayload->validate = &JoinAcceptPayload_validate;

	payload->join_nonce = malloc(BYTE_SIZE(JOIN_NONCE_SIZE));
	payload->net_id = malloc(BYTE_SIZE(NET_ID_SIZE));
	payload->dev_addr = malloc(BYTE_SIZE(DEV_ADDR_SIZE));
	payload->dl_settings = malloc(BYTE_SIZE(DLSETTINGS_SIZE));
	payload->cf_list = malloc(BYTE_SIZE(CFLIST_SIZE));
	payload->rx_delay = malloc(BYTE_SIZE(RX_DELAY_SIZE));

	return payload;
}

int JoinAcceptPayload_validate(JoinAcceptPayload* payload)
{
	uint16_t size = payload->_payload->size;
	if (size < JOIN_NONCE_SIZE + NET_ID_SIZE + DEV_ADDR_SIZE + 
		DLSETTINGS_SIZE + RX_DELAY_SIZE) 
	{
		return INVALID_DATA_SIZE;
	}

	uint8_t* pdata = payload->_payload->data;

	memcpy(payload->join_nonce, pdata, BYTE_SIZE(JOIN_NONCE_SIZE));
	size -= JOIN_NONCE_SIZE;
	pdata += JOIN_NONCE_SIZE;

	memcpy(payload->net_id, pdata, BYTE_SIZE(NET_ID_SIZE));
	size -= NET_ID_SIZE;
	pdata += NET_ID_SIZE;
	
	memcpy(payload->dev_addr, pdata, BYTE_SIZE(DEV_ADDR_SIZE));
	size -= DEV_ADDR_SIZE;
	pdata += DEV_ADDR_SIZE;

	memcpy(payload->dl_settings, pdata, BYTE_SIZE(DLSETTINGS_SIZE));
	size -= DLSETTINGS_SIZE;
	pdata += DLSETTINGS_SIZE;

	memcpy(payload->rx_delay, pdata, BYTE_SIZE(RX_DELAY_SIZE));
	size -= RX_DELAY_SIZE;
	pdata += RX_DELAY_SIZE;


	if (size == CFLIST_SIZE)
	{
		memcpy(payload->cf_list, pdata, BYTE_SIZE(CFLIST_SIZE));
		size -= CFLIST_SIZE;
		pdata += CFLIST_SIZE;
	}
	else
	{
		free(payload->cf_list);
		payload->cf_list = NULL;
	}

	if (size != 0)
	{
		return INVALID_DATA_SIZE;
	}
	
	return 0;
}

void JoinAcceptPayload_set_join_nonce(
	JoinAcceptPayload* payload, 
	uint32_t join_nonce)
{
	payload->join_nonce[2] = (uint8_t)(join_nonce >> 16);
	payload->join_nonce[1] = (uint8_t)(join_nonce >> 8);
	payload->join_nonce[0] = (uint8_t)(join_nonce);
}

void JoinAcceptPayload_extract(JoinAcceptPayload* payload)
{
	uint8_t* pdata = payload->_payload->data;

	memcpy(pdata, payload->join_nonce, BYTE_SIZE(JOIN_NONCE_SIZE));
	pdata += JOIN_NONCE_SIZE;
	payload->_payload->size += JOIN_NONCE_SIZE;

	memcpy(pdata, payload->net_id, BYTE_SIZE(NET_ID_SIZE));
	pdata += NET_ID_SIZE;
	payload->_payload->size += NET_ID_SIZE;
	
	memcpy(pdata, payload->dev_addr, BYTE_SIZE(DEV_ADDR_SIZE));
	pdata += DEV_ADDR_SIZE;
	payload->_payload->size += DEV_ADDR_SIZE;

	memcpy(pdata, payload->dl_settings, BYTE_SIZE(DLSETTINGS_SIZE));
	pdata += DLSETTINGS_SIZE;
	payload->_payload->size += DLSETTINGS_SIZE;

	if (payload->cf_list)
	{
		memcpy(pdata, payload->cf_list, BYTE_SIZE(CFLIST_SIZE));
		pdata += CFLIST_SIZE;
		payload->_payload->size += CFLIST_SIZE;
	}

	memcpy(pdata, payload->rx_delay, BYTE_SIZE(RX_DELAY_SIZE));
	pdata += RX_DELAY_SIZE;
	payload->_payload->size += RX_DELAY_SIZE;
}

JoinAcceptPayload* JoinAcceptPayload_create(
	uint32_t join_nonce, 
	uint8_t* net_id, 
	uint8_t* dev_addr, 
	DLSettings* dl_settings, 
	uint16_t rx_delay, 
	CFList* cf_list)
{
	JoinAcceptPayload* payload = malloc(sizeof(JoinAcceptPayload));
    payload->instance = payload;

	payload->join_nonce = malloc(BYTE_SIZE(JOIN_NONCE_SIZE));
	payload->net_id = malloc(BYTE_SIZE(NET_ID_SIZE));
	payload->dev_addr = malloc(BYTE_SIZE(DEV_ADDR_SIZE));
	payload->dl_settings = malloc(BYTE_SIZE(DLSETTINGS_SIZE));
	
    JoinAcceptPayload_set_join_nonce(payload, join_nonce);
	memcpy(payload->net_id, net_id, sizeof(uint8_t)* NET_ID_SIZE);
	memcpy(payload->dev_addr, dev_addr, sizeof(uint8_t)* DEV_ADDR_SIZE);
	payload->dl_settings[0] = 
		SET_BITS(dl_settings->rx1_dr_offset, RX1DROFFSET_BITS, RX1DROFFSET_OFFSET) |
		SET_BITS(dl_settings->rx2_data_rate, RX2DATARATE_BITS, RX2DATARATE_OFFSET);

	if (cf_list)
	{
		payload->cf_list = malloc(BYTE_SIZE(CFLIST_SIZE));
		memcpy(
			&payload->cf_list[FRE_CH3_OFFSET], 
			&cf_list->fre_ch3, 
			BYTE_SIZE(FRE_CHX_SIZE)
		);
		memcpy(
			&payload->cf_list[FRE_CH4_OFFSET], 
			&cf_list->fre_ch4, 
			BYTE_SIZE(FRE_CHX_SIZE)
		);
		memcpy(
			&payload->cf_list[FRE_CH5_OFFSET], 
			&cf_list->fre_ch5, 
			BYTE_SIZE(FRE_CHX_SIZE)
		);
		memcpy(
			&payload->cf_list[FRE_CH6_OFFSET], 
			&cf_list->fre_ch6, 
			BYTE_SIZE(FRE_CHX_SIZE)
		);
		memcpy(
			&payload->cf_list[FRE_CH7_OFFSET], 
			&cf_list->fre_ch7, 
			BYTE_SIZE(FRE_CHX_SIZE)
		);
		memset(
			&payload->cf_list[CFLIST_TYPE_OFFSET], 
			0x0, 
			BYTE_SIZE(FRE_CHX_SIZE)
		);
	}
	else
	{
        payload->cf_list = NULL;
	}
	payload->rx_delay = malloc(BYTE_SIZE(RX_DELAY_SIZE));
	memset(payload->rx_delay, rx_delay, BYTE_SIZE(RX_DELAY_SIZE));

	payload->_payload = Payload_create();
	payload->_payload->instance = payload->instance;

	payload->_ipayload = payload->_payload->_ipayload;
	payload->_ipayload->extract = &JoinAcceptPayload_extract;
	payload->_ipayload->validate = &JoinAcceptPayload_validate;

	return payload;
}

void JoinAcceptPayload_destroy(JoinAcceptPayload* payload)
{
	free(payload->join_nonce);
	free(payload->net_id);
	free(payload->dev_addr);
	free(payload->rx_delay);
	free(payload->dl_settings);
	free(payload->cf_list);
	
	if (payload->_payload != NULL) Payload_destroy(payload->_payload);
    free(payload);
}

////////////////////////////////////////////////////////////////////////////////