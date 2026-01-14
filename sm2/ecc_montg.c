
#include "include/ecc.h"
#include "include/utils.h"
#include "include/sm2_table.h"


static const UINT64 BN_ZERO[4] = { 0,0,0,0 };
// The number of 'One' in montgomery domain
static const UINT64 MONTG_ONE[4] = { 0x0000000000000001,0x00000000ffffffff,0x0000000000000000,0x0000000100000000 };

// Test wether equal to montgomery one
static inline bool montg_equal_one(const UINT64 src[4])
{
	const bool ret = 
		(src[0] ^ MONTG_ONE[0]) |
		(src[1] ^ MONTG_ONE[1]) | 
		(src[2] ^ MONTG_ONE[2]) | 
		(src[3] ^ MONTG_ONE[3]) ;
	return !ret;
}

// Test wether two number is equal
static inline bool bignum_is_equal(const UINT64 a[4], const UINT64 b[4])
{
	const bool ret =
		(a[0] ^ b[0]) |
		(a[1] ^ b[1]) |
		(a[2] ^ b[2]) |
		(a[3] ^ b[3]);
	return !ret;
}

// Set a big number to zero
static inline void bignum_set_to_zero(UINT64 a[4])
{
	memset(a, 0, sizeof(BN_ZERO));
}

// Copy big number for 4 slob
static inline void copy_bignum(const UINT64* src, UINT64* dst)
{
	memcpy(dst, src, sizeof(UINT64) * 4);
}

// Set jacobian point to zero in montgomery domain
static inline void montg_set_jpoint_to_zero(JPoint* dst)
{
	memcpy(dst->x.v, MONTG_ONE, sizeof(MONTG_ONE));
	memcpy(dst->y.v, MONTG_ONE, sizeof(MONTG_ONE));
	memset(dst->z.v, 0,         sizeof(MONTG_ONE));
}

// Whether this is a zero affine point
static inline bool montg_is_apoint_zero(const AFPoint* dst)
{
	// Affine zero point is represented as (0, 0)
	if (!bignum_is_equal(dst->x.v, BN_ZERO))
	{
		return false;
	}
	if (!bignum_is_equal(dst->y.v, BN_ZERO))
	{
		return false;
	}
	return true;
}

// Whether this is a zero jacobian point
static inline bool montg_is_jpoint_zero(const JPoint* dst)
{
	// Jacobian zero point is represented as (x, y, 0)
	if (bignum_is_equal(dst->z.v, BN_ZERO))
	{
		return true;
	}
	return false;
}

// Convert jacobian point (montgomery domain) to affine point (residue domain)
// Complexity:
//      3M + 1S + 1I
//      M: Multiplication
//      S: Square
//      I: Inversion
int montg_jpoint_to_apoint(const JPoint* a, UINT64* rx, UINT64* ry)
{
	// Transform (X, Y, Z) into (x, y) = (X/Z^2, Y/Z^3)
	const u32* ax = &(a->x);
	const u32* ay = &(a->y);
	const u32* az = &(a->z);
	u32 Zinv, Zisqr, T;

	if (montg_is_jpoint_zero(a))
	{
		if (rx) bignum_set_to_zero(rx);
		if (ry) bignum_set_to_zero(ry);
		return 1;
	}

	// Montgomery version of "1"
	if (montg_equal_one(az->v))
	{
		// Convert x and y from montgomery domain to residue domain
		if (rx) montg_back_mod_p(ax, (u32*)rx);
		if (ry) montg_back_mod_p(ay, (u32*)ry);
		return 1;
	}

	montg_back_mod_p(az, &T);         // Convert z from montgomery domain to residue domain
	// Zinv = Z^(-1) = 1 / z
	montg_inv_mod_p_ex(T.v, Zinv.v);
	montg_sqr_mod_p(&Zinv, &Zisqr);    // Zisqr = 1 / (z^2)

	if (rx)
	{
		montg_mul_mod_p(ax, &Zisqr, (u32*)rx);  // rx = x / (z^2)
		montg_back_mod_p((u32*)rx, (u32*)rx);        // Convert x from montgomery domain to residue domain
	}
	if (ry)
	{
		montg_mul_mod_p(&Zisqr, &Zinv, &T); // T = 1 / (z^3)
		montg_mul_mod_p(ay, &T, (u32*)ry);      // ry = y / (z^3)
		montg_back_mod_p((u32*)ry, (u32*)ry);        // Convert y from montgomery domain to residue domain
	}
	return 1;
}

