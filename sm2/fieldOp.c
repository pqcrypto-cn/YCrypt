#include "include/fieldOp.h"
#include "include/utils.h"
// const static size_t BITS = 256;

void get_random_u32_in_mod_n(u32* k)
{
	u32_rand(k);
	mod_n(k, k);
}
void get_random_u32_in_mod_p(u32* k)
{
	u32_rand(k);
	mod_p(k, k);
}

// because 2*P>2**256, 2*N>2**256, we can use 'if' to replace 'while'
void mod(u32* input, const u32* m, const u32* rhoM)
{
	if (u32_ge(input, m))
	{
		u32_add(input, rhoM, input);
	}
}

void double_mod(const u32* x, u32* result, const u32* m, const u32* rhoM)
{
	*result = *x;
	if (u32_shl(result)) // if it is overflowed
		u32_add(rhoM, result, result);

	if (u32_ge(result, m))
		u32_add(rhoM, result, result);
}

//before add_mod, x and y must in field M.
void add_mod(const u32* x, const u32* y, u32* result, const u32* m, const u32* rhoM)
{
	if (u32_add(x, y, result)) // if it is overflowed
	{
		u32_add(rhoM, result, result); 		// add_mod_n(rhoN, result, result);
	}

	if (u32_ge(result, m))
	{
		u32_add(rhoM, result, result);
	}
}


void sub_mod(const u32* x, const u32* y, u32* result, const u32* m, const u32* rhoM)
{
	u32 inversion_y;
	inv_for_add(y, &inversion_y, m);

	add_mod(x, &inversion_y, result, m, rhoM);
}


void mul_mod_n(const u32* x, const u32* y, u32* result)
{
	if (u32_eq_one(x))
	{
		*result = *y;
		return;
	}
	if (u32_eq_one(y))
	{
		*result = *x;
		return;
	}

	sm2n_mong_mul(x, y, result);
	sm2n_mong_mul(result, &SM2_NH, result);
}

//#define MONG
void pow_mod_p(const u32* x, u32* result)
{
#ifdef MONG
	if (u32_eq_one(x))
	{
		*result = *x;
		return;
	}

	// Non-standard use of montgomery multiplication
	montg_sqr_mod_p(x, result);
	montg_mul_mod_p(result, &SM2_H, result);
#else
	u8 res[8];
	if (u32_eq_one(x))
	{
		*result = *x;
		return;
	}
	raw_pow(x->v, res);
	solinas_reduce(res, result);
#endif
}

void mul_mod_p(const u32* x, const u32* y, u32* result)
{
#ifdef MONG
	if (u32_eq_one(x))
	{
		*result = *y;
		return;
	}
	if (u32_eq_one(y))
	{
		*result = *x;
		return;
	}

	// Non-standard use of montgomery multiplication
	montg_mul_mod_p(x->v, y->v, result->v);
	montg_mul_mod_p(result->v, SM2_H.v, result->v);
#else
	u8 res[8];
	if (u32_eq_one(x))
	{
		*result = *y;
		return;
	}
	if (u32_eq_one(y))
	{
		*result = *x;
		return;
	}
	raw_mul(x->v, y->v, res);
	solinas_reduce(res, result);
#endif
}

void div_mod_p(const u32* x, const u32* y, u32* result)
{
	u32 inversion_y;
	inv_for_mul_mod_p(y, &inversion_y);
	mul_mod_p(x, &inversion_y, result);
}

void inv_for_add(const u32* input, u32* result, const u32* m)
{
	u32_sub(m, input, result);
}

static inline bool transform(u32* x, u32* y, const u32* m, bool carry)
{
	while (x->v[0] % 2 == 0)
	{
		u32_shr(x);

		if (y->v[0] % 2 == 0)
		{
			u32_shr(y);
		}
		else
		{
			carry = u32_add(y, m, y);
			u32_shr(y);
			if (carry)
			{
				y->v[3] |= 0x8000000000000000;
			}
		}
	}
	return carry;
}


