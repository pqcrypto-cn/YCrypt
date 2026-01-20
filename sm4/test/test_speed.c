/**
 * SM4 Performance Benchmark
 * - ECB mode speed test
 * - CTR mode speed test
 * - Optional comparison with OpenSSL
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "../include/sm4.h"
#include "../include/sm4_ctr.h"
#include "../include/sm4_cbc.h"

/* Benchmark configuration */
#define MIN_BENCH_TIME  2.0    /* Minimum seconds to run each benchmark */
#define DATA_SIZE_MB    64     /* Data size for CTR benchmark (MB) */

/* ============================================================
 * OpenSSL for comparison
 * ============================================================ */

#ifdef TEST_WITH_OPENSSL
#include <openssl/evp.h>

static void openssl_sm4_ctr_bench(const uint8_t *input, size_t len, uint8_t *output,
                                   const uint8_t *key, const uint8_t *iv)
{
    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    EVP_EncryptInit_ex(ctx, EVP_sm4_ctr(), NULL, key, iv);

    int outlen, tmplen;
    EVP_EncryptUpdate(ctx, output, &outlen, input, (int)len);
    EVP_EncryptFinal_ex(ctx, output + outlen, &tmplen);

    EVP_CIPHER_CTX_free(ctx);
}

static void openssl_sm4_cbc_bench(const uint8_t *input, size_t len, uint8_t *output,
                                   const uint8_t *key, const uint8_t *iv)
{
    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    EVP_EncryptInit_ex(ctx, EVP_sm4_cbc(), NULL, key, iv);

    int outlen, tmplen;
    EVP_EncryptUpdate(ctx, output, &outlen, input, (int)len);
    EVP_EncryptFinal_ex(ctx, output + outlen, &tmplen);

    EVP_CIPHER_CTX_free(ctx);
}
#endif

/* ============================================================
 * Utility functions
 * ============================================================ */

static void print_speed(const char *name, double mb_per_sec)
{
    printf("  %-35s %10.2f MB/s\n", name, mb_per_sec);
}

static double get_time_sec(void)
{
    return (double)clock() / CLOCKS_PER_SEC;
}

/* ============================================================
 * ECB Benchmark
 * ============================================================ */

static void bench_sm4_ecb(void)
{
    printf("\n========== SM4 ECB Benchmark ==========\n");

    const uint8_t key[16] = {
        0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF,
        0xFE, 0xDC, 0xBA, 0x98, 0x76, 0x54, 0x32, 0x10
    };

    __attribute__((aligned(64))) uint8_t buf[64];
    uint32_t rk[SM4_KEY_SCHEDULE];

    memset(buf, 0x55, sizeof(buf));
    sm4_key_schedule(key, rk);

    /* Warm up */
    for (int i = 0; i < 1000; i++) {
        sm4_encrypt(rk, buf, buf);
    }

    /* Benchmark: encrypt as many blocks as possible in MIN_BENCH_TIME */
    uint64_t total_bytes = 0;
    uint64_t iterations = 64;
    double start = get_time_sec();
    double elapsed;

    do {
        for (uint64_t i = 0; i < iterations; i++) {
            sm4_encrypt(rk, buf, buf);
            sm4_encrypt(rk, buf + 16, buf + 16);
            sm4_encrypt(rk, buf + 32, buf + 32);
            sm4_encrypt(rk, buf + 48, buf + 48);
        }
        total_bytes += 64 * iterations;
        iterations <<= 1;
        elapsed = get_time_sec() - start;
    } while (elapsed < MIN_BENCH_TIME);

    double mb_per_sec = (total_bytes / 1e6) / elapsed;
    print_speed("sm4_encrypt (ref)", mb_per_sec);
}

/* ============================================================
 * CTR Benchmark
 * ============================================================ */

