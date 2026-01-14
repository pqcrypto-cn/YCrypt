#ifndef _SM_INTERFACE_H_
#define _SM_INTERFACE_H_

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Basic types
 */
typedef unsigned long long u8;
typedef unsigned char u1;

typedef struct {
    u8 v[4];
} u32;

typedef struct {
    u32 x;
    u32 y;
} AFPoint;

typedef struct {
    u32 r;
    u32 s;
} SM2SIG;

typedef struct {
    u32 da;
} PrivKey;

typedef AFPoint PubKey;

/*
 * SM2 Key Generation
 */

/**
 * Generate SM2 key pair (public key and private key)
 *
 * @param pubkey  [out] Generated public key
 * @param privkey [out] Generated private key
 */
void sm2_keypair(PubKey* pubkey, PrivKey* privkey);

/**
 * Compute public key from private key
 *
 * @param privkey [in]  Private key
 * @param pubkey  [out] Computed public key
 */
void sm2_get_public_key(const PrivKey* privkey, PubKey* pubkey);

/*
 * SM2 Sign and Verify
 */

/**
 * Sign a message using SM2 algorithm
 *
 * @param sig     [out] Output signature (r, s)
 * @param msg     [in]  Message to sign
 * @param msglen  [in]  Message length in bytes
 * @param id      [in]  User ID (e.g., "1234567812345678")
 * @param idlen   [in]  User ID length in bytes
 * @param pubkey  [in]  Public key (used for computing ZA)
 * @param privkey [in]  Private key (used for signing)
 *
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
 *
 * @param sig     [in] Signature to verify (r, s)
 * @param msg     [in] Original message
 * @param msg_len [in] Message length in bytes
 * @param id      [in] User ID
 * @param id_len  [in] User ID length in bytes
 * @param pubkey  [in] Public key for verification
 *
 * @return 1 if signature is valid, 0 if invalid
 */
int sm2_verify_msg(
    const SM2SIG* sig,
    const u1* msg,
    size_t msg_len,
    const u1* id,
    size_t id_len,
    const PubKey* pubkey);

#ifdef __cplusplus
}
#endif

#endif /* _SM_INTERFACE_H_ */
