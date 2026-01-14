#include <stdio.h>
#include <time.h>

#ifdef TEST_WITH_GMSSL
// Use gmssl impl as ref
// Note: GmSSL headers have been modified to rename conflicting symbols
// (SM3_CTX -> GMSSL_SM3_CTX, sm2_sign -> gmssl_sm2_sign, etc.)
#include "gmssl/asn1.h"
#include "gmssl/rand.h"
#include "gmssl/error.h"
#include "gmssl/sm2.h"

// Undef all renaming macros so YCrypt headers are not affected
#undef SM3_CTX
#undef sm3_init
#undef sm3_update
#undef sm3_finish
#undef SM3_HMAC_CTX
#undef sm3_hmac_init
#undef sm3_hmac_update
#undef sm3_hmac_finish
#undef sm2_sign
#undef sm2_verify
#endif

#include "../include/sm2.h"
#include "../include/utils.h"

#ifdef TEST_WITH_OPENSSL
#include <openssl/evp.h>
#include <openssl/ec.h>
#include <openssl/bn.h>
#include <openssl/err.h>
#include <openssl/rand.h>
#include <openssl/param_build.h>
#include <openssl/core_names.h>
#include "../include/ecc.h"
#endif

#define MSG_LEN 10000
// #define NTESTS 500000
#define NTESTS 500


void sm2_single_test() {
	unsigned char message[MSG_LEN];
	srand(0);
	random_fill(message, MSG_LEN);
	unsigned char IDA[17] = "1234567812345678";
	int ret;

	puts("message: ");
	for(int i = 0; i < MSG_LEN; i++) {
		printf("0x%x, ", message[i]);
	}
	puts("\n");

	u1 id_dgst[32], dgst[32];
	// u32 da = { 0x667eeb532ee03680, 0xee8c369f729cdcb6, 0x10570e23664e918d, 0x01a5b14aba92a216 };
	PrivKey privkey;
	PubKey pubkey;
	SM2SIG sig;


	sm2_keypair(&pubkey, &privkey);

	puts("pubkey: ");
	print_AFPoint(&pubkey);
	puts("\n");

	puts("privkey: ");
	printBN(privkey.da.v, 4);
	puts("\n");

	sm2_sign(&sig, message, MSG_LEN, IDA, strlen((char*)IDA), &pubkey, &privkey);

	puts("sig: ");
	printf("r = \n");
	for(int i = 0; i < 4; i++) {
		printf("0x%llx, ", sig.r.v[i]);
	}
	puts("");
	printf("s = \n");
	for(int i = 0; i < 4; i++) {
		printf("0x%llx, ", sig.s.v[i]);
	}
	puts("");

	ret = sm2_verify(&sig, message, MSG_LEN, IDA, strlen((char*)IDA), &pubkey);
	if (ret != 1)
	{
		printf("change da, low level sm2_verify failed.\n");
	} else {
		printf("Success.\n");
	}
}