// Convert jacobian point (montgomery domain) to affine point (montgomery domain)
// Complexity:
//      3M + 1S + 1I
//      M: Multiplication
//      S: Square
//      I: Inversion
int montg_jpoint_to_apoint2(const JPoint* a, AFPoint* r)
{
	// Transform (X, Y, Z) into (x, y) = (X/Z^2, Y/Z^3)
	const u32* ax = &(a->x);
	const u32* ay = &(a->y);
	const u32* az = &(a->z);
	u32* rx = &(r->x);
	u32* ry = &(r->y);
	u32 Zinv, Zisqr, T;

	//// Infinity point represented as (x, y, 0)
	if (montg_is_jpoint_zero(a))
	{
		bignum_set_to_zero(rx->v);
		bignum_set_to_zero(ry->v);
		return 1;
	}

	// Montgomery version of "1"
	if (montg_equal_one(az->v))
	{
		// Convert x and y from montgomery domain to residue domain
		copy_bignum(ax->v, rx->v);
		copy_bignum(ay->v, ry->v);
		return 1;
	}

	montg_back_mod_p(az, &T);         // Convert z from montgomery domain to residue domain
	// Zinv = Z^(-1) = 1 / z
	montg_inv_mod_p_ex(T.v, Zinv.v);
	montg_sqr_mod_p(&Zinv, &Zisqr);    // Zisqr = 1 / (z^2)

	montg_mul_mod_p(ax, &Zisqr, rx);  // rx = x / (z^2)
	montg_mul_mod_p(&Zisqr, &Zinv, &T); // T = 1 / (z^3)
	montg_mul_mod_p(ay, &T, ry);      // ry = y / (z^3)

	return 1;
}

// Convert affine point (residue domain) to jacobian point (montgomery domain)
int montg_apoint_to_jpoint(const AFPoint* a, JPoint* r)
{
	const u32* ax = &(a->x);
	const u32* ay = &(a->y);
	const UINT64* az = MONTG_ONE;
	u32* rx = &(r->x);
	u32* ry = &(r->y);
	UINT64* rz = r->z.v;

	if (montg_is_apoint_zero(a))
	{
		montg_set_jpoint_to_zero(r);
		return 1;
	}

	// Convert x and y from residue domain to montgomery domain
	montg_to_mod_p(ax, rx);
	montg_to_mod_p(ay, ry);
	copy_bignum(az, rz);

	return 1;
}

// Convert affine point (montgomery domain) to jacobian point (montgomery domain)
int montg_apoint_to_jpoint2(const AFPoint* a, JPoint* r)
{
	const UINT64* ax = a->x.v;
	const UINT64* ay = a->y.v;
	const UINT64* az = MONTG_ONE;
	UINT64* rx = r->x.v;
	UINT64* ry = r->y.v;
	UINT64* rz = r->z.v;

	if (montg_is_apoint_zero(a))
	{
		montg_set_jpoint_to_zero(r);
		return 1;
	}

	copy_bignum(ax, rx);
	copy_bignum(ay, ry);
	copy_bignum(az, rz);

	return 1;
}

// Convert affine point (residue domain) to affine point (montgomery domain)
// Input: 
//      a -- in residue domain
// Output: 
//      r -- in montgomery domain
int montg_apoint_to_montg(const AFPoint* a, AFPoint* r)
{
	const u32* ax = &(a->x);
	const u32* ay = &(a->y);
	u32* rx = &(r->x);
	u32* ry = &(r->y);
	
	// Convert x and y from residue domain to montgomery domain
	montg_to_mod_p(ax, rx);
	montg_to_mod_p(ay, ry);

	return 1;
}

