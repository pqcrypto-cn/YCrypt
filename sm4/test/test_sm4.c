/**
 * SM4 Correctness Tests
 * - ECB mode self-test with standard test vectors
 * - CTR mode test against OpenSSL implementation
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "../include/sm4.h"
#include "../include/sm4_ctr.h"
#include "../include/sm4_cbc.h"

#define NTESTS 10000

/* Test result macros */
#define TEST_PASS(name) printf("[PASS] %s\n", name)
#define TEST_FAIL(name) printf("[FAIL] %s\n", name)

/* Generate random bytes */
static void random_bytes(uint8_t *data, size_t len)
{
    for (size_t i = 0; i < len; i++) {
        data[i] = (uint8_t)(rand() % 256);
    }
}

/* Print hex data for debugging */
static void print_hex(const char *label, const uint8_t *data, size_t len)
{
    printf("%s: ", label);
    for (size_t i = 0; i < len; i++) {
        printf("%02X", data[i]);
    }
    printf("\n");
}

/* ============================================================
 * OpenSSL reference implementation for comparison
 * ============================================================ */

#ifdef TEST_WITH_OPENSSL
#include <openssl/evp.h>

static int openssl_sm4_ctr(const uint8_t *input, size_t len, uint8_t *output,
                           const uint8_t *key, const uint8_t *iv)
{
    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    if (!ctx) return -1;

    if (EVP_EncryptInit_ex(ctx, EVP_sm4_ctr(), NULL, key, iv) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        return -1;
    }

    int outlen, tmplen;
    if (EVP_EncryptUpdate(ctx, output, &outlen, input, (int)len) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        return -1;
    }

    if (EVP_EncryptFinal_ex(ctx, output + outlen, &tmplen) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        return -1;
    }

    EVP_CIPHER_CTX_free(ctx);
    return 0;
}

static int openssl_sm4_cbc_encrypt(const uint8_t *input, size_t len, uint8_t *output,
                                    const uint8_t *key, const uint8_t *iv)
{
    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    if (!ctx) return -1;

    if (EVP_EncryptInit_ex(ctx, EVP_sm4_cbc(), NULL, key, iv) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        return -1;
    }

    /* Disable padding for CBC mode */
    EVP_CIPHER_CTX_set_padding(ctx, 0);

    int outlen, tmplen;
    if (EVP_EncryptUpdate(ctx, output, &outlen, input, (int)len) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        return -1;
    }

    if (EVP_EncryptFinal_ex(ctx, output + outlen, &tmplen) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        return -1;
    }

    EVP_CIPHER_CTX_free(ctx);
    return outlen + tmplen;
}

static int openssl_sm4_cbc_decrypt(const uint8_t *input, size_t len, uint8_t *output,
                                    const uint8_t *key, const uint8_t *iv)
{
    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    if (!ctx) return -1;

    if (EVP_DecryptInit_ex(ctx, EVP_sm4_cbc(), NULL, key, iv) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        return -1;
    }

    /* Disable padding for CBC mode */
    EVP_CIPHER_CTX_set_padding(ctx, 0);

    int outlen, tmplen;
    if (EVP_DecryptUpdate(ctx, output, &outlen, input, (int)len) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        return -1;
    }

    if (EVP_DecryptFinal_ex(ctx, output + outlen, &tmplen) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        return -1;
    }

    EVP_CIPHER_CTX_free(ctx);
    return outlen + tmplen;
}
#endif

/* ============================================================
 * SM4 ECB Tests (standard test vectors)
 * ============================================================ */