int sm2_self_check()
{
	unsigned char message[MSG_LEN];
	unsigned char IDA[17] = "1234567812345678";
	int ret, i;
	int fail = 0;

	PrivKey privkey;
	PubKey pubkey;
	SM2SIG sig;

	srand((unsigned)time(NULL));
	random_fill(message, MSG_LEN);

	puts("======== Self sign/verify correctness test =======");

	//change da
	for (i = 0; i < NTESTS; i++)
	{
		// generate keypair
        sm2_keypair(&pubkey, &privkey);

		// //caculate dgst
		// sm2_get_id_digest(IDA, strlen((char *)IDA), &pubkey, id_dgst);
		// sm2_get_message_digest(message, MSG_LEN, id_dgst, dgst);
		// sm2_sign(message, MSG_LEN, &sig, &privkey);

        sm2_sign(&sig, message, MSG_LEN, IDA, strlen((char*)IDA), &pubkey, &privkey);

		// ret = sm2_verify(message, MSG_LEN, &sig, &pubkey);
		ret = sm2_verify(&sig, message, MSG_LEN, IDA, strlen((char*)IDA), &pubkey);
		if (ret != 1)
		{
			fail += 1;
		}
	}

	if (fail == 0)
	{
		printf("[SUCCESS] SM2 change da test correct.\n");
	} else {
		printf("[ERROR] Change da, low level sm2_verify failed. Test total : %d, fail: %d, fail rate: %lf.\n", NTESTS, fail, (double)fail/NTESTS);
    }

	fail = 0;

	//change msg
	for (i = 0; i < NTESTS; i++)
	{
		random_fill(message, MSG_LEN);

		//caculate dgst
		// sm2_get_id_digest(IDA, strlen((char *)IDA), &pubkey, id_dgst);
		// sm2_get_message_digest(message, MSG_LEN, id_dgst, dgst);
		// sm2_sign(message, MSG_LEN, &sig, &privkey);

        sm2_sign(&sig, message, MSG_LEN, IDA, strlen((char*)IDA), &pubkey, &privkey);
		// ret = sm2_verify(message, MSG_LEN, &sig, &pubkey);
        ret = sm2_verify(&sig, message, MSG_LEN, IDA, strlen((char*)IDA), &pubkey);
		if (ret != 1)
		{
			fail += 1;
		}
	}

	if (fail == 0)
	{
		printf("[SUCCESS] SM2 change msg test correct.\n");
	} else {
		printf("[ERROR] Change msg, low level sm2_verify failed. Test total : %d, fail: %d, fail rate: %lf.\n", NTESTS, fail, (double)fail/NTESTS);
    }

	return 0;
}

int sm2_demo(unsigned char *message, size_t len) //demo for sign and verify
{
	unsigned char IDA[17] = "1234567812345678";
	int ret;

	u1 id_dgst[32], dgst[32];
	PrivKey privkey;
	PubKey pubkey;
	SM2SIG sig;

	srand((unsigned)time(NULL));

	// generate keypair
    sm2_keypair(&pubkey, &privkey);


	//caculate dgst
	// sm2_get_id_digest(IDA, strlen((char*)IDA), &pubkey, id_dgst);
	// sm2_get_message_digest(message, len, id_dgst, dgst);

	printf("[INFO] --- use random private key  ---- start sign  ---\n");
	// sm2_sign(dgst, MSG_LEN, &sig, &privkey);
    sm2_sign(&sig, message, MSG_LEN, IDA, strlen((char*)IDA), &pubkey, &privkey);
	printf("=========== private ============\n");
	for (int i = 0; i < 4; i++) {
		printf("%llu\n", privkey.da.v[i]);
	}

	printf("============== public key =============\n");
	printf("x : \n");
	for (int i = 0; i < 4; i++) {
		printf("%llu\n", pubkey.x.v[i]);
	}
	printf("y : \n");
	for (int i = 0; i < 4; i++) {
		printf("%llu\n", pubkey.y.v[i]);
	}

	printf("============= sign =============\n");
	printf("r : \n");
	for (int i = 0; i < 4; i++) {
		printf("%llu\n", sig.r.v[i]);
	}
	printf("s : \n");
	for (int i = 0; i < 4; i++) {
		printf("%llu\n", sig.s.v[i]);
	}

	ret = sm2_verify(&sig, message, MSG_LEN, IDA, strlen((char*)IDA), &pubkey);
	if (ret != 1)
	{
		printf("[ERROR] -- low level sm2_verify failed.\n");
	} else {
		puts("SM2 self test success.");
	}

	return 0;
}

#ifdef TEST_WITH_GMSSL
static int test_sm2_do_sign_gmssl(void)
{
	SM2_KEY sm2_key;
	uint8_t dgst[32];
	SM2_SIGNATURE sig;
	size_t i;

	puts("====== GmSSL sign verify self test =======");

	for (i = 0; i < NTESTS; i++) {

		if (sm2_key_generate(&sm2_key) != 1) {
			error_print();
			return -1;
		}
		rand_bytes(dgst, 32);

		if (sm2_do_sign(&sm2_key, dgst, &sig) != 1) {
			error_print();
			return -1;
		}
		if (sm2_do_verify(&sm2_key, dgst, &sig) != 1) {
			error_print();
			return -1;
		}
	}

	printf("%s() ok\n", __FUNCTION__);
	return 1;
}

