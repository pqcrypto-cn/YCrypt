#ifndef ENSUREINTRIN_H
#define ENSUREINTRIN_H
#include "dataType.h"

// Portable C implementations of x64 intrinsics
u1 portable_addcarryx_u64(u1 carry_in, u8 a, u8 b, u8* result);
u8 portable_mulx_u64(u8 a, u8 b, u8* hi_ptr);

// Portable C implementations of raw_mul and raw_pow
void raw_mul(const UINT64 a[4], const UINT64 b[4], UINT64 result[8]);
void raw_pow(const UINT64 a[4], UINT64 result[8]);

#endif // ENSUREINTRIN_H