void inv_for_mul(const u32* input, u32* result, const u32* m, const u32* rhoM)
{
	if (u32_eq_zero(input))
	{
		*result = *input;
		return;
	}

	u32 u = *input;
	u32 v = *m;
	u32 x1 = { 1, 0, 0, 0 };
	u32 x2 = { 0, 0, 0, 0 };


	bool overflowFlag = false;

	while ((!u32_eq_one(&u)) && (!u32_eq_one(&v)))
	{
		overflowFlag = transform(&u, &x1, m, overflowFlag);
		overflowFlag = transform(&v, &x2, m, overflowFlag);

		if (u32_ge(&u, &v))
		{
			sub_mod(&u, &v, &u, m, rhoM);
			sub_mod(&x1, &x2, &x1, m, rhoM);
		}
		else
		{
			sub_mod(&v, &u, &v, m, rhoM);
			sub_mod(&x2, &x1, &x2, m, rhoM);
		}
	}

	if (u32_eq_one(&u))
	{
		mod(&x1, m, rhoM);
		*result = x1;
	}
	else
	{
		mod(&x2, m, rhoM);
		*result = x2;
	}

}

// This two functions are declared at file ecc.h
extern void montg_inv_mod_p_ex(const UINT64 a[4], UINT64 r[4]);
extern void montg_inv_mod_n_ex(const UINT64 a[4], UINT64 r[4]);
// Get inversion of mod p by montgomery method that is more efficent
void inv_for_mul_mod_p(const u32* input, u32* result)
{
	montg_inv_mod_p_ex(input->v, result->v);
	montg_back_mod_p(result, result);
}

// Get inversion of mod n by montgomery method that is more efficent
void inv_for_mul_mod_n(const u32* input, u32* result)
{
	const u32 ONE = { 1,0, 0, 0 };
	montg_inv_mod_n_ex(input->v, result->v);
	sm2n_mong_mul(result, &ONE,result);
}

