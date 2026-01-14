/**
 * SM2 Performance Benchmark
 * - Key generation speed test
 * - Sign speed test
 * - Verify speed test
 * - Optional comparison with OpenSSL
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "../include/sm2.h"
#include "../include/utils.h"

/* Benchmark configuration */
#define MIN_BENCH_TIME  2.0    /* Minimum seconds to run each benchmark */
#define MSG_LEN         32     /* Message length (digest size) */

/* ============================================================
 * OpenSSL for comparison
 * ============================================================ */

#ifdef TEST_WITH_OPENSSL
#include <openssl/evp.h>
#include <openssl/ec.h>
#include <openssl/bn.h>
#include <openssl/rand.h>
#include <openssl/err.h>

static EVP_PKEY* ossl_sm2_keygen(void)
{
    EVP_PKEY *pkey = NULL;
    EVP_PKEY_CTX *ctx = EVP_PKEY_CTX_new_from_name(NULL, "SM2", NULL);
    if (!ctx) return NULL;

    if (EVP_PKEY_keygen_init(ctx) <= 0) {
        EVP_PKEY_CTX_free(ctx);
        return NULL;
    }

    if (EVP_PKEY_keygen(ctx, &pkey) <= 0) {
        EVP_PKEY_CTX_free(ctx);
        return NULL;
    }

    EVP_PKEY_CTX_free(ctx);
    return pkey;
}

static int ossl_sm2_sign(EVP_PKEY *pkey, const uint8_t *dgst, size_t dgst_len,
                         uint8_t *sig, size_t *sig_len)
{
    EVP_PKEY_CTX *ctx = EVP_PKEY_CTX_new(pkey, NULL);
    if (!ctx) return 0;

    if (EVP_PKEY_sign_init(ctx) <= 0) {
        EVP_PKEY_CTX_free(ctx);
        return 0;
    }

    if (EVP_PKEY_sign(ctx, sig, sig_len, dgst, dgst_len) <= 0) {
        EVP_PKEY_CTX_free(ctx);
        return 0;
    }

    EVP_PKEY_CTX_free(ctx);
    return 1;
}

static int ossl_sm2_verify(EVP_PKEY *pkey, const uint8_t *dgst, size_t dgst_len,
                           const uint8_t *sig, size_t sig_len)
{
    EVP_PKEY_CTX *ctx = EVP_PKEY_CTX_new(pkey, NULL);
    if (!ctx) return 0;

    if (EVP_PKEY_verify_init(ctx) <= 0) {
        EVP_PKEY_CTX_free(ctx);
        return 0;
    }

    int ret = EVP_PKEY_verify(ctx, sig, sig_len, dgst, dgst_len);
    EVP_PKEY_CTX_free(ctx);
    return ret == 1;
}
#endif

/* ============================================================
 * Utility functions
 * ============================================================ */

static void print_speed(const char *name, double ops_per_sec)
{
    printf("  %-35s %10.2f ops/s  (%6.2f us/op)\n",
           name, ops_per_sec, 1000000.0 / ops_per_sec);
}

static double get_time_sec(void)
{
    return (double)clock() / CLOCKS_PER_SEC;
}

/* ============================================================
 * Key Generation Benchmark
 * ============================================================ */

static void bench_sm2_keygen(void)
{
    printf("\n========== SM2 Key Generation Benchmark ==========\n");

    PrivKey privkey;
    PubKey pubkey;

    /* Warm up */
    for (int i = 0; i < 100; i++) {
        sm2_keypair(&pubkey, &privkey);
    }

    /* Benchmark YCrypt */
    {
        uint64_t iterations = 0;
        double start = get_time_sec();
        double elapsed;

        do {
            sm2_keypair(&pubkey, &privkey);
            iterations++;
            elapsed = get_time_sec() - start;
        } while (elapsed < MIN_BENCH_TIME);

        double ops_per_sec = iterations / elapsed;
        print_speed("sm2_keypair (YCrypt)", ops_per_sec);
    }

#ifdef TEST_WITH_OPENSSL
    /* Benchmark OpenSSL */
    {
        uint64_t iterations = 0;
        double start = get_time_sec();
        double elapsed;

        do {
            EVP_PKEY *pkey = ossl_sm2_keygen();
            if (pkey) {
                EVP_PKEY_free(pkey);
                iterations++;
            }
            elapsed = get_time_sec() - start;
        } while (elapsed < MIN_BENCH_TIME);

        double ops_per_sec = iterations / elapsed;
        print_speed("sm2_keygen (OpenSSL)", ops_per_sec);
    }
#endif
}

