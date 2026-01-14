#ifndef ECC_H
#define ECC_H

#include "fieldOp.h"
#include "sm2_const.h"

// Jacobian point zero is encoded as (x, y, 0)
static const JPoint JPoint_ZERO = { { 1, 0, 0, 0 },{ 1, 0, 0, 0 },{ 0, 0, 0, 0 } };

bool equ_to_AFPoint_one(const AFPoint* point);
bool equ_to_JPoint_one(const JPoint* point);
bool equ_to_JPoint(const JPoint* point1, const JPoint* point2);
bool equ_to_AFPoint(const AFPoint* point1, const AFPoint* point2);
bool is_on_curve(const AFPoint* point);
void affine_to_jacobian(const AFPoint* point, JPoint* result);
void jacobian_to_affine(const JPoint* point, AFPoint* result);
void add_JPoint_and_AFPoint(const JPoint* point1, const AFPoint* point2, JPoint* result);;
void add_JPoint(const JPoint* point1, const JPoint* point2, JPoint* result);

/// NOTE:
//      To make a function available for inlining across multiple files, you must:
//      place the function in a common header file, for example foo. h.
//      mark the function as extern __inline.
//      #include the header file in each file where the inline function is needed
// inline void CopyAFPoint(AFPoint* src, AFPoint* dst);
// inline void CopyJPoint(JPoint* src, JPoint* dst);
inline void CopyAFPoint(const AFPoint* src, AFPoint* dst)
{
	memcpy(dst, src, sizeof(AFPoint));
}
inline void CopyJPoint(const JPoint* src, JPoint* dst)
{
	memcpy(dst, src, sizeof(JPoint));
}

// Computing the NAF of a positive integer k
// w = 3
void get_naf_w3(const u32* k, int8_t naf_w3[257]);

// Computing the NAF of a positive integer k
// w = 5
void get_naf_w5_2(const u32* pk, int8_t naf_w5[257]);


// NAF method for w = 3
void times_point_naf_w3(const AFPoint* point, const u32* times, JPoint* result);

// NAF method
void times_point_naf_w5(const AFPoint* point, const u32* times, JPoint* result);

// NAF method for w = 5 (ALL OPERATION IN JACOBIAN COORDINATE)
void times_point_naf_w5_all_jpoint(const AFPoint* point, const u32* times, JPoint* result);

// Deafult: naf_w3 method, because it is more efficent according to our test result.
//#define times_point times_point_naf_w3
#define times_point times_point_naf_w5_all_jpoint
//#define times_point montg_times_point_naf_w5_all_jpoint

//void times_point(const AFPoint* point, const u32* times, JPoint* result);
// Mimic Intel IPP
void times_point2(const JPoint* point, const u32* times, JPoint* result);




// Convert affine point(residue domain) to jacobian point(montgomery domain)
int montg_apoint_to_jpoint(const AFPoint* a, JPoint* r);

// Double jacobian point, either in montgomery domain or in residue domain
void montg_double_jpoint(const JPoint* a, JPoint* r);

// Add jacobian point, either in montgomery domain or in residue domain
int montg_add_jpoint(const JPoint* a, const JPoint* b, JPoint* r);

// Add affine point(montgomery domain) and jacobian point(montgomery domain)
// Input: 
//      a -- in montgomery domain, jacobian point
//      b -- in montgomery domain, affine point
// Output: 
//      r -- in montgomery domain
int montg_add_jpoint_and_apoint(const JPoint* a, const AFPoint* b, JPoint* r);

// Convert jacobian point(montgomery domain) to affine point(residue domain)
int montg_jpoint_to_apoint(const JPoint* a, UINT64* rx, UINT64* ry);

// Convert affine point(residue domain) to affine point(montgomery domain)
// Input: 
//      a -- in residue domain
// Output: 
//      r -- in montgomery domain
int montg_apoint_to_montg(const AFPoint* a, AFPoint* r);

