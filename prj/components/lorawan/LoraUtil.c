#include "LoraUtil.h"
#include "LorPayload.h"
#include "mbedtls/aes.h"
#include "mbedtls/cmac.h"
#include "esp_system.h"


void calculate_mic(
    unsigned char* key, 
    unsigned char* input, 
    size_t len, 
    unsigned char* mic)
{
	const mbedtls_cipher_info_t* cipher_info = mbedtls_cipher_info_from_type(MBEDTLS_CIPHER_AES_128_ECB);
	unsigned char cmac[16];
	ESP_ERROR_CHECK(mbedtls_cipher_cmac(cipher_info, key, 128, input, len, cmac));
	memcpy(mic, cmac, sizeof(unsigned char) * MIC_SIZE);
}

void aes128_encrypt(
	unsigned char* key, 
	unsigned char* input, 
	unsigned char* output, 
	short int len)
{
	short int times = ((len % 16 == 0) ? len : len + 1) / 16;
	mbedtls_aes_context aes;
	mbedtls_aes_init(&aes);
	ESP_ERROR_CHECK(mbedtls_aes_setkey_enc(&aes, key, 128));
	for (short int i = 0; i < times; i++)
	{
		ESP_ERROR_CHECK(mbedtls_aes_crypt_ecb(&aes, MBEDTLS_AES_ENCRYPT, input, output));
		input += 16;
		output += 16;
	}
	mbedtls_aes_free(&aes);
}

void aes128_decrypt(
	unsigned char* key, 
	unsigned char* input, 
	unsigned char* output, 
	short int len)
{
	short int times = ((len % 16 == 0) ? len : len + 1) / 16;
	mbedtls_aes_context aes;
	mbedtls_aes_init(&aes);
	ESP_ERROR_CHECK(mbedtls_aes_setkey_dec(&aes, key, 128));
	for (short int i = 0; i < times; i++)
	{
		ESP_ERROR_CHECK(mbedtls_aes_crypt_ecb(&aes, MBEDTLS_AES_DECRYPT, input, output));
		input += 16;
		output += 16;
	}
	mbedtls_aes_free(&aes);
}