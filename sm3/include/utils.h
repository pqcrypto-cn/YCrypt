#ifndef UTILS_H
#define UTILS_H

#include <stdio.h>
#include <stdint.h>

int hex_to_bytes(const char *in, size_t inlen, uint8_t *out, size_t *outlen);

#endif