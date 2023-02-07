#include "MacFrame.h"
#include "LoraUtil.h"
#include "unity.h"

static unsigned char nwk_skey[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16};
static unsigned char app_skey[] = {16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1};
static unsigned char dev_addr[] = {4, 3, 2, 1};

void print_frame_data(Frame* frame, int type)
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

TEST_CASE("Link Check Request - fopts", "[class A][MacCommand][linkcheck]")
{
	LoraDevice* device = LoraDevice_create(dev_addr, nwk_skey, app_skey, nwk_skey, nwk_skey, nwk_skey, 0);
	MacCommand* cmd = LinkCheckReq_create()->_cmd;
	static FrameHeader fhdr = {
		.dev_addr = {4, 3, 2, 1},
		.is_adr = 0,
		.is_adr_ack_req = 0,
		.is_ack = 0,
		.frame_counter = 0,
		.is_classB = 0,
		.fopts_len = 0
	};
	FrameHeader_insert_cmd(&fhdr, cmd);

	MacFrame* frame = MacFrame_create(ConfirmedDataUplink, &fhdr);
	frame->_iframe->extract(frame->instance, device);

	unsigned char expected[] = {128, 4, 3, 2, 1, 1, 0, 0, 2, 37, 211, 18, 101};
	TEST_ASSERT_EQUAL_INT(sizeof(expected), frame->_frame->size);
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected, frame->_frame->data, sizeof(expected));

	LinkCheckReq_destroy(cmd->instance);
	MacFrame_destroy(frame);
	LoraDevice_destroy(device);
}

TEST_CASE("Link Check Request - frm payload", "[class A][MacCommand][linkcheck]")
{
    LoraDevice* device = LoraDevice_create(dev_addr, nwk_skey, app_skey, nwk_skey, nwk_skey, nwk_skey, 0);
	FrameHeader fhdr = {
		.dev_addr = {4, 3, 2, 1},
		.is_adr = 0,
		.is_adr_ack_req = 0,
		.is_ack = 0,
		.frame_counter = 0,
		.is_classB = 0,
		.fopts_len = 0
	};

	MacFrame* frame = MacFrame_create(ConfirmedDataUplink, &fhdr);
	short int num_of_cmds = 1;
	MacCommand** cmds = malloc(sizeof(MacCommand*) * num_of_cmds);
	cmds[0] = LinkCheckReq_create()->_cmd;

    MacPayload_set_commands_to_payload(frame->payload, num_of_cmds, cmds);
	frame->_iframe->extract(frame->instance, device);

	unsigned char expected[] = {128, 4, 3, 2, 1, 0, 0, 0, 0, 86, 206, 7, 139, 34};
	TEST_ASSERT_EQUAL_INT(sizeof(expected), frame->_frame->size);
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected, frame->_frame->data, sizeof(expected));

	LinkCheckReq_destroy(cmds[0]->instance);
	free(cmds);
	MacFrame_destroy(frame);
	LoraDevice_destroy(device);
}

TEST_CASE("Test ADR Answer", "[class A]")
{

}

TEST_CASE("Test ADR Answer", "[class A]")
{

}

TEST_CASE("Test Duty Cycle Answer", "[class A]")
{

}

TEST_CASE("Test RX Param Setup Answer", "[class A]")
{

}

TEST_CASE("Test Device Status Answer", "[class A]")
{
	short int fport = 10;
    LoraDevice* device = LoraDevice_create(dev_addr, nwk_skey, app_skey, nwk_skey, nwk_skey, nwk_skey, 0);
	static FrameHeader fhdr = {
		.dev_addr = {4, 3, 2, 1},
		.is_adr = 0,
		.is_adr_ack_req = 0,
		.is_ack = 0,
		.frame_counter = 0,
		.is_classB = 0,
		.fopts_len = 0
	};
	MacCommand* cmd = DevStatusAns_create(115, 7)->_cmd;
	FrameHeader_insert_cmd(&fhdr, cmd);

	MacFrame* frame = MacFrame_create(ConfirmedDataUplink, &fhdr);

	unsigned char payload[] = {1, 2, 3, 4};
	MacPayload_set_app_payload(frame->payload->instance, fport, sizeof(payload), payload);

	frame->_iframe->extract(frame->instance, device);

	unsigned char expected[] = {128, 4, 3, 2, 1, 3, 0, 0, 6, 115, 7, 10, 226, 100, 212, 247, 225, 23, 210, 192};
	TEST_ASSERT_EQUAL_INT(sizeof(expected), frame->_frame->size);
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected, frame->_frame->data, sizeof(expected));

	DevStatusAns_destroy(cmd->instance);
	MacFrame_destroy(frame);
	LoraDevice_destroy(device);
}

TEST_CASE("Test New Channel Answer", "[class A]")
{

}

TEST_CASE("Test TX Param Setup Answer", "[class A]")
{

}

TEST_CASE("Test Downlink Channel Answer", "[class A]")
{

}

TEST_CASE("Test Device Time Request Answer", "[class A]")
{

}