#include "include/basicOp.h"

void u32_neg(u32* input)
{
	u1 carry = 1;
	carry = portable_addcarryx_u64(carry, ~input->v[0], 0, &input->v[0]);
	carry = portable_addcarryx_u64(carry, ~input->v[1], 0, &input->v[1]);
	carry = portable_addcarryx_u64(carry, ~input->v[2], 0, &input->v[2]);
	carry = portable_addcarryx_u64(carry, ~input->v[3], 0, &input->v[3]);
}

u1 u32_add(const u32* a, const u32* b, u32* result)
{
	u1 carry = 0;
	carry = portable_addcarryx_u64(carry, a->v[0], b->v[0], &result->v[0]);
	carry = portable_addcarryx_u64(carry, a->v[1], b->v[1], &result->v[1]);
	carry = portable_addcarryx_u64(carry, a->v[2], b->v[2], &result->v[2]);
	carry = portable_addcarryx_u64(carry, a->v[3], b->v[3], &result->v[3]);
	return carry;
}

u1 u32_inc(const u32 *a, UINT64 b, u32 *result)
{
	u1 carry = 0;
	carry = portable_addcarryx_u64(carry, a->v[0], b, &result->v[0]);
	carry = portable_addcarryx_u64(carry, a->v[1], 0, &result->v[1]);
	carry = portable_addcarryx_u64(carry, a->v[2], 0, &result->v[2]);
	carry = portable_addcarryx_u64(carry, a->v[3], 0, &result->v[3]);
	return carry;
}

void u32_sub(const u32* a, const u32* b, u32* result)
{
	u1 carry = 1;
	carry = portable_addcarryx_u64(carry, a->v[0], ~b->v[0], &result->v[0]);
	carry = portable_addcarryx_u64(carry, a->v[1], ~b->v[1], &result->v[1]);
	carry = portable_addcarryx_u64(carry, a->v[2], ~b->v[2], &result->v[2]);
	carry = portable_addcarryx_u64(carry, a->v[3], ~b->v[3], &result->v[3]);
}

u1 u32_shl(u32* input)
{
	u1 carry = (input->v[3] >> 63);
	input->v[3] = (input->v[3] << 1) | (input->v[2] >> 63);
	input->v[2] = (input->v[2] << 1) | (input->v[1] >> 63);
	input->v[1] = (input->v[1] << 1) | (input->v[0] >> 63);
	input->v[0] <<= 1;
	return carry;
}

void u32_shr(u32* input)
{
	input->v[0] = (input->v[0] >> 1) | (input->v[1] << 63);
	input->v[1] = (input->v[1] >> 1) | (input->v[2] << 63);
	input->v[2] = (input->v[2] >> 1) | (input->v[3] << 63);
	input->v[3] >>= 1;
}

// great or equal
bool u32_ge(const u32* a, const u32* b)
{
	if (a->v[3] != b->v[3])
		return (a->v[3] > b->v[3]);
	if (a->v[2] != b->v[2])
		return (a->v[2] > b->v[2]);
	if (a->v[1] != b->v[1])
		return (a->v[1] > b->v[1]);
	return (a->v[0] >= b->v[0]);
}

// great 
bool u32_gr(const u32* a, const u32* b)
{
	if (a->v[3] != b->v[3])
		return (a->v[3] > b->v[3]);
	if (a->v[2] != b->v[2])
		return (a->v[2] > b->v[2]);
	if (a->v[1] != b->v[1])
		return (a->v[1] > b->v[1]);
	return (a->v[0] > b->v[0]);
}

bool u32_eq(const u32* a, const u32* b)
{
	return !((a->v[0] ^ b->v[0]) | (a->v[1] ^ b->v[1]) | (a->v[2] ^ b->v[2]) | (a->v[3] ^ b->v[3]));
}

bool u32_eq_zero(const u32* a)
{
	return !(a->v[0] | a->v[1] | a->v[2] | a->v[3]);
}

bool u32_eq_one(const u32* a)
{
	return !((a->v[0] ^ 1) | a->v[1] | a->v[2] | a->v[3]);
}