#define JIA
void solinas_reduce(const u8 input[8], u32* result)
{
#ifdef JIA
	// fast reduction 512bit to 256bit
	// ref: http://cacr.uwaterloo.ca/techreports/1999/corr99-39.pdf 
	u1 carry;
	u32 sumD;
	u4* A = (u4*)input;
	u8 acc = (u8)A[14] + A[15];
	u8 b1tmp = acc; // acc = A[14] + A[15]
	sumD.v[3] = acc + A[11];
	acc += A[13];
	u8 b0tmp = acc;
	u8 b5tmp = acc + A[10] + A[15];
	acc += A[12]; // acc = A[14] + A[15] + A[13] + A[12]
	sumD.v[2] = acc + A[9] + A[14];
	u8 b7tmp = acc + A[15];
	acc += A[11]; // acc = A[14] + A[15] + A[13] + A[12] + A[11]
	u8 b3tmp = acc + A[8] + A[13];
	u8 minus = (u8)A[8] + A[9] + A[13] + A[14];
	carry = portable_addcarryx_u64(0, sumD.v[2], (b3tmp >> 32) | (b5tmp << 32), &sumD.v[2]);
	portable_addcarryx_u64(carry, sumD.v[3], b5tmp >> 32, &sumD.v[3]); /// may be overflow?
	acc += (u8)A[9] + A[10]; // acc = A[14] + A[15] + A[13] + A[12] + A[11] + A[9] + A[10]
	b1tmp += acc;
	acc += A[8]; // acc = A[14] + A[15] + A[13] + A[12] + A[11] + A[9] + A[10] +  A[8]
	b0tmp += acc;
	b7tmp += acc;
	carry = portable_addcarryx_u64(0, b0tmp, b1tmp << 32, &sumD.v[0]);
	portable_addcarryx_u64(carry, b1tmp >> 32, b3tmp << 32, &sumD.v[1]);
	carry = portable_addcarryx_u64(0, sumD.v[3], b7tmp << 32, &sumD.v[3]);
	portable_addcarryx_u64(carry, b7tmp >> 32, 0, &acc);
	
	// u8 B[8] = {
	// 	0, 0, A[15],  // B[2] is temp
	// 	(u8)A[8] + A[13], (u8)A[9] + A[14], (u8)A[10] + A[15], A[11], A[15]
	// };
	// B[2] += A[14]; B[1] += B[2]; B[6] += B[2];
	// B[2] += A[13]; B[0] += B[2]; B[5] += B[2];
	// B[2] += A[12]; B[4] += B[2]; B[7] += B[2];
	// B[2] += A[11]; B[3] += B[2];
	// B[2] += (u8)A[9] + A[10]; B[1] += B[2];
	// B[2] += A[8]; B[0] += B[2]; B[7] += B[2];

	u32 S = { input[0], input[1], input[2], input[3] };
	carry = u32_add(&S, &sumD, &S);
	portable_addcarryx_u64(carry, acc, 0, &acc);

	u8 tmp = acc << 32;
	u32 payload = {
		acc,        // acc * SM2_rhoP.v[0]
		tmp - acc,  // acc * SM2_rhoP.v[1],
		0,          // acc * SM2_rhoP.v[2],
		tmp,        // acc * SM2_rhoP.v[3]
	};
	carry = u32_add(&payload, &S, &S);
	if (carry) {
		u32_add(&SM2_rhoP, &S, &S);
	}

	// inverse add one
	result->v[0] = S.v[0];
	carry = portable_addcarryx_u64(1, S.v[1], ~minus, &result->v[1]);
	carry = portable_addcarryx_u64(carry, S.v[2], 0xFFFFFFFFFFFFFFFF, &result->v[2]);
	carry = portable_addcarryx_u64(carry, S.v[3], 0xFFFFFFFFFFFFFFFF, &result->v[3]);
	if (!carry) {
		u32_sub(result, &SM2_rhoP, result);
	}
	if (u32_ge(result, &SM2_P)) {
		u32_add(result, &SM2_rhoP, result);
	}

#else
	// fast reduction 256bit to 128bit
	// ref: http://cacr.uwaterloo.ca/techreports/1999/corr99-39.pdf 

	u4 * A = (u4 *)(input);
	u8 B[8] = {
		0, 0, A[15],  // B[2] is temp
		(u8)A[8] + A[13], (u8)A[9] + A[14], (u8)A[10] + A[15], A[11], A[15]
	};
	B[2] += A[14]; B[1] += B[2]; B[6] += B[2];
	B[2] += A[13]; B[0] += B[2]; B[5] += B[2];
	B[2] += A[12]; B[4] += B[2]; B[7] += B[2];
	B[2] += A[11]; B[3] += B[2];
	B[2] += (u8)A[9] + A[10]; B[1] += B[2];
	B[2] += A[8]; B[0] += B[2]; B[7] += B[2];

	u32 sumD;
	u4* ptr = (u4*)sumD.v;
	ptr[0] = (u4)B[0];
	B[1] += B[0] >> 32;
	ptr[1] = (u4)B[1];
	ptr[2] = (u4)(B[1] >> 32);
	ptr[3] = (u4)B[3];
	B[4] += B[3] >> 32;
	ptr[4] = (u4)B[4];
	B[5] += B[4] >> 32;
	ptr[5] = (u4)B[5];
	B[6] += B[5] >> 32;
	ptr[6] = (u4)B[6];
	B[7] += B[6] >> 32;
	ptr[7] = (u4)B[7];
	u1 carry = (u1)(B[7] >> 32);

	u32 S = { input[0], input[1], input[2], input[3] };
	carry += u32_add(&S, &sumD, &S);
	u32 payload = { carry*SM2_rhoP.v[0], carry*SM2_rhoP.v[1], carry*SM2_rhoP.v[2], carry*SM2_rhoP.v[3] };
	carry = u32_add(&payload, &S, &S);
	while (carry) {
		carry = u32_add(&SM2_rhoP, &S, &S);
	}

	result.v[0] = result.v[2] = result.v[3] = 0;
	result.v[1] = (u8)A[8] + A[9] + A[13] + A[14];
	sub_mod_p(S, result, result);

#endif // JIA

}