/* ============================================================
 * Sign Benchmark
 * ============================================================ */

static void bench_sm2_sign(void)
{
    printf("\n========== SM2 Sign Benchmark ==========\n");

    uint8_t dgst[MSG_LEN];
    PrivKey privkey;
    PubKey pubkey;
    SM2SIG sig;

    /* Generate test data */
    srand((unsigned)time(NULL));
    random_fill(dgst, MSG_LEN);
    sm2_keypair(&pubkey, &privkey);

    /* Warm up */
    for (int i = 0; i < 100; i++) {
        sm2_sign_dgst(&sig, dgst, &privkey);
    }

    /* Benchmark YCrypt */
    {
        uint64_t iterations = 0;
        double start = get_time_sec();
        double elapsed;

        do {
            sm2_sign_dgst(&sig, dgst, &privkey);
            iterations++;
            elapsed = get_time_sec() - start;
        } while (elapsed < MIN_BENCH_TIME);

        double ops_per_sec = iterations / elapsed;
        print_speed("sm2_sign_dgst (YCrypt)", ops_per_sec);
    }

#ifdef TEST_WITH_OPENSSL
    /* Benchmark OpenSSL */
    {
        EVP_PKEY *pkey = ossl_sm2_keygen();
        if (!pkey) {
            printf("  [ERROR] Failed to generate OpenSSL key\n");
            return;
        }

        uint8_t ossl_sig[128];
        size_t ossl_sig_len;

        /* Warm up */
        for (int i = 0; i < 100; i++) {
            ossl_sig_len = sizeof(ossl_sig);
            ossl_sm2_sign(pkey, dgst, MSG_LEN, ossl_sig, &ossl_sig_len);
        }

        uint64_t iterations = 0;
        double start = get_time_sec();
        double elapsed;

        do {
            ossl_sig_len = sizeof(ossl_sig);
            ossl_sm2_sign(pkey, dgst, MSG_LEN, ossl_sig, &ossl_sig_len);
            iterations++;
            elapsed = get_time_sec() - start;
        } while (elapsed < MIN_BENCH_TIME);

        double ops_per_sec = iterations / elapsed;
        print_speed("sm2_sign (OpenSSL)", ops_per_sec);

        EVP_PKEY_free(pkey);
    }
#endif
}

/* ============================================================
 * Verify Benchmark
 * ============================================================ */