// Convert jacobian point (residue domain) to jacobian point (montgomery domain)
// Input: 
//      a -- in residue domain
// Output: 
//      r -- in montgomery domain
int montg_jpoint_to_montg(const JPoint* a, JPoint* r)
{
	const u32* ax = &(a->x);
	const u32* ay = &(a->y);
	const u32* az = &(a->z);
	u32* rx = &(r->x);
	u32* ry = &(r->y);
	u32* rz = &(r->z);

	// Convert x,y and z from residue domain to montgomery domain
	montg_to_mod_p(ax, rx);
	montg_to_mod_p(ay, ry);
	montg_to_mod_p(az, rz);

	return 1;
}

// Double jacobian point, either in montgomery domain or in residue domain
// Complexity:
//      4M + 4S
//      M: Multiplication
//      S: Square
void montg_double_jpoint(const JPoint* a, JPoint* r)
{
	const u32* ax = &(a->x);
	const u32* ay = &(a->y);
	const u32* az = &(a->z);
	u32* rx = &(r->x);
	u32* ry = &(r->y);
	u32* rz = &(r->z);
	u32 U1, U2, U3, Zsqr, T;

	montg_sqr_mod_p(az, &Zsqr);      // Zsqr = z^2

	mul_by_2_mod_p(ay, &T);          // T = 2y
	montg_mul_mod_p(&T, az, rz);     // rz = 2yz

	montg_sqr_mod_p(&T, &T);          // T = 4y^2
	montg_mul_mod_p(&T, ax, &U2);     // U2 = 4xy^2

	montg_sqr_mod_p(&T, &T);          // T = 16y^4
	div_by_2_mod_p(&T, &U3);          // U3 = 8y^4

	sub_mod_p(ax, &Zsqr, &T);         // T = x - y^2
	add_mod_p(ax, &Zsqr, &U1);        // U1 = x + y^2
	montg_mul_mod_p(&T, &U1, &U1);     // U1 = (x + y^2)(x - y^2)
	mul_by_3_mod_p(&U1, &U1);         // U1 = 3(x + y^2)(x - y^2) = 3x^2 + az^4     // a = p-3

	montg_sqr_mod_p(&U1, &T);         // T = U1^2
	mul_by_2_mod_p(&U2, rx);         // rx = 2U2
	sub_mod_p(&T, rx, rx);           // rx = U1^2 - 2U2

	sub_mod_p(&U2, rx, &T);           // T = U2 - rx
	montg_mul_mod_p(&U1, &T, &T);      // T = U1(U2 - rx)
	sub_mod_p(&T, &U3, ry);           // ry = U1(U2 - rx) - U3

}