// a in the residue domain
// result = a^(-1) * 2^(256) in montgomery domain
void MontgInvModp(const u32 *a, u32 *result)
{
	u32 u, v, r, s, t;
	UINT64 k = 0;
	UINT64 i = 0;
	uint8_t carry = 0;


	// Phase I  -> Get r and k
	memcpy(&u, SM2_P.v, 32);
	memcpy(&v, a, 32);
	memset(&r, 0, 32);
	memset(&s, 0, 32); s.v[0] = 1;   // s = 1

	while (!u32_eq_zero(&v))       // v > 0
	{
		if ((u.v[0] & 1) == 0)      // u is even
		{
			u32_shr(&u);           // u = u / 2
			u32_shl(&s);           // s = 2s
		}
		else if ((v.v[0] & 1) == 0) // v is even
		{
			u32_shr(&v);           // v = v / 2
			u32_shl(&r);           // r = 2r
		}
		else if(u32_gr(&u, &v))     // u > v
		{
			u32_sub(&u, &v, &u);     // u = u - v
			u32_shr(&u);           // u = u / 2

			u32_add(&r, &s, &r);     // r = r + s
			u32_shl(&s);           // s = 2s
		}
		else// if (u32_ge(v, u))  // v >= u
		{
			u32_sub(&v, &u, &v);     // v = v - u
			u32_shr(&v);           // v = v / 2

			u32_add(&r, &s, &s);     // s = r + s
			carry += u32_shl(&r);  // r = 2r, r may by overflow at the last time
		}

		k++;                      // k = k + 1
	}

	if (carry > 0)                // This step is important
	{
		u32_sub(&r, &SM2_P, &r);
	}
	if (u32_ge(&r, &SM2_P))
	{
		u32_sub(&r, &SM2_P, &r);
	}
	u32_sub(&SM2_P, &r, &r);       // r = P - r
	 
	// r = a^(-1) * 2^(k)         //  256 <= k <= 512
	
	// Phase II  -> 
	if (k != 256)
	{
		memset(&t, 0, 32);
		k = 2 * 256 - k;          // k = 2m - k
		i = k / 64;
		k = k % 64;
		t.v[i] = ((UINT64)1) << k;      // t = 2^(2*m-k)

		// result = r * t * 2^(-256)
		montg_mul_mod_p(&r, &t, result);
	}
	else
	{
		// No need to do more
		memcpy(result, &r, 32);
	}
	// result = a^(-1) * 2^(256)
}

// a in the residue domain
// result = a^(-1) * 2^(256) in montgomery domain
void MontgInvModn(const u32 *a, u32 *result)
{
	u32 u, v, r, s, t;
	UINT64 k = 0;
	UINT64 i = 0;
	uint8_t carry = 0;

	// Phase I  -> Get r and k
	memcpy(&u, SM2_N.v, 32);
	memcpy(&v, a, 32);
	memset(&r, 0, 32);
	memset(&s, 0, 32); s.v[0] = 1; // s = 1

	while (!u32_eq_zero(&v))  // v > 0
	{
		if ((u.v[0] & 1) == 0)    // u is even
		{
			u32_shr(&u);     // u = u / 2
			u32_shl(&s);     // s = 2s
		}
		else if ((v.v[0] & 1) == 0)  // v is even
		{
			u32_shr(&v);     // v = v / 2
			u32_shl(&r);     // r = 2r
		}
		else if (u32_gr(&u, &v))  // u > v
		{
			u32_sub(&u, &v, &u);  // u = u - v
			u32_shr(&u);        // u = u / 2

			u32_add(&r, &s, &r);  // r = r + s
			u32_shl(&s);        // s = 2s
		}
		else// if (u32_ge(v, u))  // v >= u
		{
			u32_sub(&v, &u, &v);  // v = v - u
			u32_shr(&v);        // v = v / 2

			u32_add(&r, &s, &s);  // s = r + s
			carry += u32_shl(&r);        // r = 2r, r may by overflow at the last time
		}
		k++;                   // k = k + 1
	}

	if (carry > 0)  // This step is important
	{
		u32_sub(&r, &SM2_N, &r);
	}
	if (u32_ge(&r, &SM2_N))
	{
		u32_sub(&r, &SM2_N, &r);
	}
	u32_sub(&SM2_N, &r, &r);   // r = N - r

	// r = a^(-1) * 2^(k)    //  256 <= k <= 512

	// Phase II  -> 
	memset(&t, 0, 32);
	k = 2 * 256 - k;  // k = 2m - k
	i = k / 64;
	k = k % 64;
	t.v[i] = ((UINT64)1) << k;  // t = 2^(2*m-k)

	// result = r * t * 2^(-256)
	sm2n_mong_mul(&r, &t, result);
	// result = a^(-1) * 2^(256)
}

