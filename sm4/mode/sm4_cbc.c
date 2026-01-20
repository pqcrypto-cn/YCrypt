#include "../include/sm4_cbc.h"
#include "../include/sm4.h"

static inline void sm4_get_decrypt_key(const uint32_t rk_enc[32], uint32_t rk_dec[32]) {
    for (int i = 0; i < 32; i++) {
        rk_dec[i] = rk_enc[31 - i];
    }
}

static inline void xor_block(uint8_t *dst, const uint8_t *src1, const uint8_t *src2) {
    ((uint64_t*)dst)[0] = ((uint64_t*)src1)[0] ^ ((uint64_t*)src2)[0];
    ((uint64_t*)dst)[1] = ((uint64_t*)src1)[1] ^ ((uint64_t*)src2)[1];
}

void sm4_cbc_encrypt(
    const uint32_t rk[32],
    const uint8_t iv[16],
    const uint8_t *input,
    uint8_t *output,
    size_t length) 
{
    uint8_t block_buf[16];
    const uint8_t *iv_ptr = iv;

    while (length >= SM4_BLOCK_SIZE) {
        // 1. XOR: Input ^ IV (or previous ciphertext)
        xor_block(block_buf, input, iv_ptr);

        // 2. Encrypt: C_i = Enc(P_i ^ C_{i-1})
        sm4_encrypt(rk, block_buf, output);

        // 3. Update pointers
        iv_ptr = output;
        input += SM4_BLOCK_SIZE;
        output += SM4_BLOCK_SIZE;
        length -= SM4_BLOCK_SIZE;
    }
}

void sm4_cbc_decrypt(
    const uint32_t rk[32],
    const uint8_t iv[16],
    const uint8_t *input,
    uint8_t *output,
    size_t length) 
{
    uint32_t rk_dec[32];
    uint8_t temp_buf[1024];
    const uint8_t *current_iv = iv;

    sm4_get_decrypt_key(rk, rk_dec);

    while (length >= SM4_BLOCK_SIZE) {
        sm4_decrypt(rk, input, temp_buf); 
        xor_block(output, temp_buf, current_iv);
        
        current_iv = input;
        input += SM4_BLOCK_SIZE;
        output += SM4_BLOCK_SIZE;
        length -= SM4_BLOCK_SIZE;
    }
}