// Add jacobian point, either in montgomery domain or in residue domain
// Complexity:
//      12M + 4S
//      M: Multiplication
//      S: Square
int montg_add_jpoint(const JPoint* a, const JPoint* b, JPoint* r)
{
	const u32 *ax = &(a->x);
	const u32 *ay = &(a->y);
	const u32 *az = &(a->z);
	const u32 *bx = &(b->x);
	const u32 *by = &(b->y);
	const u32 *bz = &(b->z);
	u32* rx = &(r->x);
	u32* ry = &(r->y);
	u32* rz = &(r->z);
	u32 U1, U2, U3, U4, U5, U6, U7, U8, U9, Z1sqr, Z2sqr, U3sqr, T;

	if (montg_is_jpoint_zero(a))
	{
		CopyJPoint(b, r);
		return 1;
	}
	if (montg_is_jpoint_zero(b))
	{
		CopyJPoint(a, r);
		return 1;
	}

	montg_sqr_mod_p(az, &Z1sqr);       // Z1sqr = z1^2
	montg_sqr_mod_p(bz, &Z2sqr);       // Z2sqr = z2^2

	montg_mul_mod_p(ax, &Z2sqr, &U1);   // U1 = x1 * z2^2
	montg_mul_mod_p(bx, &Z1sqr, &U2);   // U2 = x2 * z1^2

	montg_mul_mod_p(az, &Z1sqr, &T);    // T = z1^3
	montg_mul_mod_p(by, &T, &U5);       // U5 = y2 * z1^3

	montg_mul_mod_p(bz, &Z2sqr, &T);    // T = z2^3
	montg_mul_mod_p(ay, &T, &U4);       // U4 = y1 * z2^3

	// a == b ?
	if (bignum_is_equal(U1.v, U2.v) && bignum_is_equal(U4.v, U5.v))
	{
		montg_double_jpoint(a, r);
		return 1;
	}

	sub_mod_p(&U1, &U2, &U3);             // U3 = U1 - U2
	sub_mod_p(&U4, &U5, &U6);             // U6 = U4 - U5
	add_mod_p(&U1, &U2, &U7);             // U7 = U1 + U2
	add_mod_p(&U4, &U5, &U8);             // U8 = U4 + U5

	// Get rx
	montg_sqr_mod_p(&U3, &U3sqr);        // U3sqr = U3^2
	montg_mul_mod_p(&U7, &U3sqr, &T);     // T = U7 * U3^2
	montg_sqr_mod_p(&U6, rx);           // rx = U6^2
	sub_mod_p(rx, &T, rx);              // rx = U6^2 - U7 * U3^2

	// Get U9
	memcpy(&U9, &T, sizeof(U9));         // U9 = U7 * U3^2
	mul_by_2_mod_p(rx, &T);             // T = 2 * rx
	sub_mod_p(&U9, &T, &U9);              // U9 = U7 * U3^2 - 2 * rx

	// Get ry
	montg_mul_mod_p(&U3sqr, &U3, &T);     // T = U3^3
	montg_mul_mod_p(&U8, &T, &T);         // T = U8 * U3^3
	montg_mul_mod_p(&U9, &U6, ry);       // ry = U9 * U6
	sub_mod_p(ry, &T, ry);              // ry = U9 * U6 - U8 * U3^3
	div_by_2_mod_p(ry, ry);            // ry = (U9 * U6 - U8 * U3^3)/2

	// Get rz
	montg_mul_mod_p(az, bz, &T);        // T = z1 * z2
	montg_mul_mod_p(&T, &U3, rz);        // rz = U3 * z1 * z2

	return 1;
}

// Add affine point (montgomery domain) and jacobian point (montgomery domain)
// Input: 
//      a -- in montgomery domain, jacobian point
//      b -- in montgomery domain, affine point
// Output: 
//      r -- in montgomery domain
// Complexity:
//      8M + 3S
//      M: Multiplication
//      S: Square
int montg_add_jpoint_and_apoint(const JPoint* a, const AFPoint* b, JPoint* r)
{
	const u32* ax = &(a->x);
	const u32* ay = &(a->y);
	const u32* az = &(a->z);
	const u32* bx = &(b->x);
	const u32* by = &(b->y);

	u32* rx = &(r->x);
	u32* ry = &(r->y);
	u32* rz = &(r->z);
	u32 A, B, C, D, E, Zsqr, Ccub, T;


	if (montg_is_jpoint_zero(a))
	{
		montg_apoint_to_jpoint2(b, r);
		return 1;
	}
	if (montg_is_apoint_zero(b))
	{
		CopyJPoint(a, r);
		return 1;
	}

	montg_sqr_mod_p(az, &Zsqr);         // Zsqr = z1^2
	montg_mul_mod_p(bx, &Zsqr, &A);      // A = x2 * z1^2

	montg_mul_mod_p(az, &Zsqr, &T);      // T = z1^3
	montg_mul_mod_p(by, &T, &B);         // B = y2 * z1^3

	sub_mod_p(&A, ax, &C);               // C = A - x1
	sub_mod_p(&B, ay, &D);               // D = B - y1

	montg_sqr_mod_p(&C, &T);             // T = C^2
	montg_mul_mod_p(&C, &T, &Ccub);       // Ccub = C^3
	montg_mul_mod_p(ax, &T, &E);         // E = x1 * C^2

	// Get rx
	mul_by_2_mod_p(&E, &T);              // T = 2 * E
	add_mod_p(&Ccub, &T, &T);             // T =  C^3 + 2 * E
	montg_sqr_mod_p(&D, rx);            // rx = D^2
	sub_mod_p(rx, &T, rx);              // rx = D^2 - (C^3 + 2 * E)

	// Get ry
	sub_mod_p(&E, rx, &T);               // T = E - rx
	montg_mul_mod_p(&D, &T, &T);          // T = D * (E - rx)
	montg_mul_mod_p(ay, &Ccub, ry);     // ry = y1 * C^3
	sub_mod_p(&T, ry, ry);              // ry = D * (E - rx) - y1 * C^3

	// Get rz
	montg_mul_mod_p(az, &C, rz);        // rz = z1 * C

	return 1;
}

