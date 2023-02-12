#include "MacFrame.h"
#include "LoraUtil.h"
#include "unity.h"

static uint8_t nwk_skey[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16};
static uint8_t app_skey[] = {16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1};
static uint16_t direction = 0;

static FrameHeader fhdr = {
	.dev_addr = {4, 3, 2, 1},
	.is_adr = 0,
	.is_adr_ack_req = 0,
	.is_ack = 0,
	.frame_counter = 0,
	.fpending = 0,
	.fopts_len = 0
};

static void print_frame_data(Frame* frame, int type)
{
	printf("[");
	for (int i = 0; i < frame->size; i++)
	{
		if (type == 0)
		{
			printf("%d", frame->data[i]);
		}
		else
		{
			printf("%02x", frame->data[i]);

		}
		if (i == frame->size - 1) printf("]\n");
		else printf(" ");
	}
}

static void check_cmd_frm_payload(MacCommand* cmd, uint16_t len, uint8_t* expected)
{
	fhdr.fopts_len = 0;

	MacFrame* frame = MacFrame_create(ConfirmedDataUplink, &fhdr);
	uint16_t num_of_cmds = 1;
	MacCommand** cmds = malloc(sizeof(MacCommand*) * num_of_cmds);
	cmds[0] = cmd;

    MacPayload_set_commands_to_payload(frame->payload, num_of_cmds, cmds);
	frame->_iframe->extract(frame->instance, nwk_skey, app_skey, direction);

	TEST_ASSERT_EQUAL_INT(len, frame->_frame->size);
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected, frame->_frame->data, len);

	free(cmds);
	MacFrame_destroy(frame);
}

static void check_cmd_fopts(MacCommand* cmd, uint16_t len, uint8_t* expected)
{
	fhdr.fopts_len = 0;
	FrameHeader_insert_cmd(&fhdr, cmd);

	MacFrame* frame = MacFrame_create(ConfirmedDataUplink, &fhdr);
	frame->_iframe->extract(frame->instance, nwk_skey, app_skey, direction);

	TEST_ASSERT_EQUAL_INT(len, frame->_frame->size);
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected, frame->_frame->data, len);

	MacFrame_destroy(frame);
}

TEST_CASE("Link Check Request", "[LinkCheckReq]")
{
	LinkCheckReq* cmd;
	
	cmd = LinkCheckReq_create();
	uint8_t expected2[] = {128, 4, 3, 2, 1, 1, 0, 0, 2, 37, 211, 18, 101};
	check_cmd_fopts(cmd->_cmd, sizeof(expected2), expected2);
	LinkCheckReq_destroy(cmd);

	cmd = LinkCheckReq_create();
	uint8_t expected1[] = {128, 4, 3, 2, 1, 0, 0, 0, 0, 86, 206, 7, 139, 34};
	check_cmd_frm_payload(cmd->_cmd, sizeof(expected1), expected1);
	LinkCheckReq_destroy(cmd);
}

TEST_CASE("Test Link ADR Answer", "[LinkADRAbs]")
{
	ADRStatus status = {
		.power = 0,
		.data_rate = 1,
		.channel_mask = 0
	};
	LinkAdrAns* cmd;
	
	cmd = LinkAdrAns_create(&status);
	uint8_t expected2[] = {128, 4, 3, 2, 1, 2, 0, 0, 3, 2, 151, 13, 75, 228};
	check_cmd_fopts(cmd->_cmd, sizeof(expected2), expected2);
	LinkAdrAns_destroy(cmd);

	cmd = LinkAdrAns_create(&status);
	uint8_t expected1[] = {128, 4, 3, 2, 1, 0, 0, 0, 0, 87, 169, 119, 227, 97, 94};
	check_cmd_frm_payload(cmd->_cmd, sizeof(expected1), expected1);
	LinkAdrAns_destroy(cmd);
}

TEST_CASE("Test Duty Cycle Answer", "[DutyCycleAns]")
{
	DutyCycleAns* cmd;

	cmd = DutyCycleAns_create();
	uint8_t expected2[] = {128, 4, 3, 2, 1, 1, 0, 0, 4, 118, 44, 151, 196};
	check_cmd_fopts(cmd->_cmd, sizeof(expected2), expected2);
	DutyCycleAns_destroy(cmd);
	
	cmd = DutyCycleAns_create();	
	uint8_t expected1[] = {128, 4, 3, 2, 1, 0, 0, 0, 0, 80, 155, 8, 220, 222};
	check_cmd_frm_payload(cmd->_cmd, sizeof(expected1), expected1);
	DutyCycleAns_destroy(cmd);
}

