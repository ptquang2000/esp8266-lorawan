#include "MacFrame.h"
#include "unity.h"


TEST_CASE("Test Join Accept", "[lorawan]")
{
	unsigned char app_key[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16};
	unsigned int join_nonce = 65793;
	unsigned char net_id[] = {5, 6, 7};
	unsigned char dev_addr[] = {4, 3, 2, 1};
	DLSettings settings = {
		.rx1_dr_offset = 1,
		.rx2_data_rate = 0
	};
	short int rx_delay = 3;

	JoinAcceptFrame* frame = JoinAcceptFrame_create(join_nonce, net_id, dev_addr, &settings, rx_delay, NULL);
	frame->_iframe->extract(frame, app_key);

	unsigned char expected[] = {32, 41, 31, 235, 47, 84, 152, 30, 82, 170, 83, 60, 153, 54, 106, 70, 65};
	TEST_ASSERT_EQUAL_INT(sizeof(expected), frame->size);
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected, frame->data, sizeof(expected));

	JoinAcceptFrame_destroy(frame);
}

TEST_CASE("Test Join Request", "[lorawan]")
{
	unsigned char app_key[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16};
	unsigned char join_eui[] = {3, 3, 3, 3, 1, 1, 1, 1};
	unsigned char dev_eui[] = {2, 2, 2, 2, 4, 4, 4, 4};
	short int dev_nonce = 772;
    LoraDevice* device = LoraDevice_create(app_key, join_eui, dev_eui, dev_nonce);

	JoinRequestFrame* frame = JoinRequestFrame_create(device);
	frame->_iframe->extract(frame, device->app_key);

	unsigned char expected[] = {0, 3, 3, 3, 3, 1, 1, 1, 1, 2, 2, 2, 2, 4, 4, 4, 4, 4, 3, 12, 81, 205, 202};
	TEST_ASSERT_EQUAL_INT(sizeof(expected), frame->size);
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected, frame->data, sizeof(expected));

	JoinRequestFrame_destroy(frame);
	LoraDevice_destroy(device);
}

TEST_CASE("Test Confirmed Data Uplink", "[lorawan]")
{
	
}