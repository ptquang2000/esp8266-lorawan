#ifndef MAC_COMMAND_DEF_H
#define MAC_COMMAND_DEF_H

#define CID_SIZE                                1

#define STATUS_SIZE                             1

#define LINK_CHECK_REQ_SIZE                     0

#define LINK_CHECK_ANS_SIZE                     2
#define     MARGIN_SIZE                         1
#define     GATEWAY_COUNT                       1

#define LINK_ADR_REQ_SIZE                       4
#define     DATA_RATE_TX_POWER_SIZE             1
#define         DATA_RATE_BITS                  4
#define         DATA_RATE_OFFSET                0
#define         TX_POWER_BITS                   4
#define         TX_POWER_OFFSET                 0
#define     CHANNEL_MASK_SIZE                   2
#define         CHANNEL_1_BIT                   0
#define         CHANNEL_2_BIT                   1
#define         CHANNEL_3_BIT                   2
#define         CHANNEL_4_BIT                   3
#define         CHANNEL_5_BIT                   4
#define         CHANNEL_6_BIT                   5
#define         CHANNEL_7_BIT                   6
#define         CHANNEL_8_BIT                   7
#define         CHANNEL_9_BIT                   8
#define         CHANNEL_10_BIT                  9
#define         CHANNEL_11_BIT                  10
#define         CHANNEL_12_BIT                  11
#define         CHANNEL_13_BIT                  12
#define         CHANNEL_14_BIT                  13
#define         CHANNEL_15_BIT                  14
#define         CHANNEL_16_BIT                  15
#define     REDUNDANCY_SIZE                     1
#define         CHANNEL_MASK_CONTROL_BITS       3
#define         CHANNEL_MASK_CONTROL_SIZE       4
#define         NB_TRANS_BITS                   4
#define         NB_TRANS_SIZE                   0

#define LINK_ADR_ANS_SIZE                       1
#define     POWER_ACK_BITS                      1
#define     POWER_ACK_OFFSET                    2
#define     DATA_RATE_ACK_BITS                  1
#define     DATA_RATE_ACK_OFFSET                1
#define     CHANNEL_MASK_ACK_BITS               1
#define     CHANNEL_MASK_ACK_OFFSET             0

#define DUTY_CYCLE_REQ_SIZE                     1
#define     MAX_DUTY_CYCLE_BITS                 4
#define     MAX_DUTY_CYCLE_OFFSET               0

#define DUTY_CYCLE_ANS_SIZE                     0

#define RX_PARAM_SETUP_REQ_SIZE                 4
#define     DLSETTINGS_SIZE                     1
#define 	    RX1DROFFSET_BITS                3
#define 	    RX1DROFFSET_OFFSET              4
#define 	    RX2DATARATE_BITS                4
#define 	    RX2DATARATE_OFFSET              0
#define     FREQUENCY_SIZE                      3

#define RX_PARAM_SETUP_ANS_SIZE                 1
#define     RX1_OFFSET_ACK_BIT                  2
#define     RX2_DATA_RATE_ACK_BIT               1
#define     CHANNEL_ACK_BIT                     0

#define DEV_STATUS_REQ_SIZE                     0

#define DEV_STATUS_ANS_SIZE                     2
#define     BATTERY_SIZE                        1
#define     RADIO_STATUS_SIZE                   1
#define         SNR_BITS                        6
#define         SNR_OFFSET                      0

#define NEW_CHANNEL_REQ_SIZE                    5
#define     CHANNEL_INDEX_SIZE                  1
#define     FREQUENCY_SIZE                      3
#define     DATA_RATE_RANGE_SIZE                1
#define         MAX_DR_BITS                     4
#define         MAX_DR_OFFSET                   4
#define         MIN_DR_BITS                     4
#define         MIN_DR_OFFSET                   4

#define NEW_CHANNEL_ANS_SIZE                    1
#define     DATA_RATE_RANGE_BIT                 1
#define     CHANNEL_FREQUENCY_BIT               0

#define DOWNLINK_CHANNEL_REQ_SIZE               4
#define     CHANNEL_INDEX_SIZE                  1
#define     FREQUENCY_SIZE                      3

#define DOWNLINK_CHANNEL_ANS_SIZE               1
#define     UPLINK_FREQUENCY_BIT                1
#define     CHANNEL_FREQUENCY_BIT               0

#define RX_TIMING_SETUP_REQ_SIZE                1
#define     DELAY_BITS                          4
#define     DELAY_OFFSET                        0
#define         DELAY_1_SECOND                  1
#define         DELAY_2_SECOND                  2
#define         DELAY_3_SECOND                  3
#define         DELAY_4_SECOND                  4
#define         DELAY_5_SECOND                  5
#define         DELAY_6_SECOND                  6
#define         DELAY_7_SECOND                  7
#define         DELAY_8_SECOND                  8
#define         DELAY_9_SECOND                  9
#define         DELAY_10_SECOND                 10
#define         DELAY_11_SECOND                 11
#define         DELAY_12_SECOND                 12
#define         DELAY_13_SECOND                 13
#define         DELAY_14_SECOND                 14
#define         DELAY_15_SECOND                 15

#define RX_TIMING_SETUP_ANS_SIZE                0

#define DEVICE_TIME_REQ_SIZE                    0

#define DEVICE_TIME_ANS_SIZE                    5
#define     SECONDS_SIZE                        4                       
#define     FRACTIONAL_SECONDS_SIZE             1                       

#define MAC_COMMAND_MAX_SIZE                    6

typedef struct DataRate_TxPower_struct
{
    short int data_rate;
    short int tx_power;
} DataRate_TxPower;

typedef struct Redundancy_struct
{
    short int channel_mask_ctrl;
    short int num_of_trans;
} Redundancy;

typedef struct ADRStatus_struct
{
    short int power;
    short int data_rate;
    short int channel_mask;
} ADRStatus;

typedef struct DutyCyclePL_struct
{
    short int max_duty_cycle;
} DutyCyclePL;

typedef struct DLSettings_struct
{
	short int rx1_dr_offset;
	short int rx2_data_rate;
} DLSettings;

typedef struct DeviceStatus_struct
{
    short int battery_power;
    short int radio_status;
} DeviceStatus;

typedef struct RXParamSetupStatus_struct
{
	short int rx1_dr_offset_ack;
	short int rx2_data_rate_ack;
    short int channel_ack;
} RXParamSetupStatus;

typedef struct DataRateRange_struct
{
    short int max_date_rate;
    short int min_data_rate;
} DataRateRange;

typedef struct NewChannelStatus_struct
{
    short int data_rate_range;
    short int channel_frequency;
} NewChannelStatus;

typedef struct DownlinkChannelStatus_struct
{
    short int uplink_frequency;
    short int channel_frequency;
} DownlinkChannelStatus;

typedef struct DeviceTimeInSecond_struct
{
    unsigned int seconds;
    short int fraction_second;
} DeviceTimeInSecond;


#endif // MAC_COMMAND_DEF_H