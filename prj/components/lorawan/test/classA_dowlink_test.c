#include "MacFrame.h"
#include "unity.h"

static unsigned char app_key[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16};
static unsigned char nwk_skey[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16};
static unsigned char app_skey[] = {16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1};
static unsigned char dev_addr[] = {4, 3, 2, 1};
static short int frame_counter  = 43605;
static short int direction = 1;

TEST_CASE("Link Check Request", "[LinkCheckAns]")
{
	unsigned char data[] = {160, 4, 3, 2, 1, 0, 85, 170, 0, 35, 243, 254, 255, 36, 1, 2};
	Frame* _frame = Frame_create_by_data(sizeof(data), data);
	TEST_ASSERT_EQUAL(0, _frame->_iframe->validate(_frame));

	MacFrame* frame = MacFrame_create_by_frame(_frame);
	TEST_ASSERT_EQUAL(0, frame->_iframe->validate(frame, nwk_skey, app_skey, direction, dev_addr, frame_counter));
	MacFrame_destroy(frame);

	unsigned char data2[] = {160, 4, 3, 2, 1, 3, 85, 170, 2, 10, 15, 116, 78, 114, 151};
	_frame = Frame_create_by_data(sizeof(data2), data2);
	TEST_ASSERT_EQUAL(0, _frame->_iframe->validate(_frame));

	frame = MacFrame_create_by_frame(_frame);
	TEST_ASSERT_EQUAL(0, frame->_iframe->validate(frame, nwk_skey, app_skey, direction, dev_addr, frame_counter));
	MacFrame_destroy(frame);

}