/*
 * SM2 Internal Header
 *
 * Public API is defined in include/sm_interface.h
 * This header is for internal use within the sm2 module.
 */
#ifndef _SM2_H_
#define _SM2_H_

#include "sm3.h"
#include "ecc.h"
#include "dataType.h"

/* Internal helper functions */
int sm2_get_id_digest(
    u1 result[32],
    const u1 *id,
    size_t id_len,
    const PubKey* pubkey);

int sm2_get_message_digest(
    u1 result[32],
    const u1 ZA[32],
    const u1 *msg,
    size_t msg_len);

/* Sign/verify on raw digest (internal use) */
int sm2_sign_dgst(
    SM2SIG *sig,
    u1 dgst[32],
    const PrivKey* priv_key);

int sm2_verify_dgst(
    const SM2SIG *signature,
    const u1 dgst[32],
    const AFPoint * pubkey);

/* Public API - also declared in include/sm_interface.h */
void sm2_keypair(PubKey* pubkey, PrivKey *privkey);
void sm2_get_public_key(const PrivKey *privkey, PubKey* pubkey);
int sm2_sign(
    SM2SIG *sig,
    const u1 *msg,
    size_t msglen,
    const u1 *id,
    size_t idlen,
    const PubKey* pubkey,
    const PrivKey* privkey);
int sm2_verify(
    const SM2SIG *sig,
    const u1 *msg,
    size_t msg_len,
    const u1 *id,
    size_t id_len,
    const PubKey* pubkey);

#endif