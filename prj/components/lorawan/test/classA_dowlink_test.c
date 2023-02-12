#include "MacFrame.h"
#include "unity.h"

static uint8_t nwk_skey[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16};
static uint8_t app_skey[] = {16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1};
static uint8_t dev_addr[] = {4, 3, 2, 1};
static uint8_t fport[] = {0};
static uint8_t empty_fopt[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
static uint16_t frame_counter  = 43605;
static uint16_t direction = 1;

static MacFrame* test_frame(uint8_t len, uint8_t* data)
{
	Frame* _frame = Frame_create_by_data(len, data);
	TEST_ASSERT_EQUAL(0, _frame->_iframe->validate(_frame));

	MacFrame* frame = MacFrame_create_by_frame(_frame);
	TEST_ASSERT_EQUAL(0, frame->_iframe->validate(frame, nwk_skey, app_skey, direction, dev_addr, frame_counter));
	TEST_ASSERT_EQUAL_UINT16(frame_counter, frame->payload->fhdr->frame_counter);
	TEST_ASSERT_EQUAL_UINT8_ARRAY(dev_addr, frame->payload->fhdr->dev_addr, sizeof(dev_addr));

	return frame;
}

TEST_CASE("Link Check Request", "[LinkCheckReq]")
{
	uint8_t cmd_payload[] = {LinkCheck, 10 ,15};
	MacFrame* frame;
	uint8_t data[] = {160, 4, 3, 2, 1, 0, 85, 170, 0, 35, 243, 254, 255, 36, 1, 2};
	frame = test_frame(sizeof(data), data);
	TEST_ASSERT_EQUAL_UINT8_ARRAY(empty_fopt, frame->payload->fhdr->fopts, MAXIMUM_FRAME_OPTIONS_SIZE);
	TEST_ASSERT_EQUAL_UINT8_ARRAY(fport, frame->payload->fport, sizeof(fport));
	TEST_ASSERT_EQUAL_UINT8_ARRAY(cmd_payload, frame->payload->frm_payload, sizeof(cmd_payload));
	MacFrame_destroy(frame);

	uint8_t data2[] = {160, 4, 3, 2, 1, 3, 85, 170, 2, 10, 15, 116, 78, 114, 151};
	frame = test_frame(sizeof(data2), data2);
	TEST_ASSERT_EQUAL_UINT8_ARRAY(cmd_payload, frame->payload->fhdr->fopts, sizeof(cmd_payload));
	TEST_ASSERT_EQUAL(NULL, frame->payload->fport);
	TEST_ASSERT_EQUAL(NULL, frame->payload->frm_payload);
	MacFrame_destroy(frame);
}

TEST_CASE("Link ADR Request", "[LinkADRReq]")
{
	uint8_t cmd_payload[] = {LinkADR, 0xF7, 0x07, 0x00, 0x03};
	MacFrame* frame;
	uint8_t data[] = {160, 4, 3, 2, 1, 0, 85, 170, 0, 34, 14, 246, 82, 223, 118, 218, 176, 20};
	frame = test_frame(sizeof(data), data);
	TEST_ASSERT_EQUAL_UINT8_ARRAY(empty_fopt, frame->payload->fhdr->fopts, MAXIMUM_FRAME_OPTIONS_SIZE);
	TEST_ASSERT_EQUAL_UINT8_ARRAY(fport, frame->payload->fport, sizeof(fport));
	TEST_ASSERT_EQUAL_UINT8_ARRAY(cmd_payload, frame->payload->frm_payload, sizeof(cmd_payload));
	MacFrame_destroy(frame);

	uint8_t data2[] = {160, 4, 3, 2, 1, 5, 85, 170, 3, 247, 7, 0, 3, 119, 5, 25, 40};
	frame = test_frame(sizeof(data2), data2);
	TEST_ASSERT_EQUAL_UINT8_ARRAY(cmd_payload, frame->payload->fhdr->fopts, sizeof(cmd_payload));
	TEST_ASSERT_EQUAL(NULL, frame->payload->fport);
	TEST_ASSERT_EQUAL(NULL, frame->payload->frm_payload);
	MacFrame_destroy(frame);
}

TEST_CASE("Link Duty Cycle Request", "[DutyCycleReq]")
{
	uint8_t cmd_payload[] = {DutyCycle, 0x02};
	MacFrame* frame;
	uint8_t data[] = {160, 4, 3, 2, 1, 0, 85, 170, 0, 37, 251, 196, 170, 110, 156};
	frame = test_frame(sizeof(data), data);
	TEST_ASSERT_EQUAL_UINT8_ARRAY(empty_fopt, frame->payload->fhdr->fopts, MAXIMUM_FRAME_OPTIONS_SIZE);
	TEST_ASSERT_EQUAL_UINT8_ARRAY(fport, frame->payload->fport, sizeof(fport));
	TEST_ASSERT_EQUAL_UINT8_ARRAY(cmd_payload, frame->payload->frm_payload, sizeof(cmd_payload));
	MacFrame_destroy(frame);

	uint8_t data2[] = {160, 4, 3, 2, 1, 2, 85, 170, 4, 2, 253, 98, 204, 116};
	frame = test_frame(sizeof(data2), data2);
	TEST_ASSERT_EQUAL_UINT8_ARRAY(cmd_payload, frame->payload->fhdr->fopts, sizeof(cmd_payload));
	TEST_ASSERT_EQUAL(NULL, frame->payload->fport);
	TEST_ASSERT_EQUAL(NULL, frame->payload->frm_payload);
	MacFrame_destroy(frame);
}

TEST_CASE("Link RX Param Setup Request", "[RXParamSetupReq]")
{
	uint8_t cmd_payload[] = {RXParamSetup, 0x2B, 0x08, 0x00, 0x00};
	MacFrame* frame;
	uint8_t data[] = {160, 4, 3, 2, 1, 0, 85, 170, 0, 36, 210, 249, 82, 220, 156, 77, 248, 20};
	frame = test_frame(sizeof(data), data);
	TEST_ASSERT_EQUAL_UINT8_ARRAY(empty_fopt, frame->payload->fhdr->fopts, MAXIMUM_FRAME_OPTIONS_SIZE);
	TEST_ASSERT_EQUAL_UINT8_ARRAY(fport, frame->payload->fport, sizeof(fport));
	TEST_ASSERT_EQUAL_UINT8_ARRAY(cmd_payload, frame->payload->frm_payload, sizeof(cmd_payload));
	MacFrame_destroy(frame);

	uint8_t data2[] = {160, 4, 3, 2, 1, 5, 85, 170, 5, 43, 8, 0, 0, 143, 10, 191, 231};
	frame = test_frame(sizeof(data2), data2);
	TEST_ASSERT_EQUAL_UINT8_ARRAY(cmd_payload, frame->payload->fhdr->fopts, sizeof(cmd_payload));
	TEST_ASSERT_EQUAL(NULL, frame->payload->fport);
	TEST_ASSERT_EQUAL(NULL, frame->payload->frm_payload);
	MacFrame_destroy(frame);
}

TEST_CASE("Link Device Status Request", "[DevStatusReq]")
{
	uint8_t cmd_payload[] = {DevStatus};
	MacFrame* frame;
	uint8_t data[] = {160, 4, 3, 2, 1, 0, 85, 170, 0, 39, 146, 131, 78, 15};
	frame = test_frame(sizeof(data), data);
	TEST_ASSERT_EQUAL_UINT8_ARRAY(empty_fopt, frame->payload->fhdr->fopts, MAXIMUM_FRAME_OPTIONS_SIZE);
	TEST_ASSERT_EQUAL_UINT8_ARRAY(fport, frame->payload->fport, sizeof(fport));
	TEST_ASSERT_EQUAL_UINT8_ARRAY(cmd_payload, frame->payload->frm_payload, sizeof(cmd_payload));
	MacFrame_destroy(frame);

	uint8_t data2[] = {160, 4, 3, 2, 1, 1, 85, 170, 6, 127, 99, 171, 48};
	frame = test_frame(sizeof(data2), data2);
	TEST_ASSERT_EQUAL_UINT8_ARRAY(cmd_payload, frame->payload->fhdr->fopts, sizeof(cmd_payload));
	TEST_ASSERT_EQUAL(NULL, frame->payload->fport);
	TEST_ASSERT_EQUAL(NULL, frame->payload->frm_payload);
	MacFrame_destroy(frame);
}

TEST_CASE("Link New Channel Request", "[NewChannelReq]")
{
	uint8_t cmd_payload[] = {NewChannel, 0x02, 0x04, 0x00, 0x00, 0x68};
	MacFrame* frame;
	uint8_t data[] = {160, 4, 3, 2, 1, 0, 85, 170, 0, 38, 251, 245, 82, 220, 153, 165, 34, 215, 144};
	frame = test_frame(sizeof(data), data);
	TEST_ASSERT_EQUAL_UINT8_ARRAY(empty_fopt, frame->payload->fhdr->fopts, MAXIMUM_FRAME_OPTIONS_SIZE);
	TEST_ASSERT_EQUAL_UINT8_ARRAY(fport, frame->payload->fport, sizeof(fport));
	TEST_ASSERT_EQUAL_UINT8_ARRAY(cmd_payload, frame->payload->frm_payload, sizeof(cmd_payload));
	MacFrame_destroy(frame);

	uint8_t data2[] = {160, 4, 3, 2, 1, 6, 85, 170, 7, 2, 4, 0, 0, 104, 248, 4, 120, 130};
	frame = test_frame(sizeof(data2), data2);
	TEST_ASSERT_EQUAL_UINT8_ARRAY(cmd_payload, frame->payload->fhdr->fopts, sizeof(cmd_payload));
	TEST_ASSERT_EQUAL(NULL, frame->payload->fport);
	TEST_ASSERT_EQUAL(NULL, frame->payload->frm_payload);
	MacFrame_destroy(frame);
}

TEST_CASE("Link Downlink Channel Request", "[DlChannelReq]")
{
	uint8_t cmd_payload[] = {DlChannel, 0x0A, 0xFE, 0xC0, 0x00};
	MacFrame* frame;
	uint8_t data[] = {160, 4, 3, 2, 1, 0, 85, 170, 0, 43, 243, 15, 146, 220, 75, 120, 66, 3};
	frame = test_frame(sizeof(data), data);
	TEST_ASSERT_EQUAL_UINT8_ARRAY(empty_fopt, frame->payload->fhdr->fopts, MAXIMUM_FRAME_OPTIONS_SIZE);
	TEST_ASSERT_EQUAL_UINT8_ARRAY(fport, frame->payload->fport, sizeof(fport));
	TEST_ASSERT_EQUAL_UINT8_ARRAY(cmd_payload, frame->payload->frm_payload, sizeof(cmd_payload));
	MacFrame_destroy(frame);

	uint8_t data2[] = {160, 4, 3, 2, 1, 5, 85, 170, 10, 10, 254, 192, 0, 5, 178, 69, 135};
	frame = test_frame(sizeof(data2), data2);
	TEST_ASSERT_EQUAL_UINT8_ARRAY(cmd_payload, frame->payload->fhdr->fopts, sizeof(cmd_payload));
	TEST_ASSERT_EQUAL(NULL, frame->payload->fport);
	TEST_ASSERT_EQUAL(NULL, frame->payload->frm_payload);
	MacFrame_destroy(frame);
}

TEST_CASE("Link Downlink Channel Request", "[DlChannelReq]")
{
	uint8_t cmd_payload[] = {DlChannel, 0x0A, 0xFE, 0xC0, 0x00};
	MacFrame* frame;
	uint8_t data[] = {160, 4, 3, 2, 1, 0, 85, 170, 0, 43, 243, 15, 146, 220, 75, 120, 66, 3};
	frame = test_frame(sizeof(data), data);
	TEST_ASSERT_EQUAL_UINT8_ARRAY(empty_fopt, frame->payload->fhdr->fopts, MAXIMUM_FRAME_OPTIONS_SIZE);
	TEST_ASSERT_EQUAL_UINT8_ARRAY(fport, frame->payload->fport, sizeof(fport));
	TEST_ASSERT_EQUAL_UINT8_ARRAY(cmd_payload, frame->payload->frm_payload, sizeof(cmd_payload));
	MacFrame_destroy(frame);

	uint8_t data2[] = {160, 4, 3, 2, 1, 5, 85, 170, 10, 10, 254, 192, 0, 5, 178, 69, 135};
	frame = test_frame(sizeof(data2), data2);
	TEST_ASSERT_EQUAL_UINT8_ARRAY(cmd_payload, frame->payload->fhdr->fopts, sizeof(cmd_payload));
	TEST_ASSERT_EQUAL(NULL, frame->payload->fport);
	TEST_ASSERT_EQUAL(NULL, frame->payload->frm_payload);
	MacFrame_destroy(frame);
}

TEST_CASE("Link RX Timing Setup Request", "[RXTimingSetupReq]")
{
	uint8_t cmd_payload[] = {RXTimingSetup, 0x09};
	MacFrame* frame;
	uint8_t data[] = {160, 4, 3, 2, 1, 0, 85, 170, 0, 41, 240, 206, 96, 35, 158};
	frame = test_frame(sizeof(data), data);
	TEST_ASSERT_EQUAL_UINT8_ARRAY(empty_fopt, frame->payload->fhdr->fopts, MAXIMUM_FRAME_OPTIONS_SIZE);
	TEST_ASSERT_EQUAL_UINT8_ARRAY(fport, frame->payload->fport, sizeof(fport));
	TEST_ASSERT_EQUAL_UINT8_ARRAY(cmd_payload, frame->payload->frm_payload, sizeof(cmd_payload));
	MacFrame_destroy(frame);

	uint8_t data2[] = {160, 4, 3, 2, 1, 2, 85, 170, 8, 9, 239, 89, 159, 5};
	frame = test_frame(sizeof(data2), data2);
	TEST_ASSERT_EQUAL_UINT8_ARRAY(cmd_payload, frame->payload->fhdr->fopts, sizeof(cmd_payload));
	TEST_ASSERT_EQUAL(NULL, frame->payload->fport);
	TEST_ASSERT_EQUAL(NULL, frame->payload->frm_payload);
	MacFrame_destroy(frame);
}


TEST_CASE("Link Device Time Answer", "[DeviceTimeAns]")
{
	uint8_t cmd_payload[] = {DeviceTime, 0x01, 0x00, 0x00, 0x00, 0x02};
	MacFrame* frame;
	uint8_t data[] = {160, 4, 3, 2, 1, 0, 85, 170, 0, 44, 248, 241, 82, 220, 243, 19, 171, 115, 21};
	frame = test_frame(sizeof(data), data);
	TEST_ASSERT_EQUAL_UINT8_ARRAY(empty_fopt, frame->payload->fhdr->fopts, MAXIMUM_FRAME_OPTIONS_SIZE);
	TEST_ASSERT_EQUAL_UINT8_ARRAY(fport, frame->payload->fport, sizeof(fport));
	TEST_ASSERT_EQUAL_UINT8_ARRAY(cmd_payload, frame->payload->frm_payload, sizeof(cmd_payload));
	MacFrame_destroy(frame);

	uint8_t data2[] = {160, 4, 3, 2, 1, 6, 85, 170, 13, 1, 0, 0, 0, 2, 162, 25, 11, 153};
	frame = test_frame(sizeof(data2), data2);
	TEST_ASSERT_EQUAL_UINT8_ARRAY(cmd_payload, frame->payload->fhdr->fopts, sizeof(cmd_payload));
	TEST_ASSERT_EQUAL(NULL, frame->payload->fport);
	TEST_ASSERT_EQUAL(NULL, frame->payload->frm_payload);
	MacFrame_destroy(frame);
}