// Standard Montgomery reduction for SM2 P
// For SM2, P[0] = -1 mod 2^64, so -P^(-1) mod 2^64 = 1
// This means factor = T[i] * 1 = T[i] in each iteration
// P = { 0xFFFFFFFFFFFFFFFF, 0xFFFFFFFF00000000, 0xFFFFFFFFFFFFFFFF, 0xFFFFFFFEFFFFFFFF }
// Note: P[0] = P[2] = -1, can use shortcut for multiplication by -1
void sm2p_mong_mul_core(UINT64 interim[9], u32* result)
{
	const static size_t LEN = 9;
	int i = 0;
	for (i = 0; i < 4; i++)
	{
		u1 carry = 0;
		u8 h;
		u8 l;
		size_t pos;

		// factor = interim[i] * (-P^(-1) mod 2^64) = interim[i] * 1 = interim[i]
		u8 factor = interim[i];

		if (factor == 0)
			continue;

		// Special case: factor == 1, just add P directly
		if (factor == 1)
		{
			carry = 0;
			carry = portable_addcarryx_u64(carry, interim[i + 0], SM2_P.v[0], &interim[i + 0]);
			carry = portable_addcarryx_u64(carry, interim[i + 1], SM2_P.v[1], &interim[i + 1]);
			carry = portable_addcarryx_u64(carry, interim[i + 2], SM2_P.v[2], &interim[i + 2]);
			carry = portable_addcarryx_u64(carry, interim[i + 3], SM2_P.v[3], &interim[i + 3]);
			pos = 4 + i;
			while (carry && pos < LEN)
			{
				carry = portable_addcarryx_u64(carry, interim[pos], 0, &interim[pos]);
				++pos;
			}
			continue;
		}

		// Add factor * P[0] at position i
		// P[0] = 0xFFFFFFFFFFFFFFFF = -1
		// factor * (-1) = (factor-1, ~factor+1) [hi, lo]
		carry = 0;
		h = factor - 1;
		l = ~factor + 1;
		carry = portable_addcarryx_u64(carry, interim[i + 0], l, &interim[i + 0]);
		carry = portable_addcarryx_u64(carry, interim[i + 1], h, &interim[i + 1]);
		pos = 2 + i;
		while (carry && pos < LEN)
		{
			carry = portable_addcarryx_u64(carry, interim[pos], 0, &interim[pos]);
			++pos;
		}

		// Add factor * P[1] at position i+1
		// P[1] = 0xFFFFFFFF00000000
		carry = 0;
		l = portable_mulx_u64(factor, SM2_P.v[1], &h);
		carry = portable_addcarryx_u64(carry, interim[i + 1], l, &interim[i + 1]);
		carry = portable_addcarryx_u64(carry, interim[i + 2], h, &interim[i + 2]);
		pos = 3 + i;
		while (carry && pos < LEN)
		{
			carry = portable_addcarryx_u64(carry, interim[pos], 0, &interim[pos]);
			++pos;
		}

		// Add factor * P[2] at position i+2
		// P[2] = 0xFFFFFFFFFFFFFFFF = -1
		// factor * (-1) = (factor-1, ~factor+1) [hi, lo]
		carry = 0;
		h = factor - 1;
		l = ~factor + 1;
		carry = portable_addcarryx_u64(carry, interim[i + 2], l, &interim[i + 2]);
		carry = portable_addcarryx_u64(carry, interim[i + 3], h, &interim[i + 3]);
		pos = 4 + i;
		while (carry && pos < LEN)
		{
			carry = portable_addcarryx_u64(carry, interim[pos], 0, &interim[pos]);
			++pos;
		}

		// Add factor * P[3] at position i+3
		// P[3] = 0xFFFFFFFEFFFFFFFF
		carry = 0;
		l = portable_mulx_u64(factor, SM2_P.v[3], &h);
		carry = portable_addcarryx_u64(carry, interim[i + 3], l, &interim[i + 3]);
		carry = portable_addcarryx_u64(carry, interim[i + 4], h, &interim[i + 4]);
		pos = 5 + i;
		while (carry && pos < LEN)
		{
			carry = portable_addcarryx_u64(carry, interim[pos], 0, &interim[pos]);
			++pos;
		}
	}

	// Result is in interim[4..7], with possible overflow in interim[8]
	memcpy(result, interim + 4, 32);

	// If there was overflow, or if result >= P, subtract P
	if (interim[8] != 0)
		u32_add(result, &SM2_rhoP, result);

	if (u32_ge(result, &SM2_P))
		u32_add(result, &SM2_rhoP, result);
}

void sm2p_mong_pow(const u32* x, u32* result)
{
	u8 interim[9] = { 0 };
	raw_pow(x->v, interim);

	sm2p_mong_mul_core(interim, result);
}

void sm2p_mong_mul(const u32* x, const u32* y, u32* result)
{
	u8 interim[9] = { 0 };
	raw_mul(x->v, y->v, interim);

	sm2p_mong_mul_core(interim, result);
}

