#include "MacCommand.h"
#include "LoraUtil.h"


////////////////////////////////////////////////////////////////////////////////

void MacCommand_extract(MacCommand* cmd)
{
    memset(cmd->data, cmd->cid, BYTE_SIZE(CID_SIZE));
    cmd->size += CID_SIZE;
}

void MacCommand_destroy(MacCommand* cmd)
{
    free(cmd->_icmd);
    free(cmd);
}

MacCommand* MacCommand_create(MacCommandCid cid, MacCommandType type)
{
    MacCommand* cmd = malloc(sizeof(MacCommand));
    cmd->instance = cmd;

    cmd->_icmd = malloc(sizeof(IMacCommand));
    cmd->_icmd->extract = &MacCommand_extract;

    cmd->cid = cid;
    cmd->type = type;
    cmd->size = 0;

    return cmd;
}

////////////////////////////////////////////////////////////////////////////////

void LinkCheckReq_extract(LinkCheckReq* cmd)
{
    MacCommand_extract(cmd->_cmd);
}

void LinkCheckReq_destroy(LinkCheckReq* cmd)
{
    if (cmd->_cmd != NULL) MacCommand_destroy(cmd->_cmd);
}

LinkCheckReq* LinkCheckReq_create()
{
    LinkCheckReq* cmd = malloc(sizeof(LinkCheckReq));
    cmd->instance = cmd;

    cmd->_cmd = MacCommand_create(LinkCheck, Answer);
    cmd->_cmd->instance = cmd->instance;

    cmd->_icmd = cmd->_cmd->_icmd;
    cmd->_icmd->extract = &LinkCheckReq_extract;

    return cmd;
}

////////////////////////////////////////////////////////////////////////////////

void LinkAdrAns_extract(LinkAdrAns* cmd)
{
    MacCommand_extract(cmd->_cmd);
    
    unsigned char* pdata = cmd->_cmd->data;
    pdata += cmd->_cmd->size;
    
    memcpy(pdata, cmd->status, BYTE_SIZE(STATUS_SIZE));
    cmd->_cmd->size += STATUS_SIZE;
}

void LinkAdrAns_destroy(LinkAdrAns* cmd)
{
    free(cmd->status);
    if (cmd->_cmd != NULL) MacCommand_destroy(cmd->_cmd);
    free(cmd);
}

LinkAdrAns* LinkAdrAns_create(ADRStatus* status)
{
    LinkAdrAns* cmd = malloc(sizeof(LinkAdrAns));
    cmd->instance = cmd;

    cmd->status = malloc(BYTE_SIZE(STATUS_SIZE));
    (*cmd->status) = 
        SET_BITS(status->power, POWER_ACK_BITS, POWER_ACK_OFFSET) |
        SET_BITS(status->data_rate, DATA_RATE_ACK_BITS, DATA_RATE_ACK_OFFSET) |
        SET_BITS(status->channel_mask, CHANNEL_MASK_ACK_BITS, CHANNEL_MASK_ACK_OFFSET);

    cmd->_cmd = MacCommand_create(LinkADR, Answer);
    cmd->_cmd->instance = cmd->instance;

    cmd->_icmd = cmd->_cmd->_icmd;
    cmd->_icmd->extract = &LinkAdrAns_extract;

    return cmd;
}

////////////////////////////////////////////////////////////////////////////////

void DutyCycleAns_extract(DutyCycleAns* cmd)
{
    MacCommand_extract(cmd->_cmd);
}

void DutyCycleAns_destroy(DutyCycleAns* cmd)
{
    if (cmd->_cmd != NULL) MacCommand_destroy(cmd->_cmd);
}

DutyCycleAns* DutyCycleAns_create()
{
    DutyCycleAns* cmd = malloc(sizeof(DutyCycleAns));
    cmd->instance = cmd;

    cmd->_cmd = MacCommand_create(DutyCycle, Answer);
    cmd->_cmd->instance = cmd->instance;

    cmd->_icmd = cmd->_cmd->_icmd;
    cmd->_icmd->extract = &DutyCycleAns_extract;

    return cmd;
}