static int test_sm4_ecb(void)
{
    printf("\n========== SM4 ECB Correctness Test ==========\n");

    /* Standard test vector from GM/T 0002-2012 */
    const uint8_t key[16] = {
        0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF,
        0xFE, 0xDC, 0xBA, 0x98, 0x76, 0x54, 0x32, 0x10
    };

    const uint8_t plaintext[16] = {
        0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF,
        0xFE, 0xDC, 0xBA, 0x98, 0x76, 0x54, 0x32, 0x10
    };

    const uint8_t expected_ciphertext[16] = {
        0x68, 0x1E, 0xDF, 0x34, 0xD2, 0x06, 0x96, 0x5E,
        0x86, 0xB3, 0xE9, 0x4F, 0x53, 0x6E, 0x42, 0x46
    };

    /* Expected after 1,000,000 iterations */
    const uint8_t expected_1m[16] = {
        0x59, 0x52, 0x98, 0xC7, 0xC6, 0xFD, 0x27, 0x1F,
        0x04, 0x02, 0xF8, 0x04, 0xC3, 0x3D, 0x3F, 0x66
    };

    uint8_t buf[16], dec[16];
    uint32_t rk[SM4_KEY_SCHEDULE];
    int pass = 1;

    sm4_key_schedule(key, rk);

    /* Test 1: Single encryption */
    sm4_encrypt(rk, plaintext, buf);
    if (memcmp(buf, expected_ciphertext, 16) != 0) {
        TEST_FAIL("sm4_encrypt single block");
        print_hex("Expected", expected_ciphertext, 16);
        print_hex("Got     ", buf, 16);
        pass = 0;
    } else {
        TEST_PASS("sm4_encrypt single block");
    }

    /* Test 2: Single decryption */
    sm4_decrypt(rk, buf, dec);
    if (memcmp(dec, plaintext, 16) != 0) {
        TEST_FAIL("sm4_decrypt single block");
        pass = 0;
    } else {
        TEST_PASS("sm4_decrypt single block");
    }

    /* Test 3: 1,000,000 iterations */
    memcpy(buf, plaintext, 16);
    for (int i = 0; i < 1000000; i++) {
        sm4_encrypt(rk, buf, buf);
    }
    if (memcmp(buf, expected_1m, 16) != 0) {
        TEST_FAIL("sm4_encrypt 1M iterations");
        print_hex("Expected", expected_1m, 16);
        print_hex("Got     ", buf, 16);
        pass = 0;
    } else {
        TEST_PASS("sm4_encrypt 1M iterations");
    }

    return pass;
}

/* ============================================================
 * SM4 CTR Tests
 * ============================================================ */