// Convert jacobian point(residue domain) to jacobian point(montgomery domain)
// Input: 
//      a -- in residue domain
// Output: 
//      r -- in montgomery domain
int montg_jpoint_to_montg(const JPoint* a, JPoint* r);

// Scalar multiplication in montgomery domain
// Input: 
//      P            -- in residue domain
//      k            -- in residue domain
// Output: 
//      result = kP  -- in montgomery domain
// Complexity:
//      ~1+257 JPOINT_DBL + 7+43 JPOINT_ADD = 258(4M + 4S) + 50(12M + 4S) = 1632M + 1232S
//      JPOINT_DBL: Jacobian point double
//      JPOINT_ADD: Jacobian point addition
void montg_times_point_naf_w5_all_jpoint(const AFPoint* P, const u32* k, JPoint* result);

// Scalar multiplication in montgomery domain
// Input: 
//      P            -- in residue domain
//      k            -- in residue domain
// Output: 
//      result = kP  -- in montgomery domain
// Complexity:
//      ~257 JPOINT_DBL + 64 MPOINT_ADD + (15M + 8S + 1I) = 
//      257(4M + 4S) + 64(8M + 3S) + (15M + 8S + 1I) = 1555M + 1228S + 1I
//      JPOINT_DBL: Jacobian point double
//      MPOINT_ADD: Mixed jacobian point  and affine point addition
void montg_times_point_naf_w3(const AFPoint* P, const u32* k, JPoint* result);

// Scalar multiplication in montgomery domain for fixed point
// Input: 
//      k            -- in residue domain
// Output: 
//      result = kG  -- in montgomery domain
void montg_times_base_point(const u32* k, JPoint* result);

// Simplest scalar multiplication in montgomery domain for random point
// Input: 
//      P            -- in montgomery domain
//      k            -- in residue domain
// Output: 
//      result = kG  -- in montgomery domain
void montg_naive_times_point(const AFPoint* P, const u32* k, JPoint* result);



void times_basepoint(const u32* times, JPoint* result);

void JPoint_neg(const JPoint* point, JPoint* result);
void AFPoint_neg(const AFPoint* point, AFPoint* result);
void add_AFPoint(const AFPoint* point1, const AFPoint* point2, AFPoint* result);
void double_JPoint(const JPoint* pt, JPoint* result);
//void double_JPoint2(const JPoint* pt, JPoint* result);
#define double_JPoint2 double_JPoint

size_t get_u8_bit(u8 input, size_t i);
size_t to_index(const u32* input, size_t i);
void gen_tables();
void precompute_ptable_for_w5(const AFPoint* point);

// ML-Version of base point multiplication
void ML_mul_basepoint(const u32* k, JPoint* result);

// Temp function
void self_main_for_base_poit_mul_test();
void self_main_test_point_mul();
void self_main_test_montg_op();



///////////////////////////////////////////////////////////////////////////////////
//
// Implementation of the following functions are located at file : ecc_as.asm
//
///////////////////////////////////////////////////////////////////////////////////

// Perform doubling jacobian point in montgomery domain
// Its functionality and complexity are equivalent to function : montg_double_jpoint
void montg_double_jpoint_ex(const JPoint* a, JPoint* r);

// Perform adding jacobian point and affine point in montgomery domain
// Its functionality and complexity are equivalent to function : montg_add_jpoint_and_apoint
void montg_add_jpoint_and_apoint_ex(const JPoint* a, const AFPoint* b, JPoint* r);

// Perform adding two jacobian points in montgomery domain
// Its functionality and complexity are equivalent to function : montg_add_jpoint
void montg_add_jpoint_ex(const JPoint* a, const JPoint* b, JPoint* r);

// Perform inversion in montgomery domain
// Its functionality and complexity are equivalent to function : MontgInvModp
void montg_inv_mod_p_ex(const UINT64 a[4], UINT64 r[4]);

// Perform inversion in montgomery domain
// Its functionality and complexity are equivalent to function : MontgInvModn
void montg_inv_mod_n_ex(const UINT64 a[4], UINT64 r[4]);



#endif