#include "MacFrame.h"
#include "LoraUtil.h"
#include "unity.h"

static unsigned char nwk_skey[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16};
static unsigned char app_skey[] = {16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1};
static unsigned char app_key[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16};
static unsigned char join_eui[] = {3, 3, 3, 3, 1, 1, 1, 1};
static unsigned char dev_eui[] = {2, 2, 2, 2, 4, 4, 4, 4};
static short int dev_nonce = 772;


static FrameHeader fhdr = {
	.dev_addr = {4, 3, 2, 1},
	.is_adr = 0,
	.is_adr_ack_req = 0,
	.is_ack = 0,
	.frame_counter = 0,
	.is_classB = 0,
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

static void check_cmd_frm_payload(MacCommand* cmd, short int len, unsigned char* expected)
{
	fhdr.fopts_len = 0;

	MacFrame* frame = MacFrame_create(ConfirmedDataUplink, &fhdr);
	short int num_of_cmds = 1;
	MacCommand** cmds = malloc(sizeof(MacCommand*) * num_of_cmds);
	cmds[0] = cmd;

    MacPayload_set_commands_to_payload(frame->payload, num_of_cmds, cmds);
	frame->_iframe->extract(frame->instance, nwk_skey, app_skey);

	TEST_ASSERT_EQUAL_INT(len, frame->_frame->size);
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected, frame->_frame->data, len);

	free(cmds);
	MacFrame_destroy(frame);
}

static void check_cmd_fopts(MacCommand* cmd, short int len, unsigned char* expected)
{
	fhdr.fopts_len = 0;
	FrameHeader_insert_cmd(&fhdr, cmd);

	MacFrame* frame = MacFrame_create(ConfirmedDataUplink, &fhdr);
	frame->_iframe->extract(frame->instance, nwk_skey, app_skey);

	TEST_ASSERT_EQUAL_INT(len, frame->_frame->size);
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected, frame->_frame->data, len);

	MacFrame_destroy(frame);
}



TEST_CASE("Test Join Request", "[lorawan]")
{
    LoraDevice* device = LoraDevice_create(app_key, app_key, app_key, app_key, join_eui, dev_eui, dev_nonce);

	JoinRequestFrame* frame = JoinRequestFrame_create(device->join_eui, device->dev_eui, device->dev_nonce);
	frame->_iframe->extract(frame, device->app_key);

	unsigned char expected[] = {0, 3, 3, 3, 3, 1, 1, 1, 1, 2, 2, 2, 2, 4, 4, 4, 4, 4, 3, 12, 81, 205, 202};
	TEST_ASSERT_EQUAL_INT(sizeof(expected), frame->_frame->size);
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected, frame->_frame->data, sizeof(expected));

	JoinRequestFrame_destroy(frame);
	LoraDevice_destroy(device);
}

TEST_CASE("Test JoinAccept frame validation no CFlist", "[JoinAccept]")
{
	unsigned char expected[] = {32, 41, 31, 235, 47, 84, 152, 30, 82, 170, 83, 60, 153, 54, 106, 70, 65};
	Frame* _frame = Frame_create_by_data(sizeof(expected), expected);
	TEST_ASSERT_EQUAL(0, _frame->_iframe->validate(_frame->instance));

	JoinAcceptFrame* frame = JoinAcceptFrame_create_by_frame(_frame);
	TEST_ASSERT_EQUAL(JoinAccept, frame->_frame->type);
	TEST_ASSERT_EQUAL(0, frame->_iframe->validate(frame->instance, app_key));

	unsigned char join_nonce[] = {1, 1, 1};
	unsigned char net_id[] = {5, 6, 7};
	unsigned char dev_addr[] = {4, 3, 2, 1};
	unsigned char dl_settings[] = {16};
	unsigned char rx_delay[] = {3};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(join_nonce, frame->payload->join_nonce, sizeof(join_nonce));
	TEST_ASSERT_EQUAL_UINT8_ARRAY(net_id, frame->payload->net_id, sizeof(net_id));
	TEST_ASSERT_EQUAL_UINT8_ARRAY(dev_addr, frame->payload->dev_addr, sizeof(dev_addr));
	TEST_ASSERT_EQUAL_UINT8_ARRAY(dl_settings, frame->payload->dl_settings, sizeof(dl_settings));
	TEST_ASSERT_EQUAL(NULL, frame->payload->cf_list);
	TEST_ASSERT_EQUAL_UINT8_ARRAY(rx_delay, frame->payload->rx_delay, sizeof(rx_delay));

	JoinAcceptFrame_destroy(frame);
}