static int sm4_ctr_self_check(void)
{
    printf("\n========== SM4 CTR Self Check ==========\n");

    const uint8_t key[16] = {
        0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF,
        0xFE, 0xDC, 0xBA, 0x98, 0x76, 0x54, 0x32, 0x10
    };
    const uint8_t iv[16] = {
        0x12, 0x34, 0x56, 0x78, 0x9A, 0xBC, 0xDE, 0xF0,
        0xF0, 0xDE, 0xBC, 0x9A, 0x78, 0x56, 0x34, 0x12
    };

    int pass = 1;

    /* ========== OpenSSL generated test vectors ========== */
    printf("[+] Testing with OpenSSL generated test vectors\n");

    /* Test case 1: "Hello, SM4 CTR!1234567" */
    {
        uint8_t input1[] = "Hello, SM4 CTR!1234567";
        size_t len1 = sizeof(input1);
        uint8_t encrypted1[50] = {0};
        const uint8_t expected1[] = {
            0xBA, 0x9D, 0x7B, 0xEB, 0xC3, 0x40, 0xDC, 0xC4,
            0xFF, 0xC2, 0x45, 0xAA, 0xC7, 0xB5, 0xC1, 0xCB,
            0xA1, 0x9F, 0x16, 0xAE, 0x08, 0x79, 0x57
        };

        sm4_ctr_once(input1, len1, encrypted1, key, iv);

        if (memcmp(encrypted1, expected1, len1) != 0) {
            TEST_FAIL("CTR test vector 1 (Hello string)");
            print_hex("Expected", expected1, len1);
            print_hex("Got     ", encrypted1, len1);
            pass = 0;
        } else {
            TEST_PASS("CTR test vector 1 (Hello string)");
        }
    }

    /* Test case 2: empty string (1 byte - null terminator) */
    {
        uint8_t input2[] = "";
        size_t len2 = sizeof(input2);
        uint8_t encrypted2[50] = {0};
        const uint8_t expected2[] = {0xF2};

        sm4_ctr_once(input2, len2, encrypted2, key, iv);

        if (memcmp(encrypted2, expected2, len2) != 0) {
            TEST_FAIL("CTR test vector 2 (empty string)");
            print_hex("Expected", expected2, len2);
            print_hex("Got     ", encrypted2, len2);
            pass = 0;
        } else {
            TEST_PASS("CTR test vector 2 (empty string)");
        }
    }

    /* Test case 3: 100 bytes of zeros */
    {
        uint8_t input3[100] = {0};
        size_t len3 = sizeof(input3);
        uint8_t encrypted3[100] = {0};
        const uint8_t expected3[] = {
            0xF2, 0xF8, 0x17, 0x87, 0xAC, 0x6C, 0xFC, 0x97,
            0xB2, 0xF6, 0x65, 0xE9, 0x93, 0xE7, 0xE0, 0xFA,
            0x93, 0xAC, 0x22, 0x9B, 0x3E, 0x4E, 0x57, 0x12,
            0xA5, 0x83, 0x7C, 0xEF, 0xE8, 0x87, 0x28, 0x23,
            0xF6, 0xB6, 0xF2, 0xCB, 0x9E, 0xA8, 0x8D, 0xAF,
            0xA1, 0x8D, 0x52, 0xEA, 0x5E, 0x01, 0x79, 0x72,
            0x0F, 0xD9, 0xA2, 0xC2, 0xB6, 0x3E, 0x06, 0x15,
            0x64, 0xCD, 0x4B, 0x6A, 0x25, 0x4F, 0x0B, 0x63,
            0x73, 0x77, 0xE7, 0x9E, 0x95, 0x29, 0x5D, 0x1A,
            0x70, 0x58, 0x2B, 0xDD, 0x1D, 0x5C, 0xCD, 0x28,
            0x77, 0xBA, 0x61, 0xDA, 0x30, 0x9E, 0x33, 0x1A,
            0x14, 0xDB, 0xD6, 0xB6, 0x1A, 0xB2, 0xE4, 0x21,
            0xEB, 0xB8, 0x01, 0x36
        };

        sm4_ctr_once(input3, len3, encrypted3, key, iv);

        if (memcmp(encrypted3, expected3, len3) != 0) {
            TEST_FAIL("CTR test vector 3 (100 zeros)");
            print_hex("Expected", expected3, 32);
            print_hex("Got     ", encrypted3, 32);
            pass = 0;
        } else {
            TEST_PASS("CTR test vector 3 (100 zeros)");
        }
    }

    /* ========== Functional tests ========== */
    printf("[+] Testing encrypt-decrypt roundtrip\n");

    /* Test: Encrypt then decrypt should recover plaintext */
    {
        uint8_t plaintext[100];
        uint8_t ciphertext[100];
        uint8_t recovered[100];

        random_bytes(plaintext, sizeof(plaintext));

        sm4_ctr_once(plaintext, sizeof(plaintext), ciphertext, key, iv);
        sm4_ctr_once(ciphertext, sizeof(ciphertext), recovered, key, iv);

        if (memcmp(plaintext, recovered, sizeof(plaintext)) != 0) {
            TEST_FAIL("CTR encrypt-decrypt roundtrip");
            pass = 0;
        } else {
            TEST_PASS("CTR encrypt-decrypt roundtrip");
        }
    }

    /* Test: Streaming API consistency */
    {
        uint8_t plaintext[100];
        uint8_t out_oneshot[100];
        uint8_t out_streaming[100];

        random_bytes(plaintext, sizeof(plaintext));

        /* One-shot */
        sm4_ctr_once(plaintext, sizeof(plaintext), out_oneshot, key, iv);

        /* Streaming in chunks */
        SM4_CTR_CTX ctx;
        sm4_ctr_init(&ctx, key, iv);
        sm4_ctr_update(&ctx, plaintext, out_streaming, 33);
        sm4_ctr_update(&ctx, plaintext + 33, out_streaming + 33, 50);
        sm4_ctr_update(&ctx, plaintext + 83, out_streaming + 83, 17);
        sm4_ctr_clean(&ctx);

        if (memcmp(out_oneshot, out_streaming, sizeof(plaintext)) != 0) {
            TEST_FAIL("CTR streaming API consistency");
            pass = 0;
        } else {
            TEST_PASS("CTR streaming API consistency");
        }
    }

    /* Test: Various message lengths */
    {
        int lengths[] = {1, 15, 16, 17, 31, 32, 33, 100, 255, 256, 1000};
        int num_lengths = sizeof(lengths) / sizeof(lengths[0]);
        int all_pass = 1;

        for (int i = 0; i < num_lengths; i++) {
            int len = lengths[i];
            uint8_t *pt = malloc(len);
            uint8_t *ct = malloc(len);
            uint8_t *rt = malloc(len);

            random_bytes(pt, len);

            sm4_ctr_once(pt, len, ct, key, iv);
            sm4_ctr_once(ct, len, rt, key, iv);

            if (memcmp(pt, rt, len) != 0) {
                printf("[FAIL] CTR roundtrip len=%d\n", len);
                all_pass = 0;
            }

            free(pt);
            free(ct);
            free(rt);
        }

        if (all_pass) {
            TEST_PASS("CTR various message lengths");
        } else {
            pass = 0;
        }
    }

    return pass;
}