// Pre-compute for point, Just two item for each table, aka. PT[1] and PT[3]
// Input: 
//      apoint -- in residue domain
// Output: 
//      PT -- in montgomery domain, store positive point
//      NT -- in montgomery domain, store negative point
// Complexity:
//      1 JPOINT_DBL + 1 MPOINT_ADD + 1 JP_TO_AP = 
//      1(4M + 4S) + 1(8M + 3S) + 1(3M + 1S + 1I) = 15M + 8S + 1I
//      JPOINT_DBL: Jacobian point double
//      JPOINT_ADD: Jacobian point addition
void montg_pre_compute_naf_w3(const AFPoint* apoint, AFPoint PT[4], AFPoint NT[4])
{
	// Get 1P, 3P
	// P[i] = iP, i is odd
	// P[i] = 0,  i is even
	JPoint r, dr;

	// Convert to montgomery domain
	montg_apoint_to_montg(apoint, PT + 1); // 1 P
	AFPoint_neg(PT + 1, NT + 1);           // -1 P

	// Convert to jacobian point
	montg_apoint_to_jpoint2(PT + 1, &r);   // r  = 1 P
	montg_double_jpoint_ex(&r, &dr);          // dr = 2 P

	// Get 3P and convert to residue domain
	montg_add_jpoint_and_apoint_ex(&dr, PT + 1, &r);
	montg_jpoint_to_apoint2(&r, PT + 3);
	AFPoint_neg(PT + 3, NT + 3);
}

// Scalar multiplication in montgomery domain
// Input: 
//      P            -- in residue domain
//      k            -- in residue domain
// Output: 
//      result = kP  -- in montgomery domain
// Complexity:
//      ~256 JPOINT_DBL + 64 MPOINT_ADD + (15M + 8S + 1I) = 
//      256(4M + 4S) + 64(8M + 3S) + (15M + 8S + 1I) = 1551M + 1224S + 1I
//      JPOINT_DBL: Jacobian point double
//      MPOINT_ADD: Mixed jacobian point  and affine point addition
void montg_times_point_naf_w3(const AFPoint* P, const u32* k, JPoint* result)
{
	int i = 0;
	int8_t ki = 0;
	int8_t naf_k[257] = { 0 };
	JPoint Q;
	static AFPoint PT[4] = { 0 };
	static AFPoint NT[4] = { 0 };

	montg_pre_compute_naf_w3(P, PT, NT);
	get_naf_w3(k, naf_k);
	montg_set_jpoint_to_zero(&Q);

	// First time
	ki = naf_k[256];
	if (ki > 0)
	{
		montg_apoint_to_jpoint2(PT + ki, &Q);
	}
	else if(ki < 0)
	{
		montg_apoint_to_jpoint2(NT + (-ki), &Q);
	}
	
	// Left 256 times
	for (i = 255; i >= 0; i--)
	{
		montg_double_jpoint_ex(&Q, &Q);
		ki = naf_k[i];
		if (ki)
		{
			if (ki > 0)
			{
				montg_add_jpoint_and_apoint_ex(&Q, PT + ki, &Q);
			}
			else
			{
				montg_add_jpoint_and_apoint_ex(&Q, NT + (-ki), &Q);
			}
		}
	}
	CopyJPoint(&Q, result);
}

