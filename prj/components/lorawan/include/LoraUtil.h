#ifndef LORA_ULTIL_H
#define LORA_ULTIL_H

#include "stddef.h"
#include "stdint.h"
#include "string.h"
#include "stdlib.h"
#include "stdio.h"
#include "esp_system.h"

#define INVALID_MIC			-1
#define INVALID_DATA_SIZE	-2

#define BYTE_SIZE(size) (sizeof(uint8_t) * (size))
#define BIT_MASK(value, mask) ((value) & ((1 << (mask)) - 1))
#define SET_BITS(value, mask, offset) (BIT_MASK((value), (mask)) << (offset))
#define SET_BIT(value, offset) SET_BITS((value), 1, (offset))
#define GET_BITS(value, mask, offset) BIT_MASK((value) >> (offset), (mask))

void calculate_mic(
    uint8_t* key, 
    uint8_t* input, 
    size_t len, 
    uint8_t* mic);

void aes128_encrypt(
	uint8_t* key, 
	uint8_t* input, 
	uint8_t* output, 
	uint16_t len);

void aes128_decrypt(
	uint8_t* key, 
	uint8_t* input, 
	uint8_t* output, 
	uint16_t len);
	
void debug_print_array(int size, uint8_t* data, int type);


#endif // LORA_ULTIL_H