/* ============================================================
 * SM4 CBC Tests
 * ============================================================ */

static int sm4_cbc_self_check(void)
{
    printf("\n========== SM4 CBC Self Check ==========\n");

    const uint8_t key[16] = {
        0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF,
        0xFE, 0xDC, 0xBA, 0x98, 0x76, 0x54, 0x32, 0x10
    };
    const uint8_t iv[16] = {
        0x12, 0x34, 0x56, 0x78, 0x9A, 0xBC, 0xDE, 0xF0,
        0xF0, 0xDE, 0xBC, 0x9A, 0x78, 0x56, 0x34, 0x12
    };

    uint32_t rk[SM4_KEY_SCHEDULE];
    sm4_key_schedule(key, rk);

    int pass = 1;

    /* ========== OpenSSL generated test vectors ========== */
    printf("[+] Testing with OpenSSL generated test vectors\n");

    /* Test case 1: 16-byte block (single block) */
    {
        uint8_t plaintext1[16] = {
            0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF,
            0xFE, 0xDC, 0xBA, 0x98, 0x76, 0x54, 0x32, 0x10
        };
        uint8_t ciphertext1[16] = {0};
        const uint8_t expected1[16] = {
            0xD4, 0x81, 0x4B, 0x61, 0x1F, 0xCA, 0xF1, 0xD9,
            0xAB, 0x58, 0x98, 0xA7, 0x69, 0x4C, 0xEA, 0xEE
        };

        sm4_cbc_encrypt(rk, iv, plaintext1, ciphertext1, 16);

        if (memcmp(ciphertext1, expected1, 16) != 0) {
            TEST_FAIL("CBC test vector 1 (single block)");
            print_hex("Expected", expected1, 16);
            print_hex("Got     ", ciphertext1, 16);
            pass = 0;
        } else {
            TEST_PASS("CBC test vector 1 (single block)");
        }
    }

    /* Test case 2: 32-byte (two blocks) */
    {
        uint8_t plaintext2[32] = {
            0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF,
            0xFE, 0xDC, 0xBA, 0x98, 0x76, 0x54, 0x32, 0x10,
            0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF,
            0xFE, 0xDC, 0xBA, 0x98, 0x76, 0x54, 0x32, 0x10
        };
        uint8_t ciphertext2[32] = {0};
        const uint8_t expected2[32] = {
            0xD4, 0x81, 0x4B, 0x61, 0x1F, 0xCA, 0xF1, 0xD9,
            0xAB, 0x58, 0x98, 0xA7, 0x69, 0x4C, 0xEA, 0xEE,
            0x0E, 0x83, 0x4B, 0x61, 0x07, 0x8A, 0xE1, 0x8A,
            0x5E, 0x02, 0xCB, 0x05, 0x05, 0xA3, 0x2C, 0xB9
        };

        sm4_cbc_encrypt(rk, iv, plaintext2, ciphertext2, 32);

        if (memcmp(ciphertext2, expected2, 32) != 0) {
            TEST_FAIL("CBC test vector 2 (two blocks)");
            print_hex("Expected", expected2, 32);
            print_hex("Got     ", ciphertext2, 32);
            pass = 0;
        } else {
            TEST_PASS("CBC test vector 2 (two blocks)");
        }
    }

    /* Test case 3: 64 bytes (four blocks) */
    {
        uint8_t plaintext3[64];
        memset(plaintext3, 0xAA, 64);
        uint8_t ciphertext3[64] = {0};
        const uint8_t expected3[64] = {
            0x23, 0x3B, 0x32, 0xDD, 0xC2, 0x40, 0x73, 0x8D,
            0xB0, 0x34, 0x6D, 0x07, 0x08, 0x65, 0x6E, 0x4B,
            0x2D, 0x96, 0x67, 0x20, 0xB0, 0x1D, 0x65, 0xE1,
            0x45, 0x7F, 0x53, 0x1E, 0xD2, 0xEF, 0x39, 0xCF,
            0x3E, 0x4F, 0xA4, 0xD8, 0x8D, 0x8F, 0xD9, 0xEB,
            0x9D, 0x82, 0xFA, 0xC5, 0x9A, 0x17, 0x82, 0x37,
            0x10, 0x21, 0x72, 0x1B, 0x62, 0x28, 0x8D, 0x07,
            0xAA, 0xDD, 0xFF, 0x32, 0x3A, 0xD7, 0x7E, 0x20
        };

        sm4_cbc_encrypt(rk, iv, plaintext3, ciphertext3, 64);

        if (memcmp(ciphertext3, expected3, 64) != 0) {
            TEST_FAIL("CBC test vector 3 (64 bytes)");
            print_hex("Expected", expected3, 32);
            print_hex("Got     ", ciphertext3, 32);
            pass = 0;
        } else {
            TEST_PASS("CBC test vector 3 (64 bytes)");
        }
    }

    /* ========== Functional tests ========== */
    printf("[+] Testing encrypt-decrypt roundtrip\n");

    /* Test: Encrypt then decrypt should recover plaintext */
    {
        uint8_t plaintext[96];
        uint8_t ciphertext[96];
        uint8_t recovered[96];

        random_bytes(plaintext, sizeof(plaintext));

        sm4_cbc_encrypt(rk, iv, plaintext, ciphertext, sizeof(plaintext));
        sm4_cbc_decrypt(rk, iv, ciphertext, recovered, sizeof(ciphertext));

        if (memcmp(plaintext, recovered, sizeof(plaintext)) != 0) {
            TEST_FAIL("CBC encrypt-decrypt roundtrip");
            pass = 0;
        } else {
            TEST_PASS("CBC encrypt-decrypt roundtrip");
        }
    }

    /* Test: Various message lengths (multiples of 16) */
    {
        int lengths[] = {16, 32, 48, 64, 80, 96, 112, 128, 256, 512, 1024};
        int num_lengths = sizeof(lengths) / sizeof(lengths[0]);
        int all_pass = 1;

        for (int i = 0; i < num_lengths; i++) {
            int len = lengths[i];
            uint8_t *pt = malloc(len);
            uint8_t *ct = malloc(len);
            uint8_t *rt = malloc(len);

            random_bytes(pt, len);

            sm4_cbc_encrypt(rk, iv, pt, ct, len);
            sm4_cbc_decrypt(rk, iv, ct, rt, len);

            if (memcmp(pt, rt, len) != 0) {
                printf("[FAIL] CBC roundtrip len=%d\n", len);
                all_pass = 0;
            }

            free(pt);
            free(ct);
            free(rt);
        }

        if (all_pass) {
            TEST_PASS("CBC various message lengths");
        } else {
            pass = 0;
        }
    }

    return pass;
}