void sm2_check_use_gmssl()
{
	int ret;
	int fail = 0;

	PrivKey priKey;
	PubKey pubKey;
	SM2SIG sig;

	SM2_KEY sm2_key_gmssl;
	uint8_t dgst[32];
	SM2_SIGNATURE sig_gmssl;
	SM2_Z256_AFFINE_POINT gmssl_pubkey;

    u1 data[32], ZA[32], SMX_SDUSM_dgst[32];

	printf("====== Check key generation =======\n");
	for (int i = 0; i < NTESTS; i++)
	{
		// Use GmSSL generate priv and pub key.
		sm2_key_generate(&sm2_key_gmssl);

		// Use generated priv key to generate self pub key.
		memcpy(priKey.da.v, sm2_key_gmssl.private_key, 32);

		// Get self pub key affine point.
		sm2_get_public_key(&priKey, &pubKey);

		// Get gmssl pub key affine point.
		sm2_z256_point_get_xy(&sm2_key_gmssl.public_key, gmssl_pubkey.x, gmssl_pubkey.y);

		// Check pub key correctness
		if(
			memcmp(&(pubKey.x), gmssl_pubkey.x, 32) ||
			memcmp(&(pubKey.y), gmssl_pubkey.y, 32)
		) {
			printf("[ERROR] Pub Key generation error (x).\n");
			fail += 1;
		}
	}

	if (fail == 0)
	{
		printf("[SUCCESS] SM2 pubkey generation test correct.\n");
	} else {
		printf("[ERROR] SM2 pubkey generation test total : %d, fail: %d, fail rate: %lf.\n", NTESTS, fail, (double)fail/NTESTS);
    }

	fail = 0;
	printf("====== Check sm2_sign use GmSSL verify method =====\n");

    // Random test.
	for (int i = 0; i < NTESTS; i++)
	{
        // GmSSL rand_bytes generator
		rand_bytes(data, 32);

        sm2_sign_dgst(&sig, data, &priKey);

		sm2_z256_to_bytes((uint64_t*)sig.r.v, sig_gmssl.r);
		sm2_z256_to_bytes((uint64_t*)sig.s.v, sig_gmssl.s);

		if (sm2_do_verify(&sm2_key_gmssl, data, &sig_gmssl) != 1) {
			fail += 1;
		}
	}

	if (fail == 0)
	{
		printf("[SUCCESS] SM2 verify test correct.\n");
	} else {
		printf("[ERROR] SM2 verify test total : %d, fail: %d, fail rate: %lf.\n", NTESTS, fail, (double)fail/NTESTS);
    }

	fail = 0;
	printf("====== Check sm2_verify use GmSSL signed sig as input =====\n");

    // Random test.
	for (int i = 0; i < NTESTS; i++)
	{
		// GmSSL rand_bytes generator
		rand_bytes(data, 32);

        if (sm2_do_sign(&sm2_key_gmssl, data, &sig_gmssl) != 1) {
			error_print();
			return;
		}

		sm2_z256_from_bytes((uint64_t*)sig.r.v, sig_gmssl.r);
		sm2_z256_from_bytes((uint64_t*)sig.s.v, sig_gmssl.s);

		ret = sm2_verify_dgst(&sig, data, &pubKey);
		if (ret != 1)
		{
			fail+=1;
		}
	}

	if (fail == 0)
	{
		printf("[SUCCESS] SM2 verify test correct.\n");
	} else {
		printf("[ERROR] SM2 verify test total : %d, fail: %d, fail rate: %lf.\n", NTESTS, fail, (double)fail/NTESTS);
    }
}
#endif

// ============================================================================
// OpenSSL Cross Verification Tests
// ============================================================================
#ifdef TEST_WITH_OPENSSL

// Convert u32 (little-endian u64[4]) to big-endian byte array (32 bytes)
static void ossl_u32_to_bytes_be(const u32* v, unsigned char out[32])
{
    for (int i = 0; i < 4; i++) {
        // v->v[3] is the most significant, v->v[0] is the least significant
        uint64_t val = v->v[3 - i];
        out[i * 8 + 0] = (val >> 56) & 0xff;
        out[i * 8 + 1] = (val >> 48) & 0xff;
        out[i * 8 + 2] = (val >> 40) & 0xff;
        out[i * 8 + 3] = (val >> 32) & 0xff;
        out[i * 8 + 4] = (val >> 24) & 0xff;
        out[i * 8 + 5] = (val >> 16) & 0xff;
        out[i * 8 + 6] = (val >> 8) & 0xff;
        out[i * 8 + 7] = val & 0xff;
    }
}