// ============================================================================
// Portable C implementations of field operations (originally in fieldOpas.s)
// ============================================================================

// result = a + b mod P
void add_mod_p(const u32 *a, const u32 *b, u32 *result)
{
	u1 carry = 0;
	u32 tmp;
	// a + b
	carry = portable_addcarryx_u64(carry, a->v[0], b->v[0], &tmp.v[0]);
	carry = portable_addcarryx_u64(carry, a->v[1], b->v[1], &tmp.v[1]);
	carry = portable_addcarryx_u64(carry, a->v[2], b->v[2], &tmp.v[2]);
	carry = portable_addcarryx_u64(carry, a->v[3], b->v[3], &tmp.v[3]);

	// tmp - P (using two's complement: tmp + (~P + 1) = tmp - P)
	u1 borrow = 1;
	borrow = portable_addcarryx_u64(borrow, tmp.v[0], ~SM2_P.v[0], &result->v[0]);
	borrow = portable_addcarryx_u64(borrow, tmp.v[1], ~SM2_P.v[1], &result->v[1]);
	borrow = portable_addcarryx_u64(borrow, tmp.v[2], ~SM2_P.v[2], &result->v[2]);
	borrow = portable_addcarryx_u64(borrow, tmp.v[3], ~SM2_P.v[3], &result->v[3]);

	// If (a + b) < P, use tmp; otherwise use result (which is tmp - P)
	// borrow == 0 means tmp < P, so we need to use tmp
	if (!(carry | borrow)) {
		result->v[0] = tmp.v[0];
		result->v[1] = tmp.v[1];
		result->v[2] = tmp.v[2];
		result->v[3] = tmp.v[3];
	}
}

// result = a + b mod N
void add_mod_n(const u32 *a, const u32 *b, u32 *result)
{
	u1 carry = 0;
	u32 tmp;
	// a + b
	carry = portable_addcarryx_u64(carry, a->v[0], b->v[0], &tmp.v[0]);
	carry = portable_addcarryx_u64(carry, a->v[1], b->v[1], &tmp.v[1]);
	carry = portable_addcarryx_u64(carry, a->v[2], b->v[2], &tmp.v[2]);
	carry = portable_addcarryx_u64(carry, a->v[3], b->v[3], &tmp.v[3]);

	// tmp - N
	u1 borrow = 1;
	borrow = portable_addcarryx_u64(borrow, tmp.v[0], ~SM2_N.v[0], &result->v[0]);
	borrow = portable_addcarryx_u64(borrow, tmp.v[1], ~SM2_N.v[1], &result->v[1]);
	borrow = portable_addcarryx_u64(borrow, tmp.v[2], ~SM2_N.v[2], &result->v[2]);
	borrow = portable_addcarryx_u64(borrow, tmp.v[3], ~SM2_N.v[3], &result->v[3]);

	if (!(carry | borrow)) {
		result->v[0] = tmp.v[0];
		result->v[1] = tmp.v[1];
		result->v[2] = tmp.v[2];
		result->v[3] = tmp.v[3];
	}
}

// result = a - b mod P
void sub_mod_p(const u32 *a, const u32 *b, u32 *result)
{
	u1 carry = 1;
	// a - b (using two's complement)
	carry = portable_addcarryx_u64(carry, a->v[0], ~b->v[0], &result->v[0]);
	carry = portable_addcarryx_u64(carry, a->v[1], ~b->v[1], &result->v[1]);
	carry = portable_addcarryx_u64(carry, a->v[2], ~b->v[2], &result->v[2]);
	carry = portable_addcarryx_u64(carry, a->v[3], ~b->v[3], &result->v[3]);

	// If a - b < 0 (no carry), add P
	if (!carry) {
		carry = 0;
		carry = portable_addcarryx_u64(carry, result->v[0], SM2_P.v[0], &result->v[0]);
		carry = portable_addcarryx_u64(carry, result->v[1], SM2_P.v[1], &result->v[1]);
		carry = portable_addcarryx_u64(carry, result->v[2], SM2_P.v[2], &result->v[2]);
		carry = portable_addcarryx_u64(carry, result->v[3], SM2_P.v[3], &result->v[3]);
	}
}

