#ifndef RANDOMBYTES_H
#define RANDOMBYTES_H

#include <stdint.h>
#include <stdlib.h>

/*
 * Cryptographically secure random number generator
 * Uses platform-specific secure random sources:
 * - Windows: CryptGenRandom
 * - Linux: getrandom() syscall
 * - Other Unix: /dev/urandom
 */
void randombytes(uint8_t *out, size_t outlen);

#endif // RANDOMBYTES_H
