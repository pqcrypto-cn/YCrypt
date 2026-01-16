/**
 * SM4-CTR mode implementation
 */

#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include "../include/sm4_ctr.h"

/**
 * Increment 128-bit counter (big-endian)
 * Standard CTR mode increments the entire 128-bit block
 */
static void ctr_inc(uint8_t counter[SM4_BLOCK_SIZE])
{
    for (int i = SM4_BLOCK_SIZE - 1; i >= 0; i--) {
        if (++counter[i] != 0) {
            break;
        }
    }
}

/**
 * XOR two buffers: dst = a ^ b
 */
static void xor_block(uint8_t *dst, const uint8_t *a, const uint8_t *b, size_t len)
{
    for (size_t i = 0; i < len; i++) {
        dst[i] = a[i] ^ b[i];
    }
}

/**
 * Initialize CTR context
 */
void sm4_ctr_init(SM4_CTR_CTX *ctx, const uint8_t key[SM4_KEY_SIZE],
                  const uint8_t iv[SM4_BLOCK_SIZE])
{
    sm4_key_schedule(key, ctx->rk);
    memcpy(ctx->counter, iv, SM4_BLOCK_SIZE);
    ctx->buffer_used = SM4_BLOCK_SIZE; /* Force buffer generation on first use */
}

/**
 * Process data in CTR mode (encrypt or decrypt - same operation)
 * Supports streaming: can be called multiple times with partial data
 */
void sm4_ctr_update(SM4_CTR_CTX *ctx, const uint8_t *in, uint8_t *out, size_t len)
{
    size_t i = 0;

    /* Use remaining buffer from previous call */
    while (i < len && ctx->buffer_used < SM4_BLOCK_SIZE) {
        out[i] = in[i] ^ ctx->buffer[ctx->buffer_used];
        i++;
        ctx->buffer_used++;
    }

    /* Process full blocks */
    while (i + SM4_BLOCK_SIZE <= len) {
        sm4_encrypt(ctx->rk, ctx->counter, ctx->buffer);
        ctr_inc(ctx->counter);
        xor_block(out + i, in + i, ctx->buffer, SM4_BLOCK_SIZE);
        i += SM4_BLOCK_SIZE;
    }

    /* Handle remaining bytes */
    if (i < len) {
        sm4_encrypt(ctx->rk, ctx->counter, ctx->buffer);
        ctr_inc(ctx->counter);
        ctx->buffer_used = 0;

        while (i < len) {
            out[i] = in[i] ^ ctx->buffer[ctx->buffer_used];
            i++;
            ctx->buffer_used++;
        }
    }
}

/**
 * Clear sensitive data from context
 */
void sm4_ctr_clean(SM4_CTR_CTX *ctx)
{
    memset(ctx, 0, sizeof(SM4_CTR_CTX));
}

/* ============================================================
 * Convenience functions for one-shot encryption/decryption
 * ============================================================ */

/**
 * One-shot CTR encryption/decryption
 * Simple interface for encrypting/decrypting a complete message
 */
size_t sm4_ctr(uint8_t *input, size_t len, uint8_t *output,
               uint8_t key[SM4_KEY_SIZE], uint8_t iv[SM4_BLOCK_SIZE])
{
    SM4_CTR_CTX ctx;

    sm4_ctr_init(&ctx, key, iv);
    sm4_ctr_update(&ctx, input, output, len);
    sm4_ctr_clean(&ctx);

    return len;
}

/**
 * One-shot CTR with const-correct interface
 */
size_t sm4_ctr_once(const uint8_t *input, size_t len, uint8_t *output,
                    const uint8_t key[SM4_KEY_SIZE],
                    const uint8_t iv[SM4_BLOCK_SIZE])
{
    SM4_CTR_CTX ctx;

    sm4_ctr_init(&ctx, key, iv);
    sm4_ctr_update(&ctx, input, output, len);
    sm4_ctr_clean(&ctx);

    return len;
}