static void bench_sm2_verify(void)
{
    printf("\n========== SM2 Verify Benchmark ==========\n");

    uint8_t dgst[MSG_LEN];
    PrivKey privkey;
    PubKey pubkey;
    SM2SIG sig;

    /* Generate test data */
    srand((unsigned)time(NULL));
    random_fill(dgst, MSG_LEN);
    sm2_keypair(&pubkey, &privkey);
    sm2_sign_dgst(&sig, dgst, &privkey);

    /* Warm up */
    for (int i = 0; i < 100; i++) {
        sm2_verify_dgst(&sig, dgst, &pubkey);
    }

    /* Benchmark YCrypt */
    {
        uint64_t iterations = 0;
        double start = get_time_sec();
        double elapsed;

        do {
            sm2_verify_dgst(&sig, dgst, &pubkey);
            iterations++;
            elapsed = get_time_sec() - start;
        } while (elapsed < MIN_BENCH_TIME);

        double ops_per_sec = iterations / elapsed;
        print_speed("sm2_verify_dgst (YCrypt)", ops_per_sec);
    }

#ifdef TEST_WITH_OPENSSL
    /* Benchmark OpenSSL */
    {
        EVP_PKEY *pkey = ossl_sm2_keygen();
        if (!pkey) {
            printf("  [ERROR] Failed to generate OpenSSL key\n");
            return;
        }

        uint8_t ossl_sig[128];
        size_t ossl_sig_len = sizeof(ossl_sig);

        /* Sign with OpenSSL first */
        if (!ossl_sm2_sign(pkey, dgst, MSG_LEN, ossl_sig, &ossl_sig_len)) {
            printf("  [ERROR] Failed to sign with OpenSSL\n");
            EVP_PKEY_free(pkey);
            return;
        }

        /* Warm up */
        for (int i = 0; i < 100; i++) {
            ossl_sm2_verify(pkey, dgst, MSG_LEN, ossl_sig, ossl_sig_len);
        }

        uint64_t iterations = 0;
        double start = get_time_sec();
        double elapsed;

        do {
            ossl_sm2_verify(pkey, dgst, MSG_LEN, ossl_sig, ossl_sig_len);
            iterations++;
            elapsed = get_time_sec() - start;
        } while (elapsed < MIN_BENCH_TIME);

        double ops_per_sec = iterations / elapsed;
        print_speed("sm2_verify (OpenSSL)", ops_per_sec);

        EVP_PKEY_free(pkey);
    }
#endif
}

/* ============================================================
 * Full Sign+Verify (with message hashing)
 * ============================================================ */

static void bench_sm2_sign_msg(void)
{
    printf("\n========== SM2 Sign Message Benchmark ==========\n");

    uint8_t message[1024];
    uint8_t id[] = "1234567812345678";
    PrivKey privkey;
    PubKey pubkey;
    SM2SIG sig;

    /* Generate test data */
    srand((unsigned)time(NULL));
    random_fill(message, sizeof(message));
    sm2_keypair(&pubkey, &privkey);

    /* Warm up */
    for (int i = 0; i < 50; i++) {
        sm2_sign(&sig, message, sizeof(message), id, strlen((char*)id), &pubkey, &privkey);
    }

    /* Benchmark YCrypt sign with message */
    {
        uint64_t iterations = 0;
        double start = get_time_sec();
        double elapsed;

        do {
            sm2_sign(&sig, message, sizeof(message), id, strlen((char*)id), &pubkey, &privkey);
            iterations++;
            elapsed = get_time_sec() - start;
        } while (elapsed < MIN_BENCH_TIME);

        double ops_per_sec = iterations / elapsed;
        print_speed("sm2_sign (msg, YCrypt)", ops_per_sec);
    }

    /* Warm up */
    for (int i = 0; i < 50; i++) {
        sm2_verify(&sig, message, sizeof(message), id, strlen((char*)id), &pubkey);
    }

    /* Benchmark YCrypt verify with message */
    {
        uint64_t iterations = 0;
        double start = get_time_sec();
        double elapsed;

        do {
            sm2_verify(&sig, message, sizeof(message), id, strlen((char*)id), &pubkey);
            iterations++;
            elapsed = get_time_sec() - start;
        } while (elapsed < MIN_BENCH_TIME);

        double ops_per_sec = iterations / elapsed;
        print_speed("sm2_verify (msg, YCrypt)", ops_per_sec);
    }
}

/* ============================================================
 * Main
 * ============================================================ */

int main(void)
{
    printf("============================================\n");
    printf("        SM2 Performance Benchmark\n");
    printf("============================================\n");
    printf("  Min bench time: %.1f sec\n", MIN_BENCH_TIME);
#ifdef TEST_WITH_OPENSSL
    printf("  OpenSSL comparison: enabled\n");
#else
    printf("  OpenSSL comparison: disabled\n");
    printf("  (compile with TEST_WITH_OPENSSL=1 to enable)\n");
#endif

    bench_sm2_keygen();
    bench_sm2_sign();
    bench_sm2_verify();
    bench_sm2_sign_msg();

    printf("\n============================================\n");
    printf("        Benchmark Complete\n");
    printf("============================================\n");

    return 0;
}
