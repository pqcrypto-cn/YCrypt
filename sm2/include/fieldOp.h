#ifndef FIELDOP_H
#define FIELDOP_H

#include "basicOp.h"
#include "extra.h"

void get_random_u32_in_mod_n(u32* k);
void get_random_u32_in_mod_p(u32* k);

//void mod(u32* input, const u32* m);
void mod(u32* input, const u32* m, const u32* rhoM);
void mod_p(const u32* a, u32 *result);
void mod_n(const u32* a, u32 *result);

// void add_mod(const u32* x, const u32* y, u32*result, const u32* m, const u32* rhoM);
// void sub_mod(const u32* x, const u32*y, u32* result, const u32* m, const u32* rhoM);
// void double_mod(const u32* x, u32*result, const u32* m, const u32* rhoM);
void mul_mod_n(const u32* x, const u32* y, u32* result);
void mul_mod_p(const u32* x, const u32* y, u32* result);

// Montgomery multiplication
// extern void montg_to_mod_p(const UINT64 a[4], UINT64 result[4]);
// extern void montg_back_mod_p(const UINT64 a[4], UINT64 result[4]);
// extern void montg_mul_mod_p(const UINT64 a[4], const UINT64 b[4], UINT64 result[4]);
// extern void montg_sqr_mod_p(const UINT64 a[4], UINT64 result[4]);
/// NOTE: u32 *a == a.v, so we define the params here using u32.
void montg_to_mod_p(const u32 *a, u32 *result);
void montg_back_mod_p(const u32 *a, u32 *result);
void montg_mul_mod_p(const u32 *a, const u32 *b, u32 *result);
void montg_sqr_mod_p(const u32 *a, u32 *result);

void pow_mod_p(const u32* x, u32* result);
void div_mod_p(const u32* x, const u32*y, u32* result);

void inv_for_add(const u32* input, u32* result, const u32* rhoM);
void inv_for_mul(const u32* input, u32* result, const u32* m, const u32* rhoM);

// void solinas_reduce_asm(const UINT64 a[8], UINT64 result[4]);
void solinas_reduce(const u8 input[8], u32* result);



// //#define double_mod_n(a, c) double_mod((a), (c), &SM2_N, &SM2_rhoN)
// //#define double_mod_p(a, c) double_mod((a), (c), &SM2_P, &SM2_rhoP)
// void double_mod_p(const UINT64 a[4], UINT64 result[4]);
// void double_mod_n(const UINT64 a[4], UINT64 result[4]);
// void mul_by_2_mod_p(const UINT64 a[4], UINT64 result[4]);
// void mul_by_3_mod_p(const UINT64 a[4], UINT64 result[4]);

// //#define add_mod_n(a, b, c) add_mod((a), (b), (c), &SM2_N, &SM2_rhoN)
// //#define add_mod_p(a, b, c) add_mod((a), (b), (c), &SM2_P, &SM2_rhoP)

// void add_mod_p(const UINT64 a[4], const UINT64 b[4], UINT64 result[4]);
// void add_mod_n(const UINT64 a[4], const UINT64 b[4], UINT64 result[4]);
// void div_by_2_mod_p(const UINT64 a[4], UINT64 result[4]);

// //#define sub_mod_n(a, b, c) sub_mod((a), (b), (c), &SM2_N, &SM2_rhoN)
// //#define sub_mod_p(a, b, c) sub_mod((a), (b), (c), &SM2_P, &SM2_rhoP)
// void sub_mod_p(const UINT64 a[4], const UINT64 b[4], UINT64 result[4]);
// void sub_mod_n(const UINT64 a[4], const UINT64 b[4], UINT64 result[4]);

void double_mod_p(const u32 *a, u32 *result);
void double_mod_n(const u32 *a, u32 *result);
void mul_by_2_mod_p(const u32 *a, u32 *result);
void mul_by_3_mod_p(const u32 *a, u32 *result);

void add_mod_p(const u32 *a, const u32 *b, u32 *result);
void add_mod_n(const u32 *a, const u32 *b, u32 *result);
void div_by_2_mod_p(const u32 *a, u32 *result);

void sub_mod_p(const u32 *a, const u32 *b, u32 *result);
void sub_mod_n(const u32 *a, const u32 *b, u32 *result);

void neg_mod_p(const u32 *a, u32 *result);
void neg_mod_n(const u32 *a, u32 *result);

//#define inv_for_mul_mod_p(a, b)	inv_for_mul((a), (b), &SM2_P, &SM2_rhoP)
// Get inversion of mod p by montgomery method that is more efficent
void inv_for_mul_mod_p(const u32 *input, u32 *result);
//#define inv_for_mul_mod_n(a, b)	inv_for_mul((a), (b), &SM2_N, &SM2_rhoN)
// Get inversion of mod n by montgomery method that is more efficent
void inv_for_mul_mod_n(const u32 *input, u32 *result);

// a in the residue domain
// result = a^(-1) * 2^(256) in montgomery domain
// void MontgInvModp(const UINT64 a[4], UINT64 result[4]);
// void MontgInvModn(const UINT64 a[4], UINT64 result[4]);
void MontgInvModp(const u32 *a, u32 *result);
void MontgInvModn(const u32 *a, u32 *result);


#endif
