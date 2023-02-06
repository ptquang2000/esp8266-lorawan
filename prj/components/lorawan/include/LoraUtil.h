#ifndef LORA_ULTIL_H
#define LORA_ULTIL_H

#include "stddef.h"
#include "string.h"
#include "stdlib.h"
#include "stdio.h"
#include "esp_system.h"

#define BYTE_SIZE(size) (sizeof(unsigned char) * (size))
#define BIT_MASK(value, mask) ((value) & ((1 << (mask)) - 1))
#define SET_BITS(value, mask, offset) (BIT_MASK((value), (mask)) << (offset))


void calculate_mic(
    unsigned char* key, 
    unsigned char* input, 
    size_t len, 
    unsigned char* mic);

void aes128_encrypt(
	unsigned char* key, 
	unsigned char* input, 
	unsigned char* output, 
	short int len);

void aes128_decrypt(
	unsigned char* key, 
	unsigned char* input, 
	unsigned char* output, 
	short int len);


#endif // LORA_ULTIL_H