TEST_CASE("Test RX Param Setup Answer", "[RxParamSetupAns]")
{
	RXParamSetupStatus status = {
		.rx1_dr_offset_ack = 0,
		.rx2_data_rate_ack = 1,
		.channel_ack = 0
	};
	RxParamSetupAns* cmd;
	
	cmd = RxParamSetupAns_create(&status);
	uint8_t expected2[] = {128, 4, 3, 2, 1, 2, 0, 0, 5, 2, 103, 96, 41, 22};
	check_cmd_fopts(cmd->_cmd, sizeof(expected2), expected2);
	RxParamSetupAns_destroy(cmd);

	cmd = RxParamSetupAns_create(&status);
	uint8_t expected1[] = {128, 4, 3, 2, 1, 0, 0, 0, 0, 81, 169, 78, 227, 251, 51};
	check_cmd_frm_payload(cmd->_cmd, sizeof(expected1), expected1);
	RxParamSetupAns_destroy(cmd);
}

TEST_CASE("Test Device Status Answer", "[DevStatusAns]")
{
	DeviceStatus status = {
		.battery_power = 115,
		.radio_status = 7
	};
	DevStatusAns* cmd;
	
	cmd = DevStatusAns_create(&status);
	uint8_t expected2[] = {128, 4, 3, 2, 1, 3, 0, 0, 6, 115, 7, 147, 230, 218, 159};
	check_cmd_fopts(cmd->_cmd, sizeof(expected2), expected2);
	DevStatusAns_destroy(cmd);

	cmd = DevStatusAns_create(&status);
	uint8_t expected1[] = {128, 4, 3, 2, 1, 0, 0, 0, 0, 82, 216, 9, 37, 24, 198, 131};
	check_cmd_frm_payload(cmd->_cmd, sizeof(expected1), expected1);
	DevStatusAns_destroy(cmd);
}

TEST_CASE("Test New Channel Answer", "[NewChannelAns]")
{
	NewChannelStatus status = {
		.channel_frequency = 1,
		.data_rate_range = 0,
	};
	NewChannelAns* cmd;

	cmd = NewChannelAns_create(&status);
	uint8_t expected1[] = {128, 4, 3, 2, 1, 0, 0, 0, 0, 83, 170, 171, 190, 214, 60};
	check_cmd_frm_payload(cmd->_cmd, sizeof(expected1), expected1);
	NewChannelAns_destroy(cmd);

	cmd = NewChannelAns_create(&status);
	uint8_t expected2[] = {128, 4, 3, 2, 1, 2, 0, 0, 7, 1, 99, 201, 55, 120};
	check_cmd_fopts(cmd->_cmd, sizeof(expected2), expected2);
	NewChannelAns_destroy(cmd);
}

TEST_CASE("Test Downlink Channel Answer", "[DlChannelAns]")
{
	DownlinkChannelStatus status = {
		.uplink_frequency = 1,
		.channel_frequency = 0,
	};
	DownlinkChannelAns* cmd;
	
	cmd = DownlinkChannelAns_create(&status);
	uint8_t expected2[] = {128, 4, 3, 2, 1, 2, 0, 0, 10, 2, 174, 67, 220, 216};
	check_cmd_fopts(cmd->_cmd, sizeof(expected2), expected2);
	DownlinkChannelAns_destroy(cmd);

	cmd = DownlinkChannelAns_create(&status);
	uint8_t expected1[] = {128, 4, 3, 2, 1, 0, 0, 0, 0, 94, 169, 52, 193, 22, 130};
	check_cmd_frm_payload(cmd->_cmd, sizeof(expected1), expected1);
	DownlinkChannelAns_destroy(cmd);
}

TEST_CASE("Test RX Timing Setup Answer", "[RXTimingSetupAns]")
{
	RXTimingSetupAns* cmd;

	cmd = RXTimingSetupAns_create();
	uint8_t expected2[] = {128, 4, 3, 2, 1, 1, 0, 0, 8, 104, 233, 181, 62};
	check_cmd_fopts(cmd->_cmd, sizeof(expected2), expected2);
	RXTimingSetupAns_destroy(cmd);

	cmd = RXTimingSetupAns_create();
	uint8_t expected1[] = {128, 4, 3, 2, 1, 0, 0, 0, 0, 92, 190, 16, 101, 141};
	check_cmd_frm_payload(cmd->_cmd, sizeof(expected1), expected1);
	RXTimingSetupAns_destroy(cmd);
}

TEST_CASE("Test Device Time Request Request", "[DeviceTimeReq]")
{
	DeviceTimeReq* cmd;

	cmd = DeviceTimeReq_create();
	uint8_t expected2[] = {128, 4, 3, 2, 1, 1, 0, 0, 13, 204, 0, 57, 22};
	check_cmd_fopts(cmd->_cmd, sizeof(expected2), expected2);
	DeviceTimeReq_destroy(cmd);

	cmd = DeviceTimeReq_create();
	uint8_t expected1[] = {128, 4, 3, 2, 1, 0, 0, 0, 0, 89, 216, 165, 117, 18};
	check_cmd_frm_payload(cmd->_cmd, sizeof(expected1), expected1);
	DeviceTimeReq_destroy(cmd);
}