// Pre-compute for point
// Input: 
//      apoint -- in residue domain
// Output: 
//      PT -- in montgomery domain, store positive point
//      NT -- in montgomery domain, store negative point
// Complexity:
//      1 JPOINT_DBL + 7 JPOINT_ADD = 1(4M + 4S) + 7(12M + 4S) = 88M + 32S
//      JPOINT_DBL: Jacobian point double
//      JPOINT_ADD: Jacobian point addition
void montg_pre_compute_naf_w5_all_jpoint(const AFPoint* apoint, JPoint PT[16], JPoint NT[16])
{
	// Get 1P, 3P, 5P, 7P, 9P, 11P, 13P, 15P
	// P[i] = iP, i is odd
	// P[i] = 0,  i is even
	int i = 0;
	JPoint r, dr;

	montg_apoint_to_jpoint(apoint, &r);  // r = 1 P
	montg_double_jpoint_ex(&r, &dr);        // dr = 2 P

	CopyJPoint(&r, PT + 1);         // 1 P
	JPoint_neg(&r, NT + 1);         // -1 P

	for (i = 3; i < 16; i += 2)
	{
		montg_add_jpoint_ex(&r, &dr, &r);
		CopyJPoint(&r, PT + i);
		JPoint_neg(&r, NT + i);
	}
}

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
void montg_times_point_naf_w5_all_jpoint(const AFPoint* P, const u32* k, JPoint* result)
{
	int i = 0;
	int8_t ki = 0;
	int8_t naf_k[257] = { 0 };
	JPoint Q;
	static JPoint PT[16] = { 0 };
	static JPoint NT[16] = { 0 };

	montg_pre_compute_naf_w5_all_jpoint(P, PT, NT);
	get_naf_w5_2(k, naf_k);
	montg_set_jpoint_to_zero(&Q);

	// First time
	ki = naf_k[256];
	if (ki > 0)
	{
		montg_add_jpoint_ex(&Q, PT + ki, &Q);
	}
	else if(ki < 0)
	{
		montg_add_jpoint_ex(&Q, NT + (-ki), &Q);
	}

	// Left 256 times
	for (i = 255; i >= 0; i--)
	{
		montg_double_jpoint_ex(&Q, &Q);
		ki = naf_k[i];
		if (ki)
		{
			if (ki > 0)
			{
				montg_add_jpoint_ex(&Q, PT + ki, &Q);
			}
			else
			{
				montg_add_jpoint_ex(&Q, NT + (-ki), &Q);
			}
		}
	}
	CopyJPoint(&Q, result);
}

// Scalar multiplication in montgomery domain for fixed point
// Input: 
//      k            -- in residue domain
// Output: 
//      result = kG  -- in montgomery domain
// Complexity:
//      28 MPOINT_ADD + 3 JPOINT_ADD = 28(8M + 3S) + 3(12M + 4S) = 260M + 96S
//      MPOINT_ADD: Mix point addition
//      JPOINT_ADD: Jacobian point addition
void montg_times_base_point(const u32* k, JPoint* result)
{
	int i = 0;
	uint8_t byteIdx = 0;
	uint8_t* pk = NULL;
	const AFPoint *pT = NULL;
	JPoint K0, K1, K2, K3;

	// k[0]
	pk = (uint8_t*)(k->v + 0);
	byteIdx = pk[0];
	pT = &(g_montg_AFTable_for_base_point_mul[0][0][byteIdx]);
	montg_apoint_to_jpoint2(pT, &K0);
	for (i = 1; i < 8; i++)
	{
		byteIdx = pk[i];
		pT = &(g_montg_AFTable_for_base_point_mul[0][i][byteIdx]);
		montg_add_jpoint_and_apoint_ex(&K0, pT, &K0);
	}

	// k[1]
	pk = (uint8_t*)(k->v + 1);
	byteIdx = pk[0];
	pT = &(g_montg_AFTable_for_base_point_mul[1][0][byteIdx]);
	montg_apoint_to_jpoint2(pT, &K1);
	for (i = 1; i < 8; i++)
	{
		byteIdx = pk[i];
		pT = &(g_montg_AFTable_for_base_point_mul[1][i][byteIdx]);
		montg_add_jpoint_and_apoint_ex(&K1, pT, &K1);
	}

	// k[2]
	pk = (uint8_t*)(k->v + 2);
	byteIdx = pk[0];
	pT = &(g_montg_AFTable_for_base_point_mul[2][0][byteIdx]);
	montg_apoint_to_jpoint2(pT, &K2);
	for (i = 1; i < 8; i++)
	{
		byteIdx = pk[i];
		pT = &(g_montg_AFTable_for_base_point_mul[2][i][byteIdx]);
		montg_add_jpoint_and_apoint_ex(&K2, pT, &K2);
	}

	// k[3]
	pk = (uint8_t*)(k->v + 3);
	byteIdx = pk[0];
	pT = &(g_montg_AFTable_for_base_point_mul[3][0][byteIdx]);
	montg_apoint_to_jpoint2(pT, &K3);
	for (i = 1; i < 8; i++)
	{
		byteIdx = pk[i];
		pT = &(g_montg_AFTable_for_base_point_mul[3][i][byteIdx]);
		montg_add_jpoint_and_apoint_ex(&K3, pT, &K3);
	}

	montg_add_jpoint_ex(&K0, &K1, &K0);
	montg_add_jpoint_ex(&K0, &K2, &K0);
	montg_add_jpoint_ex(&K0, &K3, result);
}
void montg_times_base_point2(const u32* k, JPoint* result)
{
	int i = 0, j = 0;
	JPoint Tr;
	JPoint T;
	uint8_t* pByte = NULL, byte;
	const AFPoint *pTable = NULL;

	montg_set_jpoint_to_zero(&Tr);
	for (i = 0; i < 4; i++)
	{
		pByte = (uint8_t*)(&(k->v[i]));

		// Reset T to ZERO
		montg_set_jpoint_to_zero(&T);
		for (j = 0; j < 8; j++)
		{
			byte = pByte[j];
			pTable = &(g_montg_AFTable_for_base_point_mul[i][j][byte]);
			//montg_add_jpoint_and_apoint(&T, pTable, &T);
			montg_add_jpoint_and_apoint_ex(&T, pTable, &T);
		}
		//montg_add_jpoint(&Tr, &T, &Tr);
		montg_add_jpoint_ex(&Tr, &T, &Tr);
	}
	CopyJPoint(&Tr, result);
}

