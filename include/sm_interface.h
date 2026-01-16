
#ifndef _YCRYPT_H
#define _YCRYPT_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

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


// ===============================
// ============ SM2 ==============
// ===============================

typedef struct SM2SIG
{
    u32 r;
    u32 s;
} SM2SIG;

typedef struct PrivKey
{
    u32 da;
} PrivKey;

typedef AFPoint PubKey;

/**
 * Generate SM2 key pair (public key and private key)
 */
void sm2_keypair(PubKey* pubkey, PrivKey* privkey);

/**
 * Compute public key from private key
 */
void sm2_get_public_key(const PrivKey* privkey, PubKey* pubkey);

/**
 * Sign a message using SM2 algorithm
 * @return 1 on success, 0 on failure
 */
int sm2_sign(
    SM2SIG* sig,
    const u1* msg,
    size_t msglen,
    const u1* id,
    size_t idlen,
    const PubKey* pubkey,
    const PrivKey* privkey);

/**
 * Verify a message signature using SM2 algorithm
 * @return 1 if signature is valid, 0 if invalid
 */
int sm2_verify(
    const SM2SIG* sig,
    const u1* msg,
    size_t msg_len,
    const u1* id,
    size_t id_len,
    const PubKey* pubkey);

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
    u1 buffer[SM4_BLOCK_SIZE];
    size_t buffer_used;
} SM4_CTR_CTX;

/* SM4 CTR mode streaming API */
void sm4_ctr_init(SM4_CTR_CTX *ctx, const u1 key[SM4_KEY_SIZE], const u1 iv[SM4_BLOCK_SIZE]);
void sm4_ctr_update(SM4_CTR_CTX *ctx, const u1 *in, u1 *out, size_t len);
void sm4_ctr_clean(SM4_CTR_CTX *ctx);

/* SM4 CTR mode one-shot API */
size_t sm4_ctr(u1 *input, size_t len, u1 *output, u1 key[SM4_KEY_SIZE], u1 iv[SM4_BLOCK_SIZE]);
size_t sm4_ctr_once(const u1 *input, size_t len, u1 *output,
    const u1 key[SM4_KEY_SIZE], const u1 iv[SM4_BLOCK_SIZE]);

#ifdef __cplusplus
}
#endif

#endif