static void bench_sm4_ctr(void)
{
    printf("\n========== SM4 CTR Benchmark ==========\n");

    const uint8_t key[16] = {
        0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF,
        0xFE, 0xDC, 0xBA, 0x98, 0x76, 0x54, 0x32, 0x10
    };
    const uint8_t iv[16] = {
        0x12, 0x34, 0x56, 0x78, 0x9A, 0xBC, 0xDE, 0xF0,
        0xF0, 0xDE, 0xBC, 0x9A, 0x78, 0x56, 0x34, 0x12
    };

    size_t data_size = DATA_SIZE_MB * 1024 * 1024;
    uint8_t *input = aligned_alloc(64, data_size);
    uint8_t *output = aligned_alloc(64, data_size);

    if (!input || !output) {
        printf("  [ERROR] Failed to allocate memory\n");
        free(input);
        free(output);
        return;
    }

    memset(input, 0xAA, data_size);

    /* Warm up */
    sm4_ctr_once(input, 1024, output, key, iv);

    /* Benchmark our implementation */
    {
        uint64_t total_bytes = 0;
        int iterations = 0;
        double start = get_time_sec();
        double elapsed;

        do {
            sm4_ctr_once(input, data_size, output, key, iv);
            total_bytes += data_size;
            iterations++;
            elapsed = get_time_sec() - start;
        } while (elapsed < MIN_BENCH_TIME);

        double mb_per_sec = (total_bytes / 1e6) / elapsed;
        print_speed("sm4_ctr (ref)", mb_per_sec);
    }

    /* Benchmark streaming API */
    {
        uint64_t total_bytes = 0;
        int iterations = 0;
        double start = get_time_sec();
        double elapsed;

        do {
            SM4_CTR_CTX ctx;
            sm4_ctr_init(&ctx, key, iv);

            /* Process in 64KB chunks */
            size_t chunk_size = 64 * 1024;
            for (size_t offset = 0; offset < data_size; offset += chunk_size) {
                size_t len = (offset + chunk_size <= data_size) ? chunk_size : (data_size - offset);
                sm4_ctr_update(&ctx, input + offset, output + offset, len);
            }
            sm4_ctr_clean(&ctx);

            total_bytes += data_size;
            iterations++;
            elapsed = get_time_sec() - start;
        } while (elapsed < MIN_BENCH_TIME);

        double mb_per_sec = (total_bytes / 1e6) / elapsed;
        print_speed("sm4_ctr streaming (64KB chunks)", mb_per_sec);
    }

#ifdef TEST_WITH_OPENSSL
    /* Benchmark OpenSSL */
    {
        uint64_t total_bytes = 0;
        int iterations = 0;
        double start = get_time_sec();
        double elapsed;

        do {
            openssl_sm4_ctr_bench(input, data_size, output, key, iv);
            total_bytes += data_size;
            iterations++;
            elapsed = get_time_sec() - start;
        } while (elapsed < MIN_BENCH_TIME);

        double mb_per_sec = (total_bytes / 1e6) / elapsed;
        print_speed("OpenSSL sm4_ctr", mb_per_sec);
    }
#endif

    free(input);
    free(output);
}

/* ============================================================
 * CBC Benchmark
 * ============================================================ */

static void bench_sm4_cbc(void)
{
    printf("\n========== SM4 CBC Benchmark ==========\n");

    const uint8_t key[16] = {
        0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF,
        0xFE, 0xDC, 0xBA, 0x98, 0x76, 0x54, 0x32, 0x10
    };
    const uint8_t iv[16] = {
        0x12, 0x34, 0x56, 0x78, 0x9A, 0xBC, 0xDE, 0xF0,
        0xF0, 0xDE, 0xBC, 0x9A, 0x78, 0x56, 0x34, 0x12
    };

    size_t data_size = DATA_SIZE_MB * 1024 * 1024;
    uint8_t *input = aligned_alloc(64, data_size);
    uint8_t *output = aligned_alloc(64, data_size);
    uint32_t rk[SM4_KEY_SCHEDULE];

    if (!input || !output) {
        printf("  [ERROR] Failed to allocate memory\n");
        free(input);
        free(output);
        return;
    }

    memset(input, 0xAA, data_size);
    sm4_key_schedule(key, rk);

    /* Warm up */
    sm4_cbc_encrypt(rk, iv, input, output, 1024);

    /* Benchmark our implementation - encryption */
    {
        uint64_t total_bytes = 0;
        int iterations = 0;
        double start = get_time_sec();
        double elapsed;

        do {
            sm4_cbc_encrypt(rk, iv, input, output, data_size);
            total_bytes += data_size;
            iterations++;
            elapsed = get_time_sec() - start;
        } while (elapsed < MIN_BENCH_TIME);

        double mb_per_sec = (total_bytes / 1e6) / elapsed;
        print_speed("sm4_cbc_encrypt (ref)", mb_per_sec);
    }

    /* Benchmark our implementation - decryption */
    {
        /* First encrypt the data */
        sm4_cbc_encrypt(rk, iv, input, output, data_size);

        uint64_t total_bytes = 0;
        int iterations = 0;
        double start = get_time_sec();
        double elapsed;

        do {
            sm4_cbc_decrypt(rk, iv, output, input, data_size);
            total_bytes += data_size;
            iterations++;
            elapsed = get_time_sec() - start;
        } while (elapsed < MIN_BENCH_TIME);

        double mb_per_sec = (total_bytes / 1e6) / elapsed;
        print_speed("sm4_cbc_decrypt (ref)", mb_per_sec);
    }

#ifdef TEST_WITH_OPENSSL
    /* Benchmark OpenSSL */
    {
        uint64_t total_bytes = 0;
        int iterations = 0;
        double start = get_time_sec();
        double elapsed;

        do {
            openssl_sm4_cbc_bench(input, data_size, output, key, iv);
            total_bytes += data_size;
            iterations++;
            elapsed = get_time_sec() - start;
        } while (elapsed < MIN_BENCH_TIME);

        double mb_per_sec = (total_bytes / 1e6) / elapsed;
        print_speed("OpenSSL sm4_cbc", mb_per_sec);
    }
#endif

    free(input);
    free(output);
}

