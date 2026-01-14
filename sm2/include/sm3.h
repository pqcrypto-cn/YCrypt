/*
 * SM3 Header for SM2 module
 * Uses the SM3 implementation from sm3/ directory
 */
#ifndef _SM3_H_
#define _SM3_H_

#include <stdint.h>
#include <string.h>
#include <stddef.h>
#include "dataType.h"

#define SM3_DIGEST_LENGTH  32    // In bytes
#define SM3_BLOCK_SIZE     64    // In bytes
#define SM3_CBLOCK         (SM3_BLOCK_SIZE)
#define SM3_HMAC_SIZE      (SM3_DIGEST_LENGTH)

#define cpu_to_be32(v) (((v)>>24) | (((v)>>8)&0xff00) | (((v)<<8)&0xff0000) | ((v)<<24))

typedef struct _SM3_CTX
{
	uint32_t   digest[8];
	size_t     nblocks;                // How many blocks have been compressed
	uint8_t    block[SM3_BLOCK_SIZE];  // Store the rest data which not enough one block
	size_t     num;                    // Record how many bytes in block[]
} SM3_CTX;

void sm3_init(SM3_CTX *ctx);
void sm3_update(SM3_CTX *ctx, const u1 *data, size_t data_len);
void sm3_final(SM3_CTX *ctx, u1 digest[SM3_DIGEST_LENGTH]);
void sm3_compress(u4 digest[8], const u1* block, size_t nblocks);
size_t sm3(const u1 *data, size_t datalen, u1 dgst[SM3_DIGEST_LENGTH]);

typedef struct _SM3_HMAC_CTX
{
	SM3_CTX sm3_ctx;
	u1 key[SM3_BLOCK_SIZE];
} SM3_HMAC_CTX;

void sm3_hmac_init(SM3_HMAC_CTX *ctx, const u1 *key, size_t key_len);
void sm3_hmac_update(SM3_HMAC_CTX *ctx, const u1 *data, size_t data_len);
void sm3_hmac_final(SM3_HMAC_CTX *ctx, u1 mac[SM3_HMAC_SIZE]);
size_t sm3_hmac(const u1 *data, size_t data_len, const u1 *key, size_t key_len, u1 mac[SM3_HMAC_SIZE]);

#endif // end of sm3.h
