#ifndef BASICOP_H
#define BASICOP_H

#include "dataType.h"
#include "ensureintrin.h"
#include "sm2_const.h"

void u32_sub(const u32* a, const u32* b, u32* result);
u1   u32_shl(u32* input);
void u32_shr(u32* input);
void u32_neg(u32* input);

u1 u32_add(const u32* a, const u32* b, u32* result);
// uint8_t u32_add(UINT64 a[4], UINT64 b[4], UINT64 result[4]);
// uint8_t u32_inc(UINT64 a[4], UINT64 b, UINT64 result[4]);
u1 u32_inc(const u32 *a, UINT64 b, u32 *result);

bool u32_ge(const u32* a, const u32* b);
// great or equal
bool u32_gr(const u32* a, const u32* b);
bool u32_eq(const u32* a, const u32* b);
bool u32_eq_zero(const u32* a);
bool u32_eq_one(const u32* a);

// void sm2p_mong_mul(const UINT64* x, const UINT64* y, UINT64* result);
// void sm2p_mong_pow(const UINT64* x, UINT64* result);
// void sm2n_mong_mul(const UINT64* x, const UINT64* y, UINT64* result);

void sm2p_mong_mul(const u32* x, const u32* y, u32* result);
void sm2p_mong_pow(const u32* x, u32* result);
void sm2n_mong_mul(const u32* x, const u32* y, u32* result);

// raw_mul and raw_pow are declared in ensureintrin.h
// (either as extern assembly or as macros to C implementations)

#endif