// result = a - b mod N
void sub_mod_n(const u32 *a, const u32 *b, u32 *result)
{
	u1 carry = 1;
	// a - b
	carry = portable_addcarryx_u64(carry, a->v[0], ~b->v[0], &result->v[0]);
	carry = portable_addcarryx_u64(carry, a->v[1], ~b->v[1], &result->v[1]);
	carry = portable_addcarryx_u64(carry, a->v[2], ~b->v[2], &result->v[2]);
	carry = portable_addcarryx_u64(carry, a->v[3], ~b->v[3], &result->v[3]);

	// If a - b < 0, add N
	if (!carry) {
		carry = 0;
		carry = portable_addcarryx_u64(carry, result->v[0], SM2_N.v[0], &result->v[0]);
		carry = portable_addcarryx_u64(carry, result->v[1], SM2_N.v[1], &result->v[1]);
		carry = portable_addcarryx_u64(carry, result->v[2], SM2_N.v[2], &result->v[2]);
		carry = portable_addcarryx_u64(carry, result->v[3], SM2_N.v[3], &result->v[3]);
	}
}

// result = -a mod P = P - a
void neg_mod_p(const u32 *a, u32 *result)
{
	// Check if a == 0
	if (u32_eq_zero(a)) {
		result->v[0] = result->v[1] = result->v[2] = result->v[3] = 0;
		return;
	}
	u32_sub(&SM2_P, a, result);
}

// result = -a mod N = N - a
void neg_mod_n(const u32 *a, u32 *result)
{
	if (u32_eq_zero(a)) {
		result->v[0] = result->v[1] = result->v[2] = result->v[3] = 0;
		return;
	}
	u32_sub(&SM2_N, a, result);
}

// result = a mod P (simple reduction if a >= P)
void mod_p(const u32 *a, u32 *result)
{
	if (result != a) {
		*result = *a;
	}
	if (u32_ge(result, &SM2_P)) {
		u32_add(result, &SM2_rhoP, result);
	}
}

// result = a mod N
void mod_n(const u32 *a, u32 *result)
{
	if (result != a) {
		*result = *a;
	}
	if (u32_ge(result, &SM2_N)) {
		u32_add(result, &SM2_rhoN, result);
	}
}

// result = 2*a mod P
void double_mod_p(const u32 *a, u32 *result)
{
	u32 tmp;
	// Left shift (multiply by 2) - capture the overflow carry
	u1 overflow = a->v[3] >> 63;  // High bit that will be shifted out
	tmp.v[3] = (a->v[3] << 1) | (a->v[2] >> 63);
	tmp.v[2] = (a->v[2] << 1) | (a->v[1] >> 63);
	tmp.v[1] = (a->v[1] << 1) | (a->v[0] >> 63);
	tmp.v[0] = a->v[0] << 1;

	// tmp - P (add ~P + 1, i.e., subtract P)
	u1 borrow = 1;
	borrow = portable_addcarryx_u64(borrow, tmp.v[0], ~SM2_P.v[0], &result->v[0]);
	borrow = portable_addcarryx_u64(borrow, tmp.v[1], ~SM2_P.v[1], &result->v[1]);
	borrow = portable_addcarryx_u64(borrow, tmp.v[2], ~SM2_P.v[2], &result->v[2]);
	borrow = portable_addcarryx_u64(borrow, tmp.v[3], ~SM2_P.v[3], &result->v[3]);

	// If overflow, the subtraction result is always correct
	// If no overflow and no borrow (tmp < P), use tmp instead
	if (!overflow && !borrow) {
		result->v[0] = tmp.v[0];
		result->v[1] = tmp.v[1];
		result->v[2] = tmp.v[2];
		result->v[3] = tmp.v[3];
	}
}