// Convert big-endian byte array (32 bytes) to u32 (little-endian u64[4])
static void ossl_bytes_be_to_u32(const unsigned char in[32], u32* v)
{
    for (int i = 0; i < 4; i++) {
        uint64_t val = 0;
        val |= ((uint64_t)in[i * 8 + 0]) << 56;
        val |= ((uint64_t)in[i * 8 + 1]) << 48;
        val |= ((uint64_t)in[i * 8 + 2]) << 40;
        val |= ((uint64_t)in[i * 8 + 3]) << 32;
        val |= ((uint64_t)in[i * 8 + 4]) << 24;
        val |= ((uint64_t)in[i * 8 + 5]) << 16;
        val |= ((uint64_t)in[i * 8 + 6]) << 8;
        val |= ((uint64_t)in[i * 8 + 7]);
        v->v[3 - i] = val;
    }
}

// Create OpenSSL EVP_PKEY from YCrypt private key
static EVP_PKEY* ossl_create_key_from_ycrypt(const PrivKey* privkey, const PubKey* pubkey)
{
    EVP_PKEY* pkey = NULL;
    EVP_PKEY_CTX* ctx = NULL;
    BIGNUM* priv_bn = NULL;
    BIGNUM* x_bn = NULL;
    BIGNUM* y_bn = NULL;
    unsigned char priv_bytes[32];
    unsigned char x_bytes[32];
    unsigned char y_bytes[32];

    // Convert to big-endian bytes
    ossl_u32_to_bytes_be(&privkey->da, priv_bytes);
    ossl_u32_to_bytes_be(&pubkey->x, x_bytes);
    ossl_u32_to_bytes_be(&pubkey->y, y_bytes);

    priv_bn = BN_bin2bn(priv_bytes, 32, NULL);
    x_bn = BN_bin2bn(x_bytes, 32, NULL);
    y_bn = BN_bin2bn(y_bytes, 32, NULL);

    if (!priv_bn || !x_bn || !y_bn) {
        goto cleanup;
    }

    // Build parameters for key generation
    OSSL_PARAM_BLD* param_bld = OSSL_PARAM_BLD_new();
    if (!param_bld) goto cleanup;

    // Set the group name
    if (!OSSL_PARAM_BLD_push_utf8_string(param_bld, "group", "SM2", 0)) {
        OSSL_PARAM_BLD_free(param_bld);
        goto cleanup;
    }

    // Set private key
    if (!OSSL_PARAM_BLD_push_BN(param_bld, "priv", priv_bn)) {
        OSSL_PARAM_BLD_free(param_bld);
        goto cleanup;
    }

    // Create public key in uncompressed format: 04 || x || y
    unsigned char pub_bytes[65];
    pub_bytes[0] = 0x04;
    memcpy(pub_bytes + 1, x_bytes, 32);
    memcpy(pub_bytes + 33, y_bytes, 32);

    if (!OSSL_PARAM_BLD_push_octet_string(param_bld, "pub", pub_bytes, 65)) {
        OSSL_PARAM_BLD_free(param_bld);
        goto cleanup;
    }

    OSSL_PARAM* params = OSSL_PARAM_BLD_to_param(param_bld);
    OSSL_PARAM_BLD_free(param_bld);
    if (!params) goto cleanup;

    ctx = EVP_PKEY_CTX_new_from_name(NULL, "SM2", NULL);
    if (!ctx) {
        OSSL_PARAM_free(params);
        goto cleanup;
    }

    if (EVP_PKEY_fromdata_init(ctx) <= 0) {
        OSSL_PARAM_free(params);
        goto cleanup;
    }

    if (EVP_PKEY_fromdata(ctx, &pkey, EVP_PKEY_KEYPAIR, params) <= 0) {
        OSSL_PARAM_free(params);
        goto cleanup;
    }

    OSSL_PARAM_free(params);

cleanup:
    BN_free(priv_bn);
    BN_free(x_bn);
    BN_free(y_bn);
    EVP_PKEY_CTX_free(ctx);
    return pkey;
}

