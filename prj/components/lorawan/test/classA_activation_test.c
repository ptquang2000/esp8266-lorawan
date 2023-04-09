#include "MacFrame.h"
#include "LoraUtil.h"
#include "unity.h"

static uint8_t s_app_key[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16};
static uint8_t s_join_eui[] = {3, 3, 3, 3, 1, 1, 1, 1};
static uint8_t s_dev_eui[] = {2, 2, 2, 2, 4, 4, 4, 4};
static uint16_t s_dev_nonce = 772;

TEST_CASE("Test Join Request", "[JoinRequest]")
{
	uint8_t dev_nonce[] = {(uint8_t)s_dev_nonce, (uint8_t)(s_dev_nonce >> 8)};
	JoinRequestFrame* frame = JoinRequestFrame_create(s_join_eui, s_dev_eui, dev_nonce);
	frame->_iframe->extract(frame, s_app_key);

	uint8_t expected[] = {0, 3, 3, 3, 3, 1, 1, 1, 1, 2, 2, 2, 2, 4, 4, 4, 4, 4, 3, 12, 81, 205, 202};
	TEST_ASSERT_EQUAL_INT(sizeof(expected), frame->_frame->size);
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected, frame->_frame->data, sizeof(expected));

	JoinRequestFrame_destroy(frame);
}

TEST_CASE("Test JoinAccept frame validation without CFlist", "[JoinAccept]")
{
	uint8_t data[] = {32, 41, 31, 235, 47, 84, 152, 30, 82, 170, 83, 60, 153, 54, 106, 70, 65};
	Frame* _frame = Frame_create_by_data(sizeof(data), data);
	TEST_ASSERT_EQUAL(0, _frame->_iframe->validate(_frame->instance));

	JoinAcceptFrame* frame = JoinAcceptFrame_create_by_frame(_frame);
	TEST_ASSERT_EQUAL(JoinAccept, frame->_frame->type);
	TEST_ASSERT_EQUAL(0, frame->_iframe->validate(frame->instance, s_app_key));

	uint8_t join_nonce[] = {1, 1, 1};
	uint8_t net_id[] = {5, 6, 7};
	uint8_t dev_addr[] = {4, 3, 2, 1};
	uint8_t dl_settings[] = {16};
	uint8_t rx_delay[] = {3};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(join_nonce, frame->payload->join_nonce, sizeof(join_nonce));
	TEST_ASSERT_EQUAL_UINT8_ARRAY(net_id, frame->payload->net_id, sizeof(net_id));
	TEST_ASSERT_EQUAL_UINT8_ARRAY(dev_addr, frame->payload->dev_addr, sizeof(dev_addr));
	TEST_ASSERT_EQUAL_UINT8_ARRAY(dl_settings, frame->payload->dl_settings, sizeof(dl_settings));
	TEST_ASSERT_EQUAL(NULL, frame->payload->cf_list);
	TEST_ASSERT_EQUAL_UINT8_ARRAY(rx_delay, frame->payload->rx_delay, sizeof(rx_delay));

	JoinAcceptFrame_destroy(frame);
}

TEST_CASE("Test JoinAccept frame validation with CFlist", "[JoinAccept]")
{
	uint8_t data[] = {
		32, 	72, 	241, 	109, 	183, 	190, 	208, 	96, 	173, 	221, 	253, 
		54, 	59, 	249, 	241, 	147, 	186, 	223, 	242, 	15, 	219, 	135, 
		113, 	70, 	5, 		31, 	255, 	121, 	203,	172, 	185, 	99, 	7
	};
	Frame* _frame = Frame_create_by_data(sizeof(data), data);
	TEST_ASSERT_EQUAL(0, _frame->_iframe->validate(_frame->instance));

	JoinAcceptFrame* frame = JoinAcceptFrame_create_by_frame(_frame);
	TEST_ASSERT_EQUAL(JoinAccept, frame->_frame->type);
	TEST_ASSERT_EQUAL(0, frame->_iframe->validate(frame->instance, s_app_key));

	uint8_t join_nonce[] = {0, 0, 1};
	uint8_t net_id[] = {3, 2, 1};
	uint8_t dev_addr[] = {4, 3, 2, 1};
	uint8_t dl_settings[] = {21};
	uint8_t rx_delay[] = {1};
	uint8_t cf_list[] = {104, 45, 66, 56, 53, 66, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(join_nonce, frame->payload->join_nonce, sizeof(join_nonce));
	TEST_ASSERT_EQUAL_UINT8_ARRAY(net_id, frame->payload->net_id, sizeof(net_id));
	TEST_ASSERT_EQUAL_UINT8_ARRAY(dev_addr, frame->payload->dev_addr, sizeof(dev_addr));
	TEST_ASSERT_EQUAL_UINT8_ARRAY(dl_settings, frame->payload->dl_settings, sizeof(dl_settings));
	TEST_ASSERT_EQUAL_UINT8_ARRAY(rx_delay, frame->payload->rx_delay, sizeof(rx_delay));
	TEST_ASSERT_EQUAL_UINT8_ARRAY(cf_list, frame->payload->cf_list, sizeof(cf_list));

	JoinAcceptFrame_destroy(frame);
}