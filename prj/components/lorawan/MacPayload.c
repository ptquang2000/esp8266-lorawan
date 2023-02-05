#include "MacPayload.h"
#include "LoraDevice.h"
#include "MacCommand.h"


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
    payload->_ipayload = &Payload_extract;

	return payload;
}

void Payload_destroy(Payload* payload)
{
	free(payload->_ipayload);
	free(payload);
}

////////////////////////////////////////////////////////////////////////////////

void JoinRequestPayload_extract(Payload* payload)
{
	JoinRequestPayload* p = payload->instance;
	unsigned char* pdata = payload->data;

	memcpy(pdata, p->join_eui, sizeof(unsigned char) * JOIN_EUI_SIZE);
	pdata += JOIN_EUI_SIZE;
	payload->size += JOIN_EUI_SIZE;
	free(p->join_eui);

	memcpy(pdata, p->dev_eui, sizeof(unsigned char) * DEV_EUI_SIZE);
	pdata += DEV_EUI_SIZE;
	payload->size += DEV_EUI_SIZE;
	free(p->dev_eui);
	
	memcpy(pdata, p->dev_nonce, sizeof(unsigned char) * DEV_NONCE_SIZE);
	pdata += DEV_NONCE_SIZE;
	payload->size += DEV_NONCE_SIZE;
	free(p->dev_nonce);
}

void JoinRequestPayload_destroy(JoinRequestPayload* payload)
{
	free(p->join_eui);
	free(p->dev_eui);
	free(p->dev_nonce);
    Payload_destroy(payload->_payload);
    free(payload);
}

JoinRequestPayload* JoinRequestPayload_create(LoraDevice* device)
{
	JoinRequestPayload* payload = malloc(sizeof(JoinRequestPayload));
	payload->_payload->instance = payload;

	payload->join_eui = malloc(sizeof(unsigned char) * JOIN_EUI_SIZE);
	payload->dev_eui = malloc(sizeof(unsigned char) * DEV_EUI_SIZE);
	payload->dev_nonce = malloc(sizeof(unsigned char) * DEV_NONCE_SIZE);

	memcpy(payload->join_eui, device->join_eui, sizeof(unsigned char)* JOIN_EUI_SIZE);
	memcpy(payload->dev_eui, device->dev_eui, sizeof(unsigned char)* DEV_EUI_SIZE);
	memcpy(payload->dev_nonce, device->dev_nonce, sizeof(unsigned char)* DEV_NONCE_SIZE);

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

void JoinAcceptPayload_extract(Payload* payload)
{
	JoinAcceptPayload* p = payload->instance;
	unsigned char* pdata = payload->data;

	memcpy(pdata, p->join_nonce, sizeof(unsigned char) * JOIN_NONCE_SIZE);
	pdata += JOIN_NONCE_SIZE;
	payload->size += JOIN_NONCE_SIZE;
	free(p->join_nonce);

	memcpy(pdata, p->net_id, sizeof(unsigned char) * NET_ID_SIZE);
	pdata += NET_ID_SIZE;
	payload->size += NET_ID_SIZE;
	free(p->net_id);
	
	memcpy(pdata, p->dev_addr, sizeof(unsigned char) * DEV_ADDR_SIZE);
	pdata += DEV_ADDR_SIZE;
	payload->size += DEV_ADDR_SIZE;
	free(p->dev_addr);

	memcpy(pdata, p->dl_settings, sizeof(unsigned char) * DLSETTINGS_SIZE);
	pdata += DLSETTINGS_SIZE;
	payload->size += DLSETTINGS_SIZE;
	free(p->dl_settings);

	if (p->cf_list)
	{
		memcpy(pdata, p->cf_list, sizeof(unsigned char) * CFLIST_SIZE);
		pdata += CFLIST_SIZE;
		payload->size += CFLIST_SIZE;
		free(p->cf_list);
	}

	memcpy(pdata, p->rx_delay, sizeof(unsigned char) * RX_DELAY_SIZE);
	pdata += RX_DELAY_SIZE;
	payload->size += RX_DELAY_SIZE;
	free(p->rx_delay);
}

void JoinAcceptPayload_destroy(JoinAcceptPayload* payload)
{
	free(payload->join_nonce);
	free(payload->net_id);
	free(payload->dev_addr);
	free(payload->rx_delay);
	free(payload->dl_settings);
	free(payload->cf_list);
    Payload_destroy(payload->_payload);
    free(payload);
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

	payload->join_nonce = malloc(sizeof(unsigned char) * JOIN_NONCE_SIZE);
	payload->net_id = malloc(sizeof(unsigned char) * NET_ID_SIZE);
	payload->dev_addr = malloc(sizeof(unsigned char) * DEV_ADDR_SIZE);
	payload->dl_settings = malloc(sizeof(unsigned char) * DLSETTINGS_SIZE);
	payload->cf_list = malloc(sizeof(unsigned char) * CFLIST_SIZE);
	
    JoinAccept_set_join_nonce(payload, join_nonce);
	memcpy(payload->net_id, net_id, sizeof(unsigned char)* NET_ID_SIZE);
	memcpy(payload->dev_addr, dev_addr, sizeof(unsigned char)* DEV_ADDR_SIZE);
	payload->dl_settings[0] = SET_BITS(dl_settings->rx1_dr_offset, RX1DROFFSET_BITS, RX1DROFFSET_OFFSET) |
						SET_BITS(dl_settings->rx2_data_rate, RX2DATARATE_BITS, RX2DATARATE_OFFSET);
	if (cf_list)
	{
		memset(payload->cf_list[FRE_CH3_OFFSET], BIT_MASK(cf_list->fre_ch3, 12), sizeof(unsigned char) * FRE_CHX_SIZE);
		memset(payload->cf_list[FRE_CH4_OFFSET], BIT_MASK(cf_list->fre_ch4, 12), sizeof(unsigned char) * FRE_CHX_SIZE);
		memset(payload->cf_list[FRE_CH5_OFFSET], BIT_MASK(cf_list->fre_ch5, 12), sizeof(unsigned char) * FRE_CHX_SIZE);
		memset(payload->cf_list[FRE_CH6_OFFSET], BIT_MASK(cf_list->fre_ch6, 12), sizeof(unsigned char) * FRE_CHX_SIZE);
		memset(payload->cf_list[FRE_CH7_OFFSET], BIT_MASK(cf_list->fre_ch7, 12), sizeof(unsigned char) * FRE_CHX_SIZE);
		memset(payload->cf_list[CFLIST_TYPE_OFFSET], 0x0, sizeof(unsigned char) * FRE_CHX_SIZE);
	}
	else
	{
		free(payload->cf_list);
        payload->cf_list = NULL;
	}
	payload->rx_delay = malloc(sizeof(unsigned char) * RX_DELAY_SIZE);
	memset(payload->rx_delay, rx_delay & 0xf, sizeof(unsigned char) * RX_DELAY_SIZE);

	payload->_payload = Payload_create();
	payload->_payload->size = 0;
	payload->_payload->instance = payload->instance;

	payload->_ipayload = payload->_payload->_ipayload;
	payload->_ipayload->extract = &JoinAcceptPayload_extract;

	return payload;
}

////////////////////////////////////////////////////////////////////////////////