// Create OpenSSL EVP_PKEY (public key only) from YCrypt public key
static EVP_PKEY* ossl_create_pubkey_from_ycrypt(const PubKey* pubkey)
{
    EVP_PKEY* pkey = NULL;
    EVP_PKEY_CTX* ctx = NULL;
    unsigned char x_bytes[32];
    unsigned char y_bytes[32];

    // Convert to big-endian bytes
    ossl_u32_to_bytes_be(&pubkey->x, x_bytes);
    ossl_u32_to_bytes_be(&pubkey->y, y_bytes);

    // Build parameters for key generation
    OSSL_PARAM_BLD* param_bld = OSSL_PARAM_BLD_new();
    if (!param_bld) goto cleanup;

    // Set the group name
    if (!OSSL_PARAM_BLD_push_utf8_string(param_bld, "group", "SM2", 0)) {
        OSSL_PARAM_BLD_free(param_bld);
        goto cleanup;
    }

    // Create public key in uncompressed format: 04 || x || y
    unsigned char pub_bytes[65];
    pub_bytes[0] = 0x04;
    memcpy(pub_bytes + 1, x_bytes, 32);
    memcpy(pub_bytes + 33, y_bytes, 32);

    if (!OSSL_PARAM_BLD_push_octet_string(param_bld, "pub", pub_bytes, 65)) {
        OSSL_PARAM_BLD_free(param_bld);
        goto cleanup;
    }

    OSSL_PARAM* params = OSSL_PARAM_BLD_to_param(param_bld);
    OSSL_PARAM_BLD_free(param_bld);
    if (!params) goto cleanup;

    ctx = EVP_PKEY_CTX_new_from_name(NULL, "SM2", NULL);
    if (!ctx) {
        OSSL_PARAM_free(params);
        goto cleanup;
    }

    if (EVP_PKEY_fromdata_init(ctx) <= 0) {
        OSSL_PARAM_free(params);
        goto cleanup;
    }

    if (EVP_PKEY_fromdata(ctx, &pkey, EVP_PKEY_PUBLIC_KEY, params) <= 0) {
        OSSL_PARAM_free(params);
        goto cleanup;
    }

    OSSL_PARAM_free(params);

cleanup:
    EVP_PKEY_CTX_free(ctx);
    return pkey;
}

// Extract private and public key from OpenSSL EVP_PKEY
static int ossl_extract_ycrypt_keys(EVP_PKEY* pkey, PrivKey* privkey, PubKey* pubkey)
{
    BIGNUM* priv_bn = NULL;
    unsigned char priv_bytes[32];
    unsigned char pub_bytes[65];
    size_t pub_len = 65;

    // Get private key
    if (!EVP_PKEY_get_bn_param(pkey, "priv", &priv_bn)) {
        return 0;
    }

    memset(priv_bytes, 0, 32);
    int priv_len = BN_num_bytes(priv_bn);
    BN_bn2bin(priv_bn, priv_bytes + (32 - priv_len));
    ossl_bytes_be_to_u32(priv_bytes, &privkey->da);
    BN_free(priv_bn);

    // Get public key
    if (!EVP_PKEY_get_octet_string_param(pkey, "pub", pub_bytes, sizeof(pub_bytes), &pub_len)) {
        return 0;
    }

    if (pub_len != 65 || pub_bytes[0] != 0x04) {
        return 0;
    }

    ossl_bytes_be_to_u32(pub_bytes + 1, &pubkey->x);
    ossl_bytes_be_to_u32(pub_bytes + 33, &pubkey->y);

    return 1;
}