// Simplest scalar multiplication in montgomery domain for random point
// Input: 
//      P            -- in montgomery domain
//      k            -- in residue domain
// Output: 
//      result = kG  -- in montgomery domain
void montg_naive_times_point(const AFPoint* P, const u32* k, JPoint* result)
{
	int i = 0, j = 0;
	uint8_t b = 0;
	uint8_t sk[256] = { 0 };
	uint8_t* p = NULL;
	JPoint T;

	// Convert k into binary form
	p = (uint8_t*)(k->v);
	for (i = 0; i < 32; i++)
	{
		b = p[i];
		for (j = 0; j < 8; j++)
		{
			if (b & (1 << j))
			{
				sk[i * 8 + j] = 1;
			}
			else
			{
				sk[i * 8 + j] = 0;
			}
		}
	}

	montg_set_jpoint_to_zero(&T);
	for(i = 255; i >=0; i--)
	{
		montg_double_jpoint(&T, &T);
		if (sk[i] > 0)
		{
			montg_add_jpoint_and_apoint(&T, P, &T);
		}
	}
	CopyJPoint(&T, result);
}

// ============================================================================
// Portable C implementations of ECC assembly functions (originally in ecc_as.s)
// These wrapper functions call the existing C implementations
// ============================================================================

// Perform doubling jacobian point in montgomery domain
// Wrapper for montg_double_jpoint
void montg_double_jpoint_ex(const JPoint* a, JPoint* r)
{
	montg_double_jpoint(a, r);
}

// Perform adding two jacobian points in montgomery domain
// Wrapper for montg_add_jpoint
void montg_add_jpoint_ex(const JPoint* a, const JPoint* b, JPoint* r)
{
	montg_add_jpoint(a, b, r);
}

// Perform adding jacobian point and affine point in montgomery domain
// Wrapper for montg_add_jpoint_and_apoint
void montg_add_jpoint_and_apoint_ex(const JPoint* a, const AFPoint* b, JPoint* r)
{
	montg_add_jpoint_and_apoint(a, b, r);
}

// Perform inversion in montgomery domain (mod P)
// Wrapper for MontgInvModp
void montg_inv_mod_p_ex(const UINT64 a[4], UINT64 r[4])
{
	MontgInvModp((const u32*)a, (u32*)r);
}

// Perform inversion in montgomery domain (mod N)
// Wrapper for MontgInvModn
void montg_inv_mod_n_ex(const UINT64 a[4], UINT64 r[4])
{
	MontgInvModn((const u32*)a, (u32*)r);
}
