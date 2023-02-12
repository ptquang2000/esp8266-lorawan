#ifndef MAC_COMMAND_H
#define MAC_COMMAND_H

#include "MacCommandDef.h"
#include "LoraUtil.h"


typedef enum MacCommandType_enum
{
    Request         = 0,
    Answer          = 1
} MacCommandType;

typedef enum MacCommandCid_enum
{
    LinkCheck       = 0x02,
    LinkADR         = 0x03,
    DutyCycle       = 0x04,
    RXParamSetup    = 0x05,
    DevStatus       = 0x06,
    NewChannel      = 0x07,
    RXTimingSetup   = 0x08,
    TXParamSetup    = 0x09,
    DlChannel       = 0x0A,
    DeviceTime      = 0x0D
} MacCommandCid;

typedef struct IMacCommand_struct
{
    void (*extract)();
} IMacCommand;

typedef struct MacCommand_struct
{
    MacCommandType type; 
    MacCommandCid cid;

    uint16_t size;
    uint8_t data[MAC_COMMAND_MAX_SIZE];

    IMacCommand* _icmd;
    void* instance;
} MacCommand;

MacCommand* MacCommand_create(MacCommandCid cid, MacCommandType type);
void MacCommand_destroy(MacCommand* cmd);

typedef struct EmptyCommand_struct
{
    IMacCommand* _icmd;
    MacCommand* _cmd;
    void* instance;
} EmptyCommand;

typedef EmptyCommand LinkCheckReq;

void LinkCheckReq_destroy(LinkCheckReq* cmd);
LinkCheckReq* LinkCheckReq_create();

typedef struct LinkCheckAns_struct
{
    uint8_t* margin;
    uint8_t* gateway_cnt;

    IMacCommand* _icmd;
    MacCommand* _cmd;
    void* instance;
} LinkCheckAns;

typedef struct LinkAdrReq_struct
{
    uint8_t* datarate_txpower;
    uint8_t* channel_mask;
    uint8_t* redundancy;  

    IMacCommand* _icmd;
    MacCommand* _cmd;
    void* instance;
} LinkAdrReq;

typedef struct LinkAdrAns_struct
{
    uint8_t* status;

    IMacCommand* _icmd;
    MacCommand* _cmd;
    void* instance;
} LinkAdrAns;

void LinkAdrAns_destroy(LinkAdrAns* cmd);
LinkAdrAns* LinkAdrAns_create(ADRStatus* status);

typedef struct DutyCycleReq_struct
{
    uint8_t* duty_cycle;

    IMacCommand* _icmd;
    MacCommand* _cmd;
    void* instance;
} DutyCycleReq;

typedef EmptyCommand DutyCycleAns;

void DutyCycleAns_destroy(DutyCycleAns* cmd);
DutyCycleAns* DutyCycleAns_create();

typedef struct RxParamSetupReq_struct
{
    uint8_t* downlink_settings;
    uint8_t* frequency;

    IMacCommand* _icmd;
    MacCommand* _cmd;
    void* instance;
} RxParamSetupReq;

typedef struct RxParamSetupAns_struct
{
    uint8_t* status;

    IMacCommand* _icmd;
    MacCommand* _cmd;
    void* instance;
} RxParamSetupAns;

void RxParamSetupAns_destroy(RxParamSetupAns* cmd);
RxParamSetupAns* RxParamSetupAns_create(RXParamSetupStatus* status);

typedef EmptyCommand DevStatusReq;

typedef struct DevStatusAns_struct
{
    uint8_t* battery;
    uint8_t* radio_status;

    IMacCommand* _icmd;
    MacCommand* _cmd;
    void* instance;
} DevStatusAns;

void DevStatusAns_destroy(DevStatusAns* cmd);
DevStatusAns* DevStatusAns_create(DeviceStatus* status);

typedef struct NewChannelReq_struct
{
    uint8_t* channel_index;
    uint8_t* frequency;
    uint8_t* data_rate_range;

    IMacCommand* _icmd;
    MacCommand* _cmd;
    void* instance;
} NewChannelReq;

typedef struct NewChannelAns_struct
{
    uint8_t* status;

    IMacCommand* _icmd;
    MacCommand* _cmd;
    void* instance;
} NewChannelAns;

void NewChannelAns_destroy(NewChannelAns* cmd);
NewChannelAns* NewChannelAns_create(NewChannelStatus* status);

typedef struct DownlinkChannelReq_struct
{
    uint8_t* channel_index;
    uint8_t* frequency;

    IMacCommand* _icmd;
    MacCommand* _cmd;
    void* instance;
} DownlinkChannelReq;

typedef struct DownlinkChannelAns_struct
{
    uint8_t* status;

    IMacCommand* _icmd;
    MacCommand* _cmd;
    void* instance;
} DownlinkChannelAns;

DownlinkChannelAns* DownlinkChannelAns_create(DownlinkChannelStatus* status);
void DownlinkChannelAns_destroy(DownlinkChannelAns* cmd);

typedef struct RXTimingSetupReq_struct
{
    uint8_t* rx_timing_settings; 

    IMacCommand* _icmd;
    MacCommand* _cmd;
    void* instance;
} RXTimingSetupReq;

typedef EmptyCommand RXTimingSetupAns;

void RXTimingSetupAns_destroy(RXTimingSetupAns* cmd);
RXTimingSetupAns* RXTimingSetupAns_create();

typedef EmptyCommand DeviceTimeReq;

void DeviceTimeReq_destroy(RXTimingSetupAns* cmd);
DeviceTimeReq* DeviceTimeReq_create();

typedef struct DeviceTimeAns_struct
{
    uint8_t* seconds;
    uint8_t* fractional_second;

    IMacCommand* _icmd;
    MacCommand* _cmd;
    void* instance;
} DeviceTimeAns;


#endif // MAC_COMMAND_H