// Encode raw r,s signature to DER format
static int ossl_encode_sig_to_der(const SM2SIG* sig, unsigned char* der, size_t* der_len)
{
    unsigned char r_bytes[32], s_bytes[32];
    ossl_u32_to_bytes_be(&sig->r, r_bytes);
    ossl_u32_to_bytes_be(&sig->s, s_bytes);

    ECDSA_SIG* ecdsa_sig = ECDSA_SIG_new();
    if (!ecdsa_sig) return 0;

    BIGNUM* r = BN_bin2bn(r_bytes, 32, NULL);
    BIGNUM* s = BN_bin2bn(s_bytes, 32, NULL);
    if (!r || !s) {
        BN_free(r);
        BN_free(s);
        ECDSA_SIG_free(ecdsa_sig);
        return 0;
    }

    if (!ECDSA_SIG_set0(ecdsa_sig, r, s)) {
        BN_free(r);
        BN_free(s);
        ECDSA_SIG_free(ecdsa_sig);
        return 0;
    }

    unsigned char* p = der;
    int len = i2d_ECDSA_SIG(ecdsa_sig, &p);
    ECDSA_SIG_free(ecdsa_sig);

    if (len <= 0) return 0;
    *der_len = len;
    return 1;
}

// Decode DER signature to raw r,s format
static int ossl_decode_der_to_sig(const unsigned char* der, size_t der_len, SM2SIG* sig)
{
    const unsigned char* p = der;
    ECDSA_SIG* ecdsa_sig = d2i_ECDSA_SIG(NULL, &p, der_len);
    if (!ecdsa_sig) return 0;

    const BIGNUM* r = ECDSA_SIG_get0_r(ecdsa_sig);
    const BIGNUM* s = ECDSA_SIG_get0_s(ecdsa_sig);

    unsigned char r_bytes[32] = {0}, s_bytes[32] = {0};
    int r_len = BN_num_bytes(r);
    int s_len = BN_num_bytes(s);

    BN_bn2bin(r, r_bytes + (32 - r_len));
    BN_bn2bin(s, s_bytes + (32 - s_len));

    ossl_bytes_be_to_u32(r_bytes, &sig->r);
    ossl_bytes_be_to_u32(s_bytes, &sig->s);

    ECDSA_SIG_free(ecdsa_sig);
    return 1;
}

// Sign digest with OpenSSL
static int ossl_sm2_sign_digest(EVP_PKEY* pkey, const unsigned char dgst[32],
                                unsigned char* sig, size_t* sig_len)
{
    EVP_PKEY_CTX* ctx = EVP_PKEY_CTX_new(pkey, NULL);
    if (!ctx) return 0;

    if (EVP_PKEY_sign_init(ctx) <= 0) {
        EVP_PKEY_CTX_free(ctx);
        return 0;
    }

    if (EVP_PKEY_sign(ctx, sig, sig_len, dgst, 32) <= 0) {
        EVP_PKEY_CTX_free(ctx);
        return 0;
    }

    EVP_PKEY_CTX_free(ctx);
    return 1;
}

// Verify digest with OpenSSL
static int ossl_sm2_verify_digest(EVP_PKEY* pkey, const unsigned char dgst[32],
                                  const unsigned char* sig, size_t sig_len)
{
    EVP_PKEY_CTX* ctx = EVP_PKEY_CTX_new(pkey, NULL);
    if (!ctx) return 0;

    if (EVP_PKEY_verify_init(ctx) <= 0) {
        EVP_PKEY_CTX_free(ctx);
        return 0;
    }

    int ret = EVP_PKEY_verify(ctx, sig, sig_len, dgst, 32);
    EVP_PKEY_CTX_free(ctx);
    return ret == 1;
}

