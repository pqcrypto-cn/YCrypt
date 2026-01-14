#ifndef _SM3_H_
#define _SM3_H_

#include <stdint.h>
#include <string.h>

#define SM3_DIGEST_LENGTH  32    // In bytes
#define SM3_BLOCK_SIZE     64    // In bytes
#define SM3_CBLOCK	       (SM3_BLOCK_SIZE)
#define SM3_HMAC_SIZE      (SM3_DIGEST_LENGTH)

typedef struct _SM3_CTX
{
	uint32_t   digest[8];
	uint32_t   nblocks;                // How many blocks have been compressed. 
	uint8_t    block[SM3_BLOCK_SIZE];  // Store the rest data which not enougn one block
	uint32_t   num;                    // Record how many bytes in block[]

} SM3_CTX;

void sm3_init(SM3_CTX *ctx);
void sm3_update(SM3_CTX *ctx, const uint8_t * data, uint32_t data_len);
void sm3_final(SM3_CTX *ctx, uint8_t digest[SM3_DIGEST_LENGTH]);
void sm3_compress(uint32_t digest[8], const uint8_t block[SM3_BLOCK_SIZE]);
void sm3(const uint8_t * data, uint32_t datalen, uint8_t digest[SM3_DIGEST_LENGTH]);




typedef struct _SM3_HMAC_CTX
{
	SM3_CTX sm3_ctx;
	uint8_t key[SM3_BLOCK_SIZE];

} SM3_HMAC_CTX;

void sm3_hmac_init(SM3_HMAC_CTX *ctx, const uint8_t *key, uint32_t key_len);
void sm3_hmac_update(SM3_HMAC_CTX *ctx, const uint8_t *data, uint32_t data_len);
void sm3_hmac_final(SM3_HMAC_CTX *ctx, unsigned char mac[SM3_HMAC_SIZE]);
void sm3_hmac(const uint8_t *data, uint32_t data_len, const uint8_t *key, uint32_t key_len, uint8_t mac[SM3_HMAC_SIZE]);





#endif // end of sm3.h