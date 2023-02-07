#ifndef MAC_COMMAND_H
#define MAC_COMMAND_H

#include "MacCommandDef.h"


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
    NewChannel      = 0x06,
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

    short int size;
    unsigned char data[MAC_COMMAND_MAX_SIZE];

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
    unsigned char* margin;
    unsigned char* gateway_cnt;

    IMacCommand* _icmd;
    MacCommand* _cmd;
    void* instance;
} LinkCheckAns;

typedef struct LinkAdrReq_struct
{
    unsigned char* datarate_txpower;
    unsigned char* channel_mask;
    unsigned char* redundancy;  

    IMacCommand* _icmd;
    MacCommand* _cmd;
    void* instance;
} LinkAdrReq;

typedef struct LinkAdrAns_struct
{
    unsigned char* status;

    IMacCommand* _icmd;
    MacCommand* _cmd;
    void* instance;
} LinkAdrAns;

typedef struct DutyCycleReq_struct
{
    unsigned char* duty_cycle;

    IMacCommand* _icmd;
    MacCommand* _cmd;
    void* instance;
} DutyCycleReq;

typedef EmptyCommand DutyCycleAns;

typedef struct RxParamSetupReq_struct
{
    unsigned char* downlink_settings;
    unsigned char* frequency;

    IMacCommand* _icmd;
    MacCommand* _cmd;
    void* instance;
} RxParamSetupReq;

typedef struct RxParamSetupAns_struct
{
    unsigned char* status;

    IMacCommand* _icmd;
    MacCommand* _cmd;
    void* instance;
} RxParamSetupAns;

typedef EmptyCommand DevStatusReq;

typedef struct DevStatusAns_struct
{
    unsigned char* battery;
    unsigned char* radio_status;

    IMacCommand* _icmd;
    MacCommand* _cmd;
    void* instance;
} DevStatusAns;

DevStatusAns* DevStatusAns_create(short int battery, short int snr);
void DevStatusAns_destroy(DevStatusAns* cmd);

typedef struct NewChannelReq_struct
{
    unsigned char* channel_index;
    unsigned char* frequency;
    unsigned char* data_rate_range;

    IMacCommand* _icmd;
    MacCommand* _cmd;
    void* instance;
} NewChannelReq;

typedef struct NewChannelAns_struct
{
    unsigned char* status;

    IMacCommand* _icmd;
    MacCommand* _cmd;
    void* instance;
} NewChannelAns;

typedef struct DownlinkChannelReq_struct
{
    unsigned char* channel_index;
    unsigned char* frequency;

    IMacCommand* _icmd;
    MacCommand* _cmd;
    void* instance;
} DownlinkChannelReq;

typedef struct DownlinkChannelAns_struct
{
    unsigned char* status;

    IMacCommand* _icmd;
    MacCommand* _cmd;
    void* instance;
} DownlinkChannelAns;

typedef struct RXTimingSetupReq_struct
{
    unsigned char* rx_timing_settings; 

    IMacCommand* _icmd;
    MacCommand* _cmd;
    void* instance;
} RXTimingSetupReq;

typedef EmptyCommand RXTimingSetupAns;

typedef struct DeviceTimeReq_struct
{
    unsigned char* seconds;
    unsigned char* fractional_second;

    IMacCommand* _icmd;
    MacCommand* _cmd;
    void* instance;
} DeviceTimeReq;

typedef EmptyCommand DeviceTimeAns;


#endif // MAC_COMMAND_H