////////////////////////////////////////////////////////////////////////////////

void RxParamSetupAns_extract(RxParamSetupAns* cmd)
{
    MacCommand_extract(cmd->_cmd);
    
    unsigned char* pdata = cmd->_cmd->data;
    pdata += cmd->_cmd->size;
    
    memcpy(pdata, cmd->status, BYTE_SIZE(STATUS_SIZE));
    cmd->_cmd->size += STATUS_SIZE;
}

void RxParamSetupAns_destroy(RxParamSetupAns* cmd)
{
    free(cmd->status);
    if (cmd->_cmd != NULL) MacCommand_destroy(cmd->_cmd);
    free(cmd);
}

RxParamSetupAns* RxParamSetupAns_create(RXParamSetupStatus* status)
{
    RxParamSetupAns* cmd = malloc(sizeof(RxParamSetupAns));
    cmd->instance = cmd;

    cmd->status = malloc(BYTE_SIZE(STATUS_SIZE));
    (*cmd->status) = 
        SET_BIT(status->rx1_dr_offset_ack, RX1_OFFSET_ACK_BIT) |
        SET_BIT(status->rx2_data_rate_ack, RX2_DATA_RATE_ACK_BIT) |
        SET_BIT(status->channel_ack, CHANNEL_ACK_BIT);

    cmd->_cmd = MacCommand_create(RXParamSetup, Answer);
    cmd->_cmd->instance = cmd->instance;

    cmd->_icmd = cmd->_cmd->_icmd;
    cmd->_icmd->extract = &RxParamSetupAns_extract;

    return cmd;
}

////////////////////////////////////////////////////////////////////////////////

void DevStatusAns_extract(DevStatusAns* cmd)
{
    MacCommand_extract(cmd->_cmd);

    unsigned char* pdata = cmd->_cmd->data;
    pdata += cmd->_cmd->size;
    
    memcpy(pdata, cmd->battery, BYTE_SIZE(BATTERY_SIZE));
    pdata += BATTERY_SIZE;
    cmd->_cmd->size += BATTERY_SIZE;

    memcpy(pdata, cmd->radio_status, BYTE_SIZE(RADIO_STATUS_SIZE));
    pdata += RADIO_STATUS_SIZE;
    cmd->_cmd->size += RADIO_STATUS_SIZE;
}

void DevStatusAns_destroy(DevStatusAns* cmd)
{
    free(cmd->battery);
    free(cmd->radio_status);
    if (cmd->_cmd != NULL) MacCommand_destroy(cmd->_cmd);
    free(cmd);
}

DevStatusAns* DevStatusAns_create(DeviceStatus* status)
{
    ESP_ERROR_CHECK(
        status->battery_power > 0xff || status->battery_power < 0x00 ||
        status->radio_status > 0x3f || status->radio_status < 0x00
    );
    DevStatusAns* cmd = malloc(sizeof(DevStatusAns));
    cmd->instance = cmd;

    cmd->battery = malloc(BYTE_SIZE(BATTERY_SIZE));
    (*cmd->battery) = status->battery_power;
    cmd->radio_status = malloc(BYTE_SIZE(RADIO_STATUS_SIZE));
    (*cmd->radio_status) = SET_BITS(status->radio_status, SNR_BITS, SNR_OFFSET);

    cmd->_cmd = MacCommand_create(DevStatus, Answer);
    cmd->_cmd->instance = cmd->instance;

    cmd->_icmd = cmd->_cmd->_icmd;
    cmd->_icmd->extract = &DevStatusAns_extract;

    return cmd;
}

////////////////////////////////////////////////////////////////////////////////

void NewChannelAns_extract(NewChannelAns* cmd)
{
    MacCommand_extract(cmd->_cmd);
    
    unsigned char* pdata = cmd->_cmd->data;
    pdata += cmd->_cmd->size;
    
    memcpy(pdata, cmd->status, BYTE_SIZE(STATUS_SIZE));
    cmd->_cmd->size += STATUS_SIZE;
}

void NewChannelAns_destroy(NewChannelAns* cmd)
{
    free(cmd->status);
    if (cmd->_cmd != NULL) MacCommand_destroy(cmd->_cmd);
    free(cmd);
}