void sm2n_mong_mul_core(UINT64 interim[9], u32* result)
{
	const static size_t LEN = 9;
	int i = 0, j = 0;
	for(i = 0; i < 4; i++)
	{
		u1 carry = 0;
		u8 h;
		u8 l;
		size_t pos;

		u8 factor = portable_mulx_u64(interim[i], SM2_neg_invN, &h);

		if (factor == 0)
			continue;
		if (factor == 1)
		{
			for(j = 0; j < 4; j++)
			{
				carry = portable_addcarryx_u64(carry, interim[i + j + 0], SM2_N.v[j], &interim[i + j + 0]);
				size_t pos = 1 + i + j;
				while (carry && pos < LEN)
				{
					carry = portable_addcarryx_u64(carry, interim[pos], 0, &interim[pos]);
					++pos;
				}
			}
		}

		carry = 0;
		l = portable_mulx_u64(factor, SM2_N.v[0], &h);
		carry = portable_addcarryx_u64(carry, interim[i + 0], l, &interim[i + 0]);
		carry = portable_addcarryx_u64(carry, interim[i + 1], h, &interim[i + 1]);
		pos = 2 + i;
		while (carry && pos < LEN)
		{
			carry = portable_addcarryx_u64(carry, interim[pos], 0, &interim[pos]);
			++pos;
		}


		carry = 0;
		h = factor - 1;
		l = ~factor + 1;
		carry = portable_addcarryx_u64(carry, interim[i + 2], l, &interim[i + 2]);
		carry = portable_addcarryx_u64(carry, interim[i + 3], h, &interim[i + 3]);
		pos = 4 + i;
		while (carry && pos < LEN)
		{
			carry = portable_addcarryx_u64(carry, interim[pos], 0, &interim[pos]);
			++pos;
		}


		carry = 0;
		l = portable_mulx_u64(factor, SM2_N.v[1], &h);
		carry = portable_addcarryx_u64(carry, interim[i + 1], l, &interim[i + 1]);
		carry = portable_addcarryx_u64(carry, interim[i + 2], h, &interim[i + 2]);
		pos = 3 + i;
		while (carry && pos < LEN)
		{
			carry = portable_addcarryx_u64(carry, interim[pos], 0, &interim[pos]);
			++pos;
		}

		carry = 0;
		l = portable_mulx_u64(factor, SM2_N.v[3], &h);
		carry = portable_addcarryx_u64(carry, interim[i + 3], l, &interim[i + 3]);
		carry = portable_addcarryx_u64(carry, interim[i + 4], h, &interim[i + 4]);
		pos = 5 + i;
		while (carry && pos < LEN)
		{
			carry = portable_addcarryx_u64(carry, interim[pos], 0, &interim[pos]);
			++pos;
		}
	}
	memcpy(result, interim + 4, 32);

	if (interim[8] != 0)
		u32_add(result, &SM2_rhoN, result);

	if (u32_ge(result, &SM2_N))
		u32_add(result, &SM2_rhoN, result);
}

void sm2n_mong_mul(const u32* x, const u32* y, u32* result)
{
	u8 interim[9] = { 0 };
	raw_mul(x->v, y->v, interim);

	sm2n_mong_mul_core(interim, result);
}

// ============================================================================
// Portable C implementations of x64 intrinsics
// ============================================================================

// Portable implementation of _addcarryx_u64
// Computes: result = a + b + carry_in, returns carry_out
u1 portable_addcarryx_u64(u1 carry_in, u8 a, u8 b, u8* result)
{
	u8 sum = a + b;
	u1 carry1 = (sum < a) ? 1 : 0;
	u8 sum2 = sum + carry_in;
	u1 carry2 = (sum2 < sum) ? 1 : 0;
	*result = sum2;
	return carry1 | carry2;
}

// Portable implementation of _mulx_u64
// Computes: (hi:lo) = a * b, returns lo, stores hi in *hi_ptr
u8 portable_mulx_u64(u8 a, u8 b, u8* hi_ptr)
{
	// Split 64-bit values into 32-bit halves
	u4 a_lo = (u4)a;
	u4 a_hi = (u4)(a >> 32);
	u4 b_lo = (u4)b;
	u4 b_hi = (u4)(b >> 32);

	// Compute partial products (each is at most 64 bits)
	u8 p0 = (u8)a_lo * b_lo;  // low * low
	u8 p1 = (u8)a_lo * b_hi;  // low * high
	u8 p2 = (u8)a_hi * b_lo;  // high * low
	u8 p3 = (u8)a_hi * b_hi;  // high * high

	// Combine partial products
	// Result = p0 + (p1 << 32) + (p2 << 32) + (p3 << 64)
	u8 lo = p0;
	u8 hi = p3;

	// Add p1 << 32
	u8 mid = p1 + p2;
	u1 mid_carry = (mid < p1) ? 1 : 0;

	u8 mid_lo = mid << 32;
	u8 mid_hi = (mid >> 32) + ((u8)mid_carry << 32);

	lo += mid_lo;
	u1 lo_carry = (lo < mid_lo) ? 1 : 0;
	hi += mid_hi + lo_carry;

	*hi_ptr = hi;
	return lo;
}

