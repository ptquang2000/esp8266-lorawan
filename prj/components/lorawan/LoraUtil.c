#include "LoraUtil.h"
#include "MacPayload.h"
#include "mbedtls/aes.h"
#include "mbedtls/cmac.h"


void calculate_mic(
    uint8_t* key, 
    uint8_t* input, 
    size_t len, 
    uint8_t* mic)
{
	const mbedtls_cipher_info_t* cipher_info = mbedtls_cipher_info_from_type(MBEDTLS_CIPHER_AES_128_ECB);
	uint8_t cmac[16];
	ESP_ERROR_CHECK(mbedtls_cipher_cmac(cipher_info, key, 128, input, len, cmac));
	memcpy(mic, cmac, BYTE_SIZE(MIC_SIZE));
}

void aes128_encrypt(
	uint8_t* key, 
	uint8_t* input, 
	uint8_t* output, 
	uint16_t len)
{
	uint16_t times = ((len % 16 == 0) ? len : len + 1) / 16;
	mbedtls_aes_context aes;
	mbedtls_aes_init(&aes);
	ESP_ERROR_CHECK(mbedtls_aes_setkey_enc(&aes, key, 128));
	for (uint16_t i = 0; i < times; i++)
	{
		ESP_ERROR_CHECK(mbedtls_aes_crypt_ecb(&aes, MBEDTLS_AES_ENCRYPT, input, output));
		input += 16;
		output += 16;
	}
	mbedtls_aes_free(&aes);
}

void aes128_decrypt(
	uint8_t* key, 
	uint8_t* input, 
	uint8_t* output, 
	uint16_t len)
{
	uint16_t times = ((len % 16 == 0) ? len : len + 1) / 16;
	mbedtls_aes_context aes;
	mbedtls_aes_init(&aes);
	ESP_ERROR_CHECK(mbedtls_aes_setkey_dec(&aes, key, 128));
	for (uint16_t i = 0; i < times; i++)
	{
		ESP_ERROR_CHECK(mbedtls_aes_crypt_ecb(&aes, MBEDTLS_AES_DECRYPT, input, output));
		input += 16;
		output += 16;
	}
	mbedtls_aes_free(&aes);
}


void debug_print_array(int size, uint8_t* data, int type)
{
	printf("\n[");
	for (int i = 0; i < size; i++)
	{
		if (type == 0)
		{
			printf("%d", data[i]);
		}
		else
		{
			printf("%02x", data[i]);

		}
		if (i == size - 1) printf("]\n");
		else printf(" ");
	}
}