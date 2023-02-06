#include "MacCommand.h"
#include "LoraUtil.h"


////////////////////////////////////////////////////////////////////////////////

void MacCommand_extract(MacCommand* cmd)
{
    memset(cmd->data, (short int)cmd->type, BYTE_SIZE(CID_SIZE));
    cmd->size += CID_SIZE;
}

void MacCommand_destroy(MacCommand* cmd)
{
    free(cmd->_icmd);
    free(cmd);
}

MacCommand* MacCommand_create(
    MacCommandType type,
    MacCommandCid cid)
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
    MacCommand_destroy(cmd->_cmd);
    free(cmd);
}

DevStatusAns* DevStatusAns_create(short int battery, short int snr)
{
    DevStatusAns* cmd = malloc(sizeof(DevStatusAns));
    cmd->instance = cmd;

    cmd->battery = malloc(BYTE_SIZE(BATTERY_SIZE));
    (*cmd->battery) = (unsigned char)battery;
    cmd->radio_status = malloc(BYTE_SIZE(RADIO_STATUS_SIZE));
    (*cmd->radio_status) = SET_BITS((unsigned char)snr, SNR_BITS, SNR_OFFSET);

    cmd->_cmd = MacCommand_create(DevStatus, Answer);
    cmd->_icmd = cmd->_cmd->_icmd;
    cmd->_cmd->instance = cmd->instance;

    cmd->_icmd->extract = &DevStatusAns_extract;

    return cmd;
}

////////////////////////////////////////////////////////////////////////////////