#ifdef TEST_WITH_OPENSSL
static int test_sm4_ctr_vs_openssl(void)
{
    printf("\n========== SM4 CTR vs OpenSSL Test ==========\n");

    const uint8_t key[16] = {
        0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF,
        0xFE, 0xDC, 0xBA, 0x98, 0x76, 0x54, 0x32, 0x10
    };

    uint8_t *plaintext = malloc(4096);
    uint8_t *our_result = malloc(4096);
    uint8_t *openssl_result = malloc(4096);
    uint8_t iv[16], iv_copy[16];

    int fail_count = 0;

    srand((unsigned int)time(NULL));

    for (int test = 0; test < NTESTS; test++) {
        /* Random length from 1 to 4096 */
        size_t len = (rand() % 4096) + 1;

        random_bytes(plaintext, len);
        random_bytes(iv, 16);
        memcpy(iv_copy, iv, 16);

        /* Our implementation */
        sm4_ctr_once(plaintext, len, our_result, key, iv);

        /* OpenSSL implementation */
        if (openssl_sm4_ctr(plaintext, len, openssl_result, key, iv_copy) != 0) {
            printf("[ERROR] OpenSSL encryption failed\n");
            fail_count++;
            continue;
        }

        /* Compare */
        if (memcmp(our_result, openssl_result, len) != 0) {
            fail_count++;
            if (fail_count <= 3) {
                printf("[FAIL] Mismatch at test %d, len=%zu\n", test, len);
                print_hex("IV", iv, 16);
                print_hex("Our   ", our_result, len > 32 ? 32 : len);
                print_hex("OpenSSL", openssl_result, len > 32 ? 32 : len);
            }
        }
    }

    free(plaintext);
    free(our_result);
    free(openssl_result);

    if (fail_count == 0) {
        printf("[PASS] All %d tests passed against OpenSSL\n", NTESTS);
        return 1;
    } else {
        printf("[FAIL] %d/%d tests failed against OpenSSL\n", fail_count, NTESTS);
        return 0;
    }
}