// Test 1: Check public key generation matches
static int ossl_test_pubkey_generation()
{
    int fail = 0;
    printf("====== OpenSSL Test 1: Public key generation ======\n");

    for (int i = 0; i < NTESTS; i++) {
        // Generate key with OpenSSL
        EVP_PKEY* pkey = NULL;
        EVP_PKEY_CTX* ctx = EVP_PKEY_CTX_new_from_name(NULL, "SM2", NULL);
        if (!ctx) {
            fail++;
            continue;
        }

        if (EVP_PKEY_keygen_init(ctx) <= 0) {
            EVP_PKEY_CTX_free(ctx);
            fail++;
            continue;
        }

        if (EVP_PKEY_keygen(ctx, &pkey) <= 0) {
            EVP_PKEY_CTX_free(ctx);
            fail++;
            continue;
        }
        EVP_PKEY_CTX_free(ctx);

        // Extract keys to YCrypt format
        PrivKey privkey;
        PubKey pubkey_openssl;
        if (!ossl_extract_ycrypt_keys(pkey, &privkey, &pubkey_openssl)) {
            EVP_PKEY_free(pkey);
            fail++;
            continue;
        }

        // Generate public key with YCrypt
        PubKey pubkey_ycrypt;
        sm2_get_public_key(&privkey, &pubkey_ycrypt);

        // Compare
        if (!u32_eq(&pubkey_openssl.x, &pubkey_ycrypt.x) ||
            !u32_eq(&pubkey_openssl.y, &pubkey_ycrypt.y)) {
            fail++;
        }

        EVP_PKEY_free(pkey);
    }

    if (fail == 0) {
        printf("[SUCCESS] Public key generation matches OpenSSL.\n");
    } else {
        printf("[ERROR] Public key generation mismatch. Fail: %d/%d\n", fail, NTESTS);
    }

    return fail == 0;
}

// Test 2: YCrypt sign, OpenSSL verify
static int ossl_test_ycrypt_sign_openssl_verify()
{
    int fail = 0;
    printf("====== OpenSSL Test 2: YCrypt sign, OpenSSL verify ======\n");

    for (int i = 0; i < NTESTS; i++) {
        // Generate key with YCrypt
        PrivKey privkey;
        PubKey pubkey;
        sm2_keypair(&pubkey, &privkey);

        // Create OpenSSL key
        EVP_PKEY* pkey = ossl_create_pubkey_from_ycrypt(&pubkey);
        if (!pkey) {
            fail++;
            continue;
        }

        // Generate random digest
        unsigned char dgst[32];
        RAND_bytes(dgst, 32);

        // Sign with YCrypt
        SM2SIG sig;
        if (sm2_sign_dgst(&sig, dgst, &privkey) != 1) {
            EVP_PKEY_free(pkey);
            fail++;
            continue;
        }

        // Encode to DER
        unsigned char der_sig[128];
        size_t der_len;
        if (!ossl_encode_sig_to_der(&sig, der_sig, &der_len)) {
            EVP_PKEY_free(pkey);
            fail++;
            continue;
        }

        // Verify with OpenSSL
        if (!ossl_sm2_verify_digest(pkey, dgst, der_sig, der_len)) {
            fail++;
        }

        EVP_PKEY_free(pkey);
    }

    if (fail == 0) {
        printf("[SUCCESS] YCrypt signatures verified by OpenSSL.\n");
    } else {
        printf("[ERROR] YCrypt sign / OpenSSL verify failed. Fail: %d/%d\n", fail, NTESTS);
    }

    return fail == 0;
}

// Test 3: OpenSSL sign, YCrypt verify
static int ossl_test_openssl_sign_ycrypt_verify()
{
    int fail = 0;
    printf("====== OpenSSL Test 3: OpenSSL sign, YCrypt verify ======\n");

    for (int i = 0; i < NTESTS; i++) {
        // Generate key with OpenSSL
        EVP_PKEY* pkey = NULL;
        EVP_PKEY_CTX* ctx = EVP_PKEY_CTX_new_from_name(NULL, "SM2", NULL);
        if (!ctx) {
            fail++;
            continue;
        }

        if (EVP_PKEY_keygen_init(ctx) <= 0) {
            EVP_PKEY_CTX_free(ctx);
            fail++;
            continue;
        }

        if (EVP_PKEY_keygen(ctx, &pkey) <= 0) {
            EVP_PKEY_CTX_free(ctx);
            fail++;
            continue;
        }
        EVP_PKEY_CTX_free(ctx);

        // Extract public key for YCrypt
        PrivKey privkey;
        PubKey pubkey;
        if (!ossl_extract_ycrypt_keys(pkey, &privkey, &pubkey)) {
            EVP_PKEY_free(pkey);
            fail++;
            continue;
        }

        // Generate random digest
        unsigned char dgst[32];
        RAND_bytes(dgst, 32);

        // Sign with OpenSSL
        unsigned char der_sig[128];
        size_t der_len = sizeof(der_sig);
        if (!ossl_sm2_sign_digest(pkey, dgst, der_sig, &der_len)) {
            EVP_PKEY_free(pkey);
            fail++;
            continue;
        }

        // Decode DER to raw r,s
        SM2SIG sig;
        if (!ossl_decode_der_to_sig(der_sig, der_len, &sig)) {
            EVP_PKEY_free(pkey);
            fail++;
            continue;
        }

        // Verify with YCrypt
        if (sm2_verify_dgst(&sig, dgst, &pubkey) != 1) {
            fail++;
        }

        EVP_PKEY_free(pkey);
    }

    if (fail == 0) {
        printf("[SUCCESS] OpenSSL signatures verified by YCrypt.\n");
    } else {
        printf("[ERROR] OpenSSL sign / YCrypt verify failed. Fail: %d/%d\n", fail, NTESTS);
    }

    return fail == 0;
}