// result = 2*a mod N
void double_mod_n(const u32 *a, u32 *result)
{
	u32 tmp;
	// Left shift (multiply by 2) - capture the overflow carry
	u1 overflow = a->v[3] >> 63;
	tmp.v[3] = (a->v[3] << 1) | (a->v[2] >> 63);
	tmp.v[2] = (a->v[2] << 1) | (a->v[1] >> 63);
	tmp.v[1] = (a->v[1] << 1) | (a->v[0] >> 63);
	tmp.v[0] = a->v[0] << 1;

	u1 borrow = 1;
	borrow = portable_addcarryx_u64(borrow, tmp.v[0], ~SM2_N.v[0], &result->v[0]);
	borrow = portable_addcarryx_u64(borrow, tmp.v[1], ~SM2_N.v[1], &result->v[1]);
	borrow = portable_addcarryx_u64(borrow, tmp.v[2], ~SM2_N.v[2], &result->v[2]);
	borrow = portable_addcarryx_u64(borrow, tmp.v[3], ~SM2_N.v[3], &result->v[3]);

	// If overflow, the subtraction result is always correct
	// If no overflow and no borrow (tmp < N), use tmp instead
	if (!overflow && !borrow) {
		result->v[0] = tmp.v[0];
		result->v[1] = tmp.v[1];
		result->v[2] = tmp.v[2];
		result->v[3] = tmp.v[3];
	}
}

// result = 2*a mod P (alias for double_mod_p)
void mul_by_2_mod_p(const u32 *a, u32 *result)
{
	double_mod_p(a, result);
}

// result = 3*a mod P
void mul_by_3_mod_p(const u32 *a, u32 *result)
{
	u32 tmp_a, tmp_2a;
	// Copy a in case result == a
	tmp_a = *a;

	// tmp_2a = 2*a mod P
	double_mod_p(&tmp_a, &tmp_2a);

	// result = tmp_2a + a mod P
	add_mod_p(&tmp_2a, &tmp_a, result);
}

// result = a / 2 mod P
void div_by_2_mod_p(const u32 *a, u32 *result)
{
	u32 tmp;
	// Copy a in case result == a
	tmp = *a;

	// If a is odd, add P first
	if (tmp.v[0] & 1) {
		u1 carry = 0;
		carry = portable_addcarryx_u64(carry, tmp.v[0], SM2_P.v[0], &tmp.v[0]);
		carry = portable_addcarryx_u64(carry, tmp.v[1], SM2_P.v[1], &tmp.v[1]);
		carry = portable_addcarryx_u64(carry, tmp.v[2], SM2_P.v[2], &tmp.v[2]);
		carry = portable_addcarryx_u64(carry, tmp.v[3], SM2_P.v[3], &tmp.v[3]);
		// Note: carry here would go to bit 256, but we shift right so it becomes bit 255
		// Since P is odd and a < P, (a + P) fits in 257 bits, right shift gives 256 bits
		if (carry) {
			// The carry bit becomes the MSB after right shift
			result->v[0] = (tmp.v[0] >> 1) | (tmp.v[1] << 63);
			result->v[1] = (tmp.v[1] >> 1) | (tmp.v[2] << 63);
			result->v[2] = (tmp.v[2] >> 1) | (tmp.v[3] << 63);
			result->v[3] = (tmp.v[3] >> 1) | ((u8)1 << 63);
			return;
		}
	}

	// Right shift by 1
	result->v[0] = (tmp.v[0] >> 1) | (tmp.v[1] << 63);
	result->v[1] = (tmp.v[1] >> 1) | (tmp.v[2] << 63);
	result->v[2] = (tmp.v[2] >> 1) | (tmp.v[3] << 63);
	result->v[3] = tmp.v[3] >> 1;
}

// Montgomery multiplication: result = a * b * R^(-1) mod P
// where R = 2^256
void montg_mul_mod_p(const u32 *a, const u32 *b, u32 *result)
{
	sm2p_mong_mul(a, b, result);
}

// Montgomery squaring: result = a^2 * R^(-1) mod P
void montg_sqr_mod_p(const u32 *a, u32 *result)
{
	sm2p_mong_pow(a, result);
}

// Convert to Montgomery domain: result = a * R mod P
// where R = 2^256, so we multiply by R^2 * R^(-1) = R
void montg_to_mod_p(const u32 *a, u32 *result)
{
	// result = a * H mod P, where H = R^2 mod P = 2^512 mod P
	sm2p_mong_mul(a, &SM2_H, result);
}

// Convert from Montgomery domain: result = a * R^(-1) mod P
void montg_back_mod_p(const u32 *a, u32 *result)
{
	// result = a * 1 * R^(-1) mod P
	const u32 ONE = { { 1, 0, 0, 0 } };
	sm2p_mong_mul(a, &ONE, result);
}