static int test_sm4_cbc_vs_openssl(void)
{
    printf("\n========== SM4 CBC vs OpenSSL Test ==========\n");

    const uint8_t key[16] = {
        0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF,
        0xFE, 0xDC, 0xBA, 0x98, 0x76, 0x54, 0x32, 0x10
    };

    uint8_t *plaintext = malloc(4096);
    uint8_t *our_enc_result = malloc(4096);
    uint8_t *our_dec_result = malloc(4096);
    uint8_t *openssl_enc_result = malloc(4096);
    uint8_t *openssl_dec_result = malloc(4096);
    uint8_t iv[16], iv_copy[16];
    uint32_t rk[SM4_KEY_SCHEDULE];

    int fail_count = 0;

    srand((unsigned int)time(NULL));
    sm4_key_schedule(key, rk);

    for (int test = 0; test < NTESTS; test++) {
        /* Random length (multiple of 16) from 16 to 4096 */
        size_t len = ((rand() % 256) + 1) * 16;

        random_bytes(plaintext, len);
        random_bytes(iv, 16);
        memcpy(iv_copy, iv, 16);

        /* Our implementation - encrypt */
        sm4_cbc_encrypt(rk, iv, plaintext, our_enc_result, len);

        /* OpenSSL implementation - encrypt */
        int openssl_enc_len = openssl_sm4_cbc_encrypt(plaintext, len, openssl_enc_result, key, iv_copy);
        if (openssl_enc_len < 0) {
            printf("[ERROR] OpenSSL encryption failed\n");
            fail_count++;
            continue;
        }

        /* Compare encryption results */
        if (memcmp(our_enc_result, openssl_enc_result, len) != 0) {
            fail_count++;
            if (fail_count <= 3) {
                printf("[FAIL] Encryption mismatch at test %d, len=%zu\n", test, len);
                print_hex("IV", iv, 16);
                print_hex("Our   ", our_enc_result, len > 32 ? 32 : len);
                print_hex("OpenSSL", openssl_enc_result, len > 32 ? 32 : len);
            }
            continue;
        }

        /* Test decryption */
        memcpy(iv_copy, iv, 16);
        sm4_cbc_decrypt(rk, iv, our_enc_result, our_dec_result, len);

        int openssl_dec_len = openssl_sm4_cbc_decrypt(our_enc_result, len, openssl_dec_result, key, iv_copy);
        if (openssl_dec_len < 0) {
            printf("[ERROR] OpenSSL decryption failed\n");
            fail_count++;
            continue;
        }

        /* Compare decryption results with original plaintext */
        if (memcmp(our_dec_result, plaintext, len) != 0) {
            fail_count++;
            if (fail_count <= 3) {
                printf("[FAIL] Our decryption failed at test %d, len=%zu\n", test, len);
            }
            continue;
        }

        if (memcmp(openssl_dec_result, plaintext, len) != 0) {
            fail_count++;
            if (fail_count <= 3) {
                printf("[FAIL] OpenSSL decryption failed at test %d, len=%zu\n", test, len);
            }
            continue;
        }
    }

    free(plaintext);
    free(our_enc_result);
    free(our_dec_result);
    free(openssl_enc_result);
    free(openssl_dec_result);

    if (fail_count == 0) {
        printf("[PASS] All %d tests passed against OpenSSL\n", NTESTS);
        return 1;
    } else {
        printf("[FAIL] %d/%d tests failed against OpenSSL\n", fail_count, NTESTS);
        return 0;
    }
}
#endif

/* ============================================================
 * Main
 * ============================================================ */

int main(void)
{
    int all_pass = 1;

    srand((unsigned int)time(NULL));

    printf("============================================\n");
    printf("        SM4 Correctness Test Suite\n");
    printf("============================================\n");

    /* ECB tests */
    if (!test_sm4_ecb()) {
        all_pass = 0;
    }

    /* CTR self check */
    if (!sm4_ctr_self_check()) {
        all_pass = 0;
    }

    /* CBC self check */
    if (!sm4_cbc_self_check()) {
        all_pass = 0;
    }

#ifdef TEST_WITH_OPENSSL
    /* CTR vs OpenSSL */
    if (!test_sm4_ctr_vs_openssl()) {
        all_pass = 0;
    }

    /* CBC vs OpenSSL */
    if (!test_sm4_cbc_vs_openssl()) {
        all_pass = 0;
    }
#else
    printf("\n[INFO] Compile with TEST_WITH_OPENSSL=1 to test against OpenSSL\n");
#endif

    printf("\n============================================\n");
    if (all_pass) {
        printf("        All tests PASSED\n");
    } else {
        printf("        Some tests FAILED\n");
    }
    printf("============================================\n");

    return all_pass ? 0 : 1;
}