// ============================================================================
// Portable C implementations of raw_mul and raw_pow
// ============================================================================

// 256-bit multiplication: result[8] = a[4] * b[4]
// Uses schoolbook multiplication with column-wise accumulation
void raw_mul(const UINT64 a[4], const UINT64 b[4], UINT64 result[8])
{
	u8 h, l;
	u1 carry, carry2 = 0;
	result[0] = result[1] = result[2] = 0;

#define MUL_ACC(i, j) \
	l = portable_mulx_u64(a[i], b[j], &h); \
	carry = portable_addcarryx_u64(0, result[(i)+(j)], l, &result[(i)+(j)]); \
	carry2 += portable_addcarryx_u64(carry, result[(i)+(j)+1], h, &result[(i)+(j)+1])

	// result[0]: a[0] * b[0]
	MUL_ACC(0, 0);  // carry2 must remain 0 here

	// result[1]: a[0]*b[1] + a[1]*b[0]
	MUL_ACC(0, 1);
	MUL_ACC(1, 0);
	result[3] = carry2; carry2 = 0;

	// result[2]: a[0]*b[2] + a[1]*b[1] + a[2]*b[0]
	MUL_ACC(0, 2);
	MUL_ACC(1, 1);
	MUL_ACC(2, 0);
	result[4] = carry2; carry2 = 0;

	// result[3]: a[0]*b[3] + a[1]*b[2] + a[2]*b[1] + a[3]*b[0]
	MUL_ACC(0, 3);
	MUL_ACC(1, 2);
	MUL_ACC(2, 1);
	MUL_ACC(3, 0);
	result[5] = carry2; carry2 = 0;

	// result[4]: a[1]*b[3] + a[2]*b[2] + a[3]*b[1]
	MUL_ACC(1, 3);
	MUL_ACC(2, 2);
	MUL_ACC(3, 1);
	result[6] = carry2; carry2 = 0;

	// result[5]: a[2]*b[3] + a[3]*b[2]
	MUL_ACC(2, 3);
	MUL_ACC(3, 2);
	result[7] = carry2;  // carry2 becomes the initial value for result[7]

	// result[6], result[7]: a[3]*b[3]
	MUL_ACC(3, 3);

#undef MUL_ACC
}

// 256-bit squaring: result[8] = a[4] * a[4]
// Optimized by computing off-diagonal terms once and doubling
void raw_pow(const UINT64 a[4], UINT64 result[8])
{
	u8 h, l;
	u1 carry, carry2 = 0;
	result[0] = result[1] = result[2] = 0;

// Square term: add a[i]*a[i] directly
#define SQR_ACC(i) \
	l = portable_mulx_u64(a[i], a[i], &h); \
	carry = portable_addcarryx_u64(0, result[(i)*2], l, &result[(i)*2]); \
	carry2 += portable_addcarryx_u64(carry, result[(i)*2+1], h, &result[(i)*2+1])

// Cross term: add 2*a[i]*a[j] (where i < j)
#define MUL2_ACC(i, j) \
	l = portable_mulx_u64(a[i], a[j], &h); \
	carry = portable_addcarryx_u64(0, result[(i)+(j)], (l << 1), &result[(i)+(j)]); \
	carry2 += portable_addcarryx_u64(carry, result[(i)+(j)+1], (h << 1) | (l >> 63), &result[(i)+(j)+1]); \
	carry2 += (h >> 63)

	// result[0]: a[0]^2
	SQR_ACC(0);  // carry2 must remain 0 here

	// result[1]: 2*a[0]*a[1]
	MUL2_ACC(0, 1);
	result[3] = carry2; carry2 = 0;

	// result[2]: a[1]^2 + 2*a[0]*a[2]
	SQR_ACC(1);
	MUL2_ACC(0, 2);
	result[4] = carry2; carry2 = 0;

	// result[3]: 2*a[0]*a[3] + 2*a[1]*a[2]
	MUL2_ACC(0, 3);
	MUL2_ACC(1, 2);
	result[5] = carry2; carry2 = 0;

	// result[4]: a[2]^2 + 2*a[1]*a[3]
	SQR_ACC(2);
	MUL2_ACC(1, 3);
	result[6] = carry2; carry2 = 0;

	// result[5]: 2*a[2]*a[3]
	MUL2_ACC(2, 3);
	result[7] = carry2;  // carry2 becomes the initial value for result[7]

	// result[6], result[7]: a[3]^2
	SQR_ACC(3);

#undef SQR_ACC
#undef MUL2_ACC
}