/* ============================================================
 * Throughput for various data sizes
 * ============================================================ */

static void bench_sm4_ctr_sizes(void)
{
    printf("\n========== SM4 CTR Throughput by Size ==========\n");

    const uint8_t key[16] = {
        0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF,
        0xFE, 0xDC, 0xBA, 0x98, 0x76, 0x54, 0x32, 0x10
    };
    const uint8_t iv[16] = {
        0x12, 0x34, 0x56, 0x78, 0x9A, 0xBC, 0xDE, 0xF0,
        0xF0, 0xDE, 0xBC, 0x9A, 0x78, 0x56, 0x34, 0x12
    };

    /* Test different message sizes */
    size_t sizes[] = {16, 64, 256, 1024, 4096, 16384, 65536, 262144, 1048576};
    int num_sizes = sizeof(sizes) / sizeof(sizes[0]);

    uint8_t *input = aligned_alloc(64, 1048576);
    uint8_t *output = aligned_alloc(64, 1048576);

    if (!input || !output) {
        printf("  [ERROR] Failed to allocate memory\n");
        free(input);
        free(output);
        return;
    }

    memset(input, 0xAA, 1048576);

    printf("  %-15s %15s\n", "Size", "Speed (MB/s)");
    printf("  %-15s %15s\n", "----", "------------");

    for (int i = 0; i < num_sizes; i++) {
        size_t size = sizes[i];
        uint64_t total_bytes = 0;
        double start = get_time_sec();
        double elapsed;

        /* Run for at least 0.5 seconds */
        do {
            sm4_ctr_once(input, size, output, key, iv);
            total_bytes += size;
            elapsed = get_time_sec() - start;
        } while (elapsed < 0.5);

        double mb_per_sec = (total_bytes / 1e6) / elapsed;

        char size_str[32];
        if (size >= 1048576) {
            snprintf(size_str, sizeof(size_str), "%zu MB", size / 1048576);
        } else if (size >= 1024) {
            snprintf(size_str, sizeof(size_str), "%zu KB", size / 1024);
        } else {
            snprintf(size_str, sizeof(size_str), "%zu B", size);
        }

        printf("  %-15s %15.2f\n", size_str, mb_per_sec);
    }

    free(input);
    free(output);
}

/* ============================================================
 * Main
 * ============================================================ */

int main(void)
{
    printf("============================================\n");
    printf("        SM4 Performance Benchmark\n");
    printf("============================================\n");
    printf("  Data size: %d MB\n", DATA_SIZE_MB);
    printf("  Min bench time: %.1f sec\n", MIN_BENCH_TIME);
#ifdef TEST_WITH_OPENSSL
    printf("  OpenSSL comparison: enabled\n");
#else
    printf("  OpenSSL comparison: disabled\n");
    printf("  (compile with TEST_WITH_OPENSSL=1 to enable)\n");
#endif

    bench_sm4_ecb();
    bench_sm4_ctr();
    bench_sm4_cbc();
    bench_sm4_ctr_sizes();

    printf("\n============================================\n");
    printf("        Benchmark Complete\n");
    printf("============================================\n");

    return 0;
}
