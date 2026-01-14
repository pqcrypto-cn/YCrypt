
#ifndef _YCRYPT_H
#define _YCRYPT_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

typedef unsigned long long	u8;
typedef unsigned int		u4;
typedef unsigned short		u2;
typedef unsigned char		u1;


// In bytes
#define SM2_BLOCK_SIZE		32UL
#define SM3_BLOCK_SIZE	    64UL
#define SM3_DIGEST_LENGTH	32UL
#define SM3_HMAC_SIZE		32UL
#define SM4_BLOCK_SIZE		16UL
#define SM4_KEY_SIZE		16UL
#define SM4_KEY_SCHEDULE    32UL

typedef struct
{
	u8 v[4];
} u32;

typedef struct
{
	u32 x;
	u32 y;
} AFPoint;


typedef struct SM2SIG
{
    u1 r[32];
    u1 s[32];
}SM2SIG;

// data here should be sm3 digest.
void sm2_sign	(u1 * data, size_t len, SM2SIG * signature, const AFPoint * priKey);
// data here should be sm3 digest.
bool sm2_verify	(u1 * data, size_t len, SM2SIG * signature, const AFPoint * pubKey);

// ===============================
// ============ SM3 ==============
// ===============================
size_t sm3		(const u1 * data, size_t len, u1 digest[SM3_DIGEST_LENGTH]);
size_t sm3_hmac	(const u1 * data, size_t len, const u1 * key, size_t keyLen, u1 mac[SM3_HMAC_SIZE]);


// ===============================
// ============ SM4 ==============
// ===============================
/* SM4 ECB mode */
void sm4_key_schedule(const u1 key[SM4_KEY_SIZE], uint32_t rk[SM4_KEY_SCHEDULE]);
void sm4_encrypt(const uint32_t rk[SM4_KEY_SCHEDULE],
    const u1 plaintext[SM4_BLOCK_SIZE], u1 ciphertext[SM4_BLOCK_SIZE]);
void sm4_decrypt(const uint32_t rk[SM4_KEY_SCHEDULE],
    const u1 ciphertext[SM4_BLOCK_SIZE], u1 plaintext[SM4_BLOCK_SIZE]);

/* SM4 CTR mode context */
typedef struct {
    uint32_t rk[SM4_KEY_SCHEDULE];
    u1 counter[SM4_BLOCK_SIZE];
    u1 keystream[SM4_BLOCK_SIZE];
    size_t keystream_used;
} SM4_CTR_CTX;

/* SM4 CTR mode streaming API */
void sm4_ctr_init(SM4_CTR_CTX *ctx, const u1 key[SM4_KEY_SIZE], const u1 iv[SM4_BLOCK_SIZE]);
void sm4_ctr_update(SM4_CTR_CTX *ctx, const u1 *in, u1 *out, size_t len);
void sm4_ctr_clean(SM4_CTR_CTX *ctx);

/* SM4 CTR mode one-shot API */
size_t sm4_ctr(u1 *input, size_t len, u1 *output, u1 key[SM4_KEY_SIZE], u1 iv[SM4_BLOCK_SIZE]);
size_t sm4_ctr_once(const u1 *input, size_t len, u1 *output,
    const u1 key[SM4_KEY_SIZE], const u1 iv[SM4_BLOCK_SIZE]);

#endif