// Test 4: Round-trip test (same key, both directions)
static int ossl_test_roundtrip()
{
    int fail = 0;
    printf("====== OpenSSL Test 4: Round-trip cross verification ======\n");

    for (int i = 0; i < NTESTS; i++) {
        // Generate key with YCrypt
        PrivKey privkey;
        PubKey pubkey;
        sm2_keypair(&pubkey, &privkey);

        // Create OpenSSL key
        EVP_PKEY* pkey = ossl_create_key_from_ycrypt(&privkey, &pubkey);
        if (!pkey) {
            fail++;
            continue;
        }

        // Generate random digest
        unsigned char dgst[32];
        RAND_bytes(dgst, 32);

        // Test A: YCrypt sign -> OpenSSL verify
        SM2SIG sig_y;
        if (sm2_sign_dgst(&sig_y, dgst, &privkey) != 1) {
            EVP_PKEY_free(pkey);
            fail++;
            continue;
        }

        unsigned char der_sig[128];
        size_t der_len;
        if (!ossl_encode_sig_to_der(&sig_y, der_sig, &der_len)) {
            EVP_PKEY_free(pkey);
            fail++;
            continue;
        }

        if (!ossl_sm2_verify_digest(pkey, dgst, der_sig, der_len)) {
            fail++;
            EVP_PKEY_free(pkey);
            continue;
        }

        // Test B: OpenSSL sign -> YCrypt verify
        der_len = sizeof(der_sig);
        if (!ossl_sm2_sign_digest(pkey, dgst, der_sig, &der_len)) {
            EVP_PKEY_free(pkey);
            fail++;
            continue;
        }

        SM2SIG sig_o;
        if (!ossl_decode_der_to_sig(der_sig, der_len, &sig_o)) {
            EVP_PKEY_free(pkey);
            fail++;
            continue;
        }

        if (sm2_verify_dgst(&sig_o, dgst, &pubkey) != 1) {
            fail++;
        }

        EVP_PKEY_free(pkey);
    }

    if (fail == 0) {
        printf("[SUCCESS] Round-trip cross verification passed.\n");
    } else {
        printf("[ERROR] Round-trip test failed. Fail: %d/%d\n", fail, NTESTS);
    }

    return fail == 0;
}

void sm2_check_use_openssl()
{
    printf("\n======== SM2 Cross Verification with OpenSSL ========\n");
    printf("OpenSSL version: %s\n", OPENSSL_VERSION_TEXT);
    printf("Tests per case: %d\n\n", NTESTS);

    int success = 1;
    success &= ossl_test_pubkey_generation();
    success &= ossl_test_ycrypt_sign_openssl_verify();
    success &= ossl_test_openssl_sign_ycrypt_verify();
    success &= ossl_test_roundtrip();

    printf("\n======== OpenSSL Summary ========\n");
    if (success) {
        printf("All OpenSSL cross verification tests PASSED!\n");
    } else {
        printf("Some OpenSSL tests FAILED!\n");
    }
}
#endif

int main(int argc, char* argv[])
{
	// sm2_single_test();
	sm2_self_check();
#ifdef TEST_WITH_GMSSL
	// test_sm2_do_sign_gmssl();
	sm2_check_use_gmssl();
#endif
#ifdef TEST_WITH_OPENSSL
	sm2_check_use_openssl();
#endif
	return 0;
}
