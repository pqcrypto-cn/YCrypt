#ifndef _SM_INTERFACE_H_
#define _SM_INTERFACE_H_

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdio.h>
#include "../include/dataType.h"

// typedef unsigned long long	u8;
// typedef unsigned int		u4;
// typedef unsigned short		u2;
// typedef unsigned char		u1;

// typedef struct
// {
// 	u8 v[4];
// } u32;

// typedef struct
// {
// 	u32 x;
// 	u32 y;
// } AFPoint;

const static size_t SM3_DIGEST_LENGTH	= 32;
const static size_t SM3_HMAC_SIZE		= 32;
const static size_t SM4_BLOCK_SIZE		= 16;

// Both sm2_sign and sm2_verify should check the validity of pubkey/prikey!
void sm2_keypair(AFPoint* pubKey, u32 *d);
bool sm2_sign		(u1 * data, size_t len, AFPoint * signature, AFPoint * priKey);
bool sm2_verify		(u1 * data, size_t len, AFPoint * signature, AFPoint * pubKey);

// // Both sm3 and sm3_hmac only need to consider buffers with a byte (8-bit) as the basic unit
// size_t sm3			(u1 * data, size_t datalen, u1 digest[SM3_DIGEST_LENGTH]);
// size_t sm3_hmac		(u1 * data, size_t datalen, u1 * key, size_t key_len, u1 mac[SM3_HMAC_SIZE]);

// // Please implement SM4 CTR mode
// // Note that input and output buffers may overlap!
// size_t sm4_ctr	( u1 * input, size_t len, u1 * output, u1 key[SM4_BLOCK_SIZE], u1 IV[SM4_BLOCK_SIZE] );

#endif
// end of SM interface definitions