NewChannelAns* NewChannelAns_create(NewChannelStatus* status)
{
    NewChannelAns* cmd = malloc(sizeof(NewChannelAns));
    cmd->instance = cmd;

    cmd->status = malloc(BYTE_SIZE(STATUS_SIZE));
    (*cmd->status) = 
        SET_BIT(status->data_rate_range, DATA_RATE_RANGE_BIT) |
        SET_BIT(status->channel_frequency, CHANNEL_FREQUENCY_BIT);

    cmd->_cmd = MacCommand_create(NewChannel, Answer);
    cmd->_cmd->instance = cmd->instance;

    cmd->_icmd = cmd->_cmd->_icmd;
    cmd->_icmd->extract = &NewChannelAns_extract;

    return cmd;
}

////////////////////////////////////////////////////////////////////////////////

void DownlinkChannelAns_extract(DownlinkChannelAns* cmd)
{
    MacCommand_extract(cmd->_cmd);
    
    unsigned char* pdata = cmd->_cmd->data;
    pdata += cmd->_cmd->size;
    
    memcpy(pdata, cmd->status, BYTE_SIZE(STATUS_SIZE));
    cmd->_cmd->size += STATUS_SIZE;
}

void DownlinkChannelAns_destroy(DownlinkChannelAns* cmd)
{
    free(cmd->status);
    if (cmd->_cmd != NULL) MacCommand_destroy(cmd->_cmd);
    free(cmd);
}

DownlinkChannelAns* DownlinkChannelAns_create(DownlinkChannelStatus* status)
{
    DownlinkChannelAns* cmd = malloc(sizeof(DownlinkChannelAns));
    cmd->instance = cmd;

    cmd->status = malloc(BYTE_SIZE(STATUS_SIZE));
    (*cmd->status) = 
        SET_BIT(status->uplink_frequency, UPLINK_FREQUENCY_BIT) |
        SET_BIT(status->channel_frequency, CHANNEL_FREQUENCY_BIT);

    cmd->_cmd = MacCommand_create(DlChannel, Answer);
    cmd->_cmd->instance = cmd->instance;

    cmd->_icmd = cmd->_cmd->_icmd;
    cmd->_icmd->extract = &DownlinkChannelAns_extract;

    return cmd;
}

////////////////////////////////////////////////////////////////////////////////

void RXTimingSetupAns_extract(RXTimingSetupAns* cmd)
{
    MacCommand_extract(cmd->_cmd);
}

void RXTimingSetupAns_destroy(RXTimingSetupAns* cmd)
{
    if (cmd->_cmd != NULL) MacCommand_destroy(cmd->_cmd);
}

RXTimingSetupAns* RXTimingSetupAns_create()
{
    RXTimingSetupAns* cmd = malloc(sizeof(RXTimingSetupAns));
    cmd->instance = cmd;

    cmd->_cmd = MacCommand_create(RXTimingSetup, Answer);
    cmd->_cmd->instance = cmd->instance;

    cmd->_icmd = cmd->_cmd->_icmd;
    cmd->_icmd->extract = &RXTimingSetupAns_extract;

    return cmd;
}

////////////////////////////////////////////////////////////////////////////////

void DeviceTimeReq_extract(RXTimingSetupAns* cmd)
{
    MacCommand_extract(cmd->_cmd);
}

void DeviceTimeReq_destroy(RXTimingSetupAns* cmd)
{
    if (cmd->_cmd != NULL) MacCommand_destroy(cmd->_cmd);
}

DeviceTimeReq* DeviceTimeReq_create()
{
    DeviceTimeReq* cmd = malloc(sizeof(DeviceTimeReq));
    cmd->instance = cmd;

    cmd->_cmd = MacCommand_create(DeviceTime, Request);
    cmd->_cmd->instance = cmd->instance;

    cmd->_icmd = cmd->_cmd->_icmd;
    cmd->_icmd->extract = &DeviceTimeReq_extract;

    return cmd;
}

////////////////////////////////////////////////////////////////////////////////