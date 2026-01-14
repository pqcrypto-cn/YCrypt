
#include "include/ecc.h"
#include "include/utils.h"
#include "include/sm2_table.h"


static const UINT64 BN_ZERO[4] = { 0,0,0,0 };
// The number of 'One' in montgomery domain
static const UINT64 MONTG_ONE[4] = { 0x0000000000000001,0x00000000ffffffff,0x0000000000000000,0x0000000100000000 };
//static const JPoint MONTG_JPOINT_ZERO = { {MONTG_ONE}, {MONTG_ONE}, {0,0,0,0} };

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
	// Ref: Fast Prime Field Elliptic Curve Cryptography with 256 bit primes,  P4
	// Transform (X, Y, Z) into (x, y) = (X/Z^2, Y/Z^3)
	// const UINT64* ax = a->x.v;
	// const UINT64* ay = a->y.v;
	// const UINT64* az = a->z.v;
	// UINT64 t, Zinv[4], Zisqr[4], T[4];

	const u32* ax = &(a->x);
	const u32* ay = &(a->y);
	const u32* az = &(a->z);
	u32 Zinv, Zisqr, T;

	//// Infinity point represented as (x, y, 0)
	//t = az[0] | az[1] | az[2] | az[3];
	//if (t == 0)  // Error
	//{
	//	return 0;
	//}

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
	//MontgInvModp(T, Zinv);           // Zinv = Z^(-1) = 1 / z
	//inv_for_mul_mod_p(az, Zinv);
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
	// Ref: Fast Prime Field Elliptic Curve Cryptography with 256 bit primes,  P4
	// Transform (X, Y, Z) into (x, y) = (X/Z^2, Y/Z^3)
	// const UINT64* ax = a->x.v;
	// const UINT64* ay = a->y.v;
	// const UINT64* az = a->z.v;
	// UINT64* rx = r->x.v;
	// UINT64* ry = r->y.v;
	// UINT64 t, Zinv[4], Zisqr[4], T[4];

	const u32* ax = &(a->x);
	const u32* ay = &(a->y);
	const u32* az = &(a->z);
	u32* rx = &(r->x);
	u32* ry = &(r->y);
	u32 Zinv, Zisqr, T;

	//// Infinity point represented as (x, y, 0)
	//t = az[0] | az[1] | az[2] | az[3];
	//if (t == 0)  // Error
	//{
	//	return 0;
	//}

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
	//MontgInvModp(T, Zinv);           // Zinv = Z^(-1) = 1 / z
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
	// Ref: Fast Prime Field Elliptic Curve Cryptography with 256 bit primes,  P4
	// Ref: E:\CryptoLib\GmSSL\gmssl_2.5.0_debug\GmSSL-master\crypto\ec\ecp_sm2z256.c\ecp_sm2z256_get_affine()
	// const UINT64* ax = a->x.v;
	// const UINT64* ay = a->y.v;
	// const UINT64* az = MONTG_ONE;
	// UINT64* rx = r->x.v;
	// UINT64* ry = r->y.v;
	// UINT64* rz = r->z.v;

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
	// Ref: Fast Prime Field Elliptic Curve Cryptography with 256 bit primes,  P4
	// Ref: E:\CryptoLib\GmSSL\gmssl_2.5.0_debug\GmSSL-master\crypto\ec\ecp_sm2z256.c\ecp_sm2z256_get_affine()
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
	// Ref: Fast Prime Field Elliptic Curve Cryptography with 256 bit primes,  P4
	// Ref: E:\CryptoLib\GmSSL\gmssl_2.5.0_debug\GmSSL-master\crypto\ec\ecp_sm2z256.c\ecp_sm2z256_get_affine()
	// const UINT64* ax = a->x.v;
	// const UINT64* ay = a->y.v;
	// UINT64* rx = r->x.v;
	// UINT64* ry = r->y.v;

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
	// Ref: Fast Prime Field Elliptic Curve Cryptography with 256 bit primes,  P4
	// Ref: E:\CryptoLib\GmSSL\gmssl_2.5.0_debug\GmSSL-master\crypto\ec\ecp_sm2z256.c\ecp_sm2z256_get_affine()
	// const UINT64* ax = a->x.v;
	// const UINT64* ay = a->y.v;
	// const UINT64* az = a->z.v;
	// UINT64* rx = r->x.v;
	// UINT64* ry = r->y.v;
	// UINT64* rz = r->z.v;

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
	// Ref: Fast Prime Field Elliptic Curve Cryptography with 256 bit primes,  P8
	// Single operations in montgomery domain for add/sub/mult-constant are equivalent as residue domain
	// Reference from SM2 standard part I, P13
	// const UINT64* ax = a->x.v;
	// const UINT64* ay = a->y.v;
	// const UINT64* az = a->z.v;
	// UINT64* rx = r->x.v;
	// UINT64* ry = r->y.v;
	// UINT64* rz = r->z.v;
	// UINT64 U1[4], U2[4], U3[4], Zsqr[4], T[4];

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
	// const UINT64* ax = a->x.v;
	// const UINT64* ay = a->y.v;
	// const UINT64* az = a->z.v;
	// const UINT64* bx = b->x.v;
	// const UINT64* by = b->y.v;
	// const UINT64* bz = b->z.v;
	// UINT64* rx = r->x.v;
	// UINT64* ry = r->y.v;
	// UINT64* rz = r->z.v;
	// UINT64 t = 0;
	// UINT64 U1[4], U2[4], U3[4], U4[4], U5[4], U6[4], U7[4], U8[4], U9[4], Z1sqr[4], Z2sqr[4], U3sqr[4], T[4];

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

	//// Infinity point represented as (x, y, 0)
	//t = az[0] | az[1] | az[2] | az[3];
	//if (t == 0)
	//{
	//	CopyJPoint(b, r);
	//	return 1;
	//}
	//t = bz[0] | bz[1] | bz[2] | bz[3];
	//if (t == 0)
	//{
	//	CopyJPoint(a, r);
	//	return 1;
	//}

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
	// // Ref: Software implementation of the NIST elliptic curves over prime fields, P12
	// const UINT64* ax = a->x.v;
	// const UINT64* ay = a->y.v;
	// const UINT64* az = a->z.v;
	// const UINT64* bx = b->x.v;
	// const UINT64* by = b->y.v;
	// //const UINT64* bz = b->z.v;
	// UINT64* rx = r->x.v;
	// UINT64* ry = r->y.v;
	// UINT64* rz = r->z.v;
	// UINT64 A[4], B[4], C[4], D[4], E[4], Zsqr[4], Ccub[4], T[4];

	const u32* ax = &(a->x);
	const u32* ay = &(a->y);
	const u32* az = &(a->z);
	const u32* bx = &(b->x);
	const u32* by = &(b->y);
	//const UINT64* bz = b->z.v;
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
	int i = 0;
	JPoint r, dr;

	// Convert to montgomery domain
	montg_apoint_to_montg(apoint, PT + 1); // 1 P
	AFPoint_neg(PT + 1, NT + 1);           // -1 P

	// Convert to jacobian point
	montg_apoint_to_jpoint2(PT + 1, &r);   // r  = 1 P
	//montg_double_jpoint(&r, &dr);          // dr = 2 P
	montg_double_jpoint_ex(&r, &dr);          // dr = 2 P

	// Get 3P and convert to residue domain
	//montg_add_jpoint_and_apoint(&dr, PT + 1, &r);
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
		//montg_double_jpoint(&Q, &Q);
		montg_double_jpoint_ex(&Q, &Q);
		ki = naf_k[i];
		if (ki)
		{
			if (ki > 0)
			{
				//montg_add_jpoint_and_apoint(&Q, PT + ki, &Q);
				montg_add_jpoint_and_apoint_ex(&Q, PT + ki, &Q);
			}
			else
			{
				//montg_add_jpoint_and_apoint(&Q, NT + (-ki), &Q);
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
	//montg_double_jpoint(&r, &dr);        // dr = 2 P
	montg_double_jpoint_ex(&r, &dr);        // dr = 2 P

	CopyJPoint(&r, PT + 1);         // 1 P
	JPoint_neg(&r, NT + 1);         // -1 P

	for (i = 3; i < 16; i += 2)
	{
		//montg_add_jpoint(&r, &dr, &r);
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
		//montg_add_jpoint(&Q, PT + ki, &Q);
		montg_add_jpoint_ex(&Q, PT + ki, &Q);
	}
	else if(ki < 0)
	{
		//montg_add_jpoint(&Q, NT + (-ki), &Q);
		montg_add_jpoint_ex(&Q, NT + (-ki), &Q);
	}

	// Left 256 times
	for (i = 255; i >= 0; i--)
	{
		//montg_double_jpoint(&Q, &Q);
		montg_double_jpoint_ex(&Q, &Q);
		ki = naf_k[i];
		if (ki)
		{
			if (ki > 0)
			{
				//montg_add_jpoint(&Q, PT + ki, &Q);
				montg_add_jpoint_ex(&Q, PT + ki, &Q);
			}
			else
			{
				//montg_add_jpoint(&Q, NT + (-ki), &Q);
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
	JPoint K0, K1, K2, K3, T;

	// k[0]
	pk = (uint8_t*)(k->v + 0);
	byteIdx = pk[0];
	pT = &(g_montg_AFTable_for_base_point_mul[0][0][byteIdx]);
	montg_apoint_to_jpoint2(pT, &K0);
	for (i = 1; i < 8; i++)
	{
		byteIdx = pk[i];
		pT = &(g_montg_AFTable_for_base_point_mul[0][i][byteIdx]);
		//montg_add_jpoint_and_apoint(&K0, pT, &K0);
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
		//montg_add_jpoint_and_apoint(&K1, pT, &K1);
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
		//montg_add_jpoint_and_apoint(&K2, pT, &K2);
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
		//montg_add_jpoint_and_apoint(&K3, pT, &K3);
		montg_add_jpoint_and_apoint_ex(&K3, pT, &K3);
	}

	//montg_add_jpoint(&K0, &K1, &K0);
	//montg_add_jpoint(&K0, &K2, &K0);
	//montg_add_jpoint(&K0, &K3, result);
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


/*
static void test_to_montg()
{
	UINT64 a[4] = { 1, 0, 0,0 };
	UINT64 r[4], t, b;

	montg_to_mod_p(a, r);

	//puts("r = ");
	//print_u32(r);

	//puts("1 = ");
	//print_u32(MONTG_ONE);

	t = sizeof(MONTG_ONE);
	b = memcmp(r, MONTG_ONE, t);
	//if (u32_eq(r, MONTG_ONE))
	if (b == 0)
	{
		puts("Equal");
	}
	else
	{
		puts("Not Equal");
	}
}

static void test_montg_conversion()
{
	AFPoint a, b, r;
	JPoint p, q;
	u32 k = { 0xaed66ce184be2329, 0xebe9bbf1f1499052, 0x993e0c873cdba6b3, 0xde47b7061c0d5e24 };

	get_random_u32_in_mod_n(&k);
	ML_mul_basepoint(&k, &p);
	jacobian_to_affine(&p, &a);

	montg_apoint_to_jpoint(&a, &p);
	montg_jpoint_to_apoint(&p, r.x.v, r.y.v);

	if (equ_to_AFPoint(&r, &a))
	{
		puts("Equal");
	}
	else
	{
		puts("Not Equal");
	}

}

static void test_montg_double()
{
	AFPoint a, r1, r2;
	JPoint p, w1, w2;
	u32 k = { 0xaed66ce184be2329, 0xebe9bbf1f1499052, 0x993e0c873cdba6b3, 0xde47b7061c0d5e24 };

	get_random_u32_in_mod_n(&k);
	ML_mul_basepoint(&k, &p);
	jacobian_to_affine(&p, &a);

	double_JPoint(&p, &w1);
	jacobian_to_affine(&w1, &r1);

	montg_apoint_to_jpoint(&a, &p);
	montg_double_jpoint(&p, &w2);
	montg_jpoint_to_apoint(&w2, r2.x.v, r2.y.v);

	if (equ_to_AFPoint(&r1, &r2))
	{
		puts("Equal");
	}
	else
	{
		puts("Not Equal");
	}
}

static void test_montg_add()
{
	AFPoint a, b, r1, r2;
	JPoint p, q, w1, w2;
	u32 k = { 0xaed66ce184be2329, 0xebe9bbf1f1499052, 0x993e0c873cdba6b3, 0xde47b7061c0d5e24 };

	get_random_u32_in_mod_n(&k);
	ML_mul_basepoint(&k, &p);
	jacobian_to_affine(&p, &a);
	get_random_u32_in_mod_n(&k);
	ML_mul_basepoint(&k, &q);
	jacobian_to_affine(&q, &b);

	add_JPoint(&p, &q, &w1);
	jacobian_to_affine(&w1, &r1);

	montg_apoint_to_jpoint(&a, &p);
	montg_apoint_to_jpoint(&b, &q);
	montg_add_jpoint(&p, &q, &w2);
	montg_jpoint_to_apoint(&w2, r2.x.v, r2.y.v);

	if (equ_to_AFPoint(&r2, &r1))
	{
		puts("Equal");
	}
	else
	{
		puts("Not Equal");
	}
}

static void test_montg_point_mult()
{
	AFPoint a, r1, r2;
	JPoint p, w1, w2, t;
	u32 k = { 0xaed66ce184be2329, 0xebe9bbf1f1499052, 0x993e0c873cdba6b3, 0xde47b7061c0d5e24 };
	int i = 0, loop = 100, flag = 0;;

	get_random_u32_in_mod_n(&k);
	ML_mul_basepoint(&k, &p);
	jacobian_to_affine(&p, &a);


	times_point_naf_w5_all_jpoint(&a, &k, &w1);
	jacobian_to_affine(&w1, &r1);

	montg_times_point_naf_w5_all_jpoint(&a, &k, &w2);
	montg_jpoint_to_apoint(&w2, r2.x.v, r2.y.v);

	if (equ_to_AFPoint(&r2, &r1))
	{
		puts("Equal");
	}
	else
	{
		puts("Not Equal");
		return;
	}

	loop = 1000;
	for (i = 0; i < loop; i++)
	{
		get_random_u32_in_mod_n(&k);
		ML_mul_basepoint(&k, &p);
		jacobian_to_affine(&p, &a);

		times_point_naf_w5_all_jpoint(&a, &k, &w1);
		jacobian_to_affine(&w1, &r1);

		montg_times_point_naf_w5_all_jpoint(&a, &k, &w2);
		montg_jpoint_to_apoint(&w2, r2.x.v, r2.y.v);

		if (!equ_to_AFPoint(&r2, &r1))
		{
			flag = 1;
			printf("Not Equal at %d .\n", i);

			puts("k = ");
			print_u32(&k);

			puts("r1 = ");
			print_AFPoint(&r1);

			puts("r2 = ");
			print_AFPoint(&r2);

			break;
		}

	}

	if (flag == 0)
	{
		puts("Self test passed!");
	}
}

static void test_montg_point_mult2()
{
	AFPoint a, r1, r2, r3;
	JPoint p, w1, w2, w3, t;
	u32 k = { 0xaed66ce184be2329, 0xebe9bbf1f1499052, 0x993e0c873cdba6b3, 0xde47b7061c0d5e24 };
	int i = 0, loop = 100, flag = 0;

	get_random_u32_in_mod_n(&k);
	ML_mul_basepoint(&k, &p);
	jacobian_to_affine(&p, &a);


	times_point_naf_w3(&a, &k, &w1);
	jacobian_to_affine(&w1, &r1);

	times_point_naf_w5_all_jpoint(&a, &k, &w3);
	jacobian_to_affine(&w3, &r3);

	montg_times_point_naf_w3(&a, &k, &w2);
	montg_jpoint_to_apoint(&w2, r2.x.v, r2.y.v);

	if (equ_to_AFPoint(&r2, &r1))
	{
		puts("Equal");
	}
	else
	{
		puts("Not Equal");
		return;
	}

	loop = 1000;
	for (i = 0; i < loop; i++)
	{
		get_random_u32_in_mod_n(&k);
		ML_mul_basepoint(&k, &p);
		jacobian_to_affine(&p, &a);

		times_point_naf_w3(&a, &k, &w1);
		jacobian_to_affine(&w1, &r1);

		montg_times_point_naf_w3(&a, &k, &w2);
		montg_jpoint_to_apoint(&w2, r2.x.v, r2.y.v);

		if (!equ_to_AFPoint(&r2, &r1))
		{
			flag = 1;
			printf("Not Equal at %d .\n", i);

			puts("k = ");
			print_u32(&k);

			puts("r1 = ");
			print_AFPoint(&r1);

			puts("r2 = ");
			print_AFPoint(&r2);

			break;
		}

	}

	if (flag == 0)
	{
		puts("Self test passed!");
	}
}

static void test_montg_jpoint_add_apoint()
{
	AFPoint a, b, r1, r2, t;
	JPoint p, q, w1, w2;
	u32 k = { 0xaed66ce184be2329, 0xebe9bbf1f1499052, 0x993e0c873cdba6b3, 0xde47b7061c0d5e24 };

	get_random_u32_in_mod_n(&k);
	ML_mul_basepoint(&k, &p);
	jacobian_to_affine(&p, &a);
	get_random_u32_in_mod_n(&k);
	ML_mul_basepoint(&k, &q);
	jacobian_to_affine(&q, &b);

	affine_to_jacobian(&a, &p);
	add_JPoint_and_AFPoint(&p, &b, &w1);
	jacobian_to_affine(&w1, &r1);

	montg_apoint_to_jpoint(&a, &p);
	montg_apoint_to_montg(&b, &t);
	montg_add_jpoint_and_apoint(&p, &t, &w2);
	montg_jpoint_to_apoint(&w2, r2.x.v, r2.y.v);

	if (equ_to_AFPoint(&r1, &r2))
	{
		puts("Equal");
	}
	else
	{
		puts("Not Equal");
	}
}

static void test_montg_naive_point_mul()
{
	AFPoint a,b,c, r1, r2;
	JPoint p, w1, w2, t, z;
	u32 k = { 0xaed66ce184be2329, 0xebe9bbf1f1499052, 0x993e0c873cdba6b3, 0xde47b7061c0d5e24 };
	int i = 0, loop = 100, flag = 0;;

	//get_random_u32_in_mod_n(&k);
	ML_mul_basepoint(&k, &p);
	jacobian_to_affine(&p, &a);

	// k = 2;
	memset(&k, 0, sizeof(k));
	k.v[0] = 2;

	times_point_naf_w5_all_jpoint(&a, &k, &w1);
	jacobian_to_affine(&w1, &r1);

	montg_apoint_to_montg(&a, &c);
	montg_naive_times_point(&c, &k, &w2);
	montg_jpoint_to_apoint(&w2, r2.x.v, r2.y.v);

	// This part has some problem for mix point addition.
	montg_apoint_to_montg(&a, &c);
	montg_set_jpoint_to_zero(&z);
	montg_add_jpoint_and_apoint(&z, &c, &z);
	montg_double_jpoint(&z, &z);
	montg_jpoint_to_apoint(&z, c.x.v, c.y.v);

	montg_apoint_to_jpoint(&a, &t);
	montg_set_jpoint_to_zero(&z);
	montg_add_jpoint(&z, &t, &z);
	montg_double_jpoint(&z, &z);
	montg_jpoint_to_apoint(&z, b.x.v, b.y.v);

	if (equ_to_AFPoint(&r2, &r1))
	{
		puts("Equal");
	}
	else
	{
		puts("Not Equal");
		return;
	}

	loop = 1000;
	for (i = 0; i < loop; i++)
	{
		get_random_u32_in_mod_n(&k);
		ML_mul_basepoint(&k, &p);
		jacobian_to_affine(&p, &a);

		times_point_naf_w5_all_jpoint(&a, &k, &w1);
		jacobian_to_affine(&w1, &r1);

		montg_apoint_to_montg(&a, &b);
		montg_naive_times_point(&b, &k, &w2);
		montg_jpoint_to_apoint(&w2, r2.x.v, r2.y.v);

		if (!equ_to_AFPoint(&r2, &r1))
		{
			flag = 1;
			printf("Not Equal at %d .\n", i);

			puts("k = ");
			print_u32(&k);

			puts("r1 = ");
			print_AFPoint(&r1);

			puts("r2 = ");
			print_AFPoint(&r2);

			break;
		}

	}

	if (flag == 0)
	{
		puts("Self test passed!");
	}
}

static void test_montg_times_base_point()
{
	AFPoint a, b, r1, r2, r3, r4, r5, t, r6;
	JPoint p, q, w1, w2, w3, w4, w5,zero, w6;
	u32 k = { 0xaed66ce184be2329, 0xebe9bbf1f1499052, 0x993e0c873cdba6b3, 0xde47b7061c0d5e24 };
	int i = 0, loop = 100, flag = 0;;

	// k = 2;
	//memset(&k, 0, sizeof(k));
	//k.v[0] = 2;

	k.v[3] = 0;
	k.v[2] = 0;
	k.v[1] = 0;
	k.v[0] = 0xde47b7061c0d5e2;

	//get_random_u32_in_mod_n(&k);
	ML_mul_basepoint(&k, &w1);
	jacobian_to_affine(&w1, &r1);
	
	montg_times_base_point(&k, &w2);
	montg_jpoint_to_apoint(&w2, r2.x.v, r2.y.v);

	montg_times_base_point2(&k, &w3);
	montg_jpoint_to_apoint(&w3, r3.x.v, r3.y.v);

	//montg_apoint_to_jpoint(&SM2_G, &p);
	//montg_double_jpoint(&p, &w4);
	//montg_jpoint_to_apoint(&w4, r4.x.v, r4.y.v);

	montg_apoint_to_montg(&SM2_G, &t);
	montg_naive_times_point(&t, &k, &w5);
	montg_jpoint_to_apoint(&w5, r5.x.v, r5.y.v);

	//CopyAFPoint(g_montg_AFTable_for_base_point_mul[0][0] + 2, &t);
	//montg_apoint_to_jpoint2(&t, &w6);
	//montg_jpoint_to_apoint(&w6, r6.x.v, r6.y.v);

	if (equ_to_AFPoint(&r1, &r2))
	{
		puts("Equal");
	}
	else
	{
		puts("Not Equal");
		return;
	}

	loop = 1000;
	for (i = 0; i < loop; i++)
	{
		get_random_u32_in_mod_n(&k);

		ML_mul_basepoint(&k, &w1);
		jacobian_to_affine(&w1, &r1);

		montg_times_base_point(&k, &w2);
		montg_jpoint_to_apoint(&w2, r2.x.v, r2.y.v);

		if (!equ_to_AFPoint(&r2, &r1))
		{
			flag = 1;
			printf("Not Equal at %d .\n", i);

			puts("k = ");
			print_u32(&k);

			puts("r1 = ");
			print_AFPoint(&r1);

			puts("r2 = ");
			print_AFPoint(&r2);

			break;
		}

	}

	if (flag == 0)
	{
		puts("Self test passed!");
	}
}

static void test_montg_double_and_add()
{
	AFPoint a, b, r1, r2, r3, r4;
	JPoint p, q, w1, w2, w3, w4;
	u32 k = { 0xaed66ce184be2329, 0xebe9bbf1f1499052, 0x993e0c873cdba6b3, 0xde47b7061c0d5e24 };

	get_random_u32_in_mod_n(&k);
	ML_mul_basepoint(&k, &p);
	jacobian_to_affine(&p, &a);
	get_random_u32_in_mod_n(&k);
	ML_mul_basepoint(&k, &q);
	jacobian_to_affine(&q, &b);

	double_JPoint(&p, &w1);
	jacobian_to_affine(&w1, &r1);
	add_JPoint(&w1, &q, &w3);
	jacobian_to_affine(&w3, &r3);

	montg_apoint_to_jpoint(&a, &p);
	montg_double_jpoint(&p, &w2);
	montg_jpoint_to_apoint(&w2, r2.x.v, r2.y.v);
	if (equ_to_AFPoint(&r1, &r2))
	{
		puts("Equal");
	}
	else
	{
		puts("Not Equal");
	}

	montg_apoint_to_jpoint(&b, &q);
	montg_add_jpoint(&w2, &q, &w4);
	montg_jpoint_to_apoint(&w4, r4.x.v, r4.y.v);
	if (equ_to_AFPoint(&r3, &r4))
	{
		puts("Equal");
	}
	else
	{
		puts("Not Equal");
	}
}

static void test_montg_double_and_add2()
{
	AFPoint a, b, r1, r2, r3, r4, t;
	JPoint p, q, w1, w2, w3, w4;
	u32 k = { 0xaed66ce184be2329, 0xebe9bbf1f1499052, 0x993e0c873cdba6b3, 0xde47b7061c0d5e24 };

	get_random_u32_in_mod_n(&k);
	ML_mul_basepoint(&k, &p);
	jacobian_to_affine(&p, &a);
	get_random_u32_in_mod_n(&k);
	ML_mul_basepoint(&k, &q);
	jacobian_to_affine(&q, &b);

	double_JPoint(&p, &w1);
	jacobian_to_affine(&w1, &r1);
	add_JPoint(&w1, &q, &w3);
	jacobian_to_affine(&w3, &r3);

	montg_apoint_to_jpoint(&a, &p);
	montg_double_jpoint(&p, &w2);
	montg_jpoint_to_apoint(&w2, r2.x.v, r2.y.v);
	if (equ_to_AFPoint(&r1, &r2))
	{
		puts("Equal");
	}
	else
	{
		puts("Not Equal");
	}

	//montg_apoint_to_jpoint(&b, &q);
	//montg_add_jpoint(&w2, &q, &w4);
	montg_apoint_to_montg(&b, &t);
	montg_add_jpoint_and_apoint(&w2, &t, &w4);
	montg_jpoint_to_apoint(&w4, r4.x.v, r4.y.v);
	if (equ_to_AFPoint(&r3, &r4))
	{
		puts("Equal");
	}
	else
	{
		puts("Not Equal");
	}
}

static void test_montg_mix_point_op()
{
	AFPoint t1, t2, a, b, r1, r2, r3, r4;
	JPoint p, q, w1, w2, w3, w4;
	u32 k = { 0xaed66ce184be2329, 0xebe9bbf1f1499052, 0x993e0c873cdba6b3, 0xde47b7061c0d5e24 };

	get_random_u32_in_mod_n(&k);
	ML_mul_basepoint(&k, &p);
	jacobian_to_affine(&p, &a);
	get_random_u32_in_mod_n(&k);
	ML_mul_basepoint(&k, &q);
	jacobian_to_affine(&q, &b);

	affine_to_jacobian(&a, &p);
	double_JPoint(&p, &w3);
	jacobian_to_affine(&w3, &r3);

	montg_apoint_to_jpoint(&a, &p);
	montg_double_jpoint(&p, &w4);
	montg_jpoint_to_apoint(&w4, r4.x.v, r4.y.v);

	if (!equ_to_AFPoint(&r3, &r4))
	{
		puts("Not Equal r3 and r4.");
	}

	add_JPoint_and_AFPoint(&w3, &b, &w1);
	jacobian_to_affine(&w1, &r1);

	montg_apoint_to_montg(&b, &t2);
	montg_add_jpoint_and_apoint(&w4, &t2, &w2);
	montg_jpoint_to_apoint(&w2, r2.x.v, r2.y.v);

	if (equ_to_AFPoint(&r1, &r2))
	{
		puts("Equal");
	}
	else
	{
		puts("Not Equal");
	}
}

static void test_montg_mix_point_op2()
{
	// r = 2a + b + a
	AFPoint t1, t2, a, b, r1, r2, r3, r4;
	JPoint p, q, w1, w2, w3, w4;
	u32 k = { 0xaed66ce184be2329, 0xebe9bbf1f1499052, 0x993e0c873cdba6b3, 0xde47b7061c0d5e24 };

	get_random_u32_in_mod_n(&k);
	ML_mul_basepoint(&k, &p);
	jacobian_to_affine(&p, &a);
	get_random_u32_in_mod_n(&k);
	ML_mul_basepoint(&k, &q);
	jacobian_to_affine(&q, &b);

	affine_to_jacobian(&a, &p);
	double_JPoint(&p, &q);
	add_JPoint_and_AFPoint(&q, &b, &q);
	add_JPoint(&q, &p, &w1);
	jacobian_to_affine(&w1, &r1);

	montg_apoint_to_jpoint(&a, &p);
	montg_double_jpoint(&p, &q);
	montg_apoint_to_montg(&b, &t2);
	montg_add_jpoint_and_apoint(&q, &t2, &q);
	montg_add_jpoint(&q, &p, &w2);
	montg_jpoint_to_apoint(&w2, r2.x.v, r2.y.v);

	if (equ_to_AFPoint(&r1, &r2))
	{
		puts("Equal");
	}
	else
	{
		puts("Not Equal");
	}
}

static void test_montg_P()
{
	UINT64 a[4], r[4];

	montg_to_mod_p(SM2_P.v, r);

	print_u32(r);

}

static void test_montg_precompute()
{
	AFPoint PT[4] = { 0 };
	AFPoint NT[4] = { 0 };

	AFPoint a, r1, r2, r3;
	JPoint p, q, w1, w2, w3, t;
	u32 k = { 0xaed66ce184be2329, 0xebe9bbf1f1499052, 0x993e0c873cdba6b3, 0xde47b7061c0d5e24 };

	//get_random_u32_in_mod_n(&k);
	ML_mul_basepoint(&k, &p);
	jacobian_to_affine(&p, &a);

	montg_pre_compute_naf_w3(&a, PT, NT);

	montg_apoint_to_jpoint(&a, &p);
	montg_double_jpoint(&p, &q);
	montg_add_jpoint(&p, &q, &p);  // p = 3a

	montg_jpoint_to_apoint(&p, r1.x.v, r1.y.v);
	montg_apoint_to_montg(&r1, &r1);
	if (equ_to_AFPoint(PT + 3, &r1))
	{
		puts("Equal");
	}
	else
	{
		puts("Not Equal");
	}
}


//extern void montg_double_jpoint_ex(UINT64 a[3*4], UINT64 r[3*4]);
static void test_ecc_as_jpoint_double()
{
	//UINT64 a[3 * 4];
	//UINT64 r[3 * 4];
	//montg_double_jpoint_ex(a, r);

	AFPoint a, r1, r2, r3;
	JPoint p, w1, w2, w3;
	//u32 k = { 0xaed66ce184be2329, 0xebe9bbf1f1499052, 0x993e0c873cdba6b3, 0xde47b7061c0d5e24 };
	u32 k = { 0x908504D64C6F300A, 0x85F7E4B43C7095B8, 0x7CC5323FC8D8F978, 0x0B3BFD9C66ACDAF5 };
	int i = 0, loop = 100, flag = 0;


	//get_random_u32_in_mod_n(&k);
	ML_mul_basepoint(&k, &p);
	jacobian_to_affine(&p, &a);

	double_JPoint(&p, &w1);
	jacobian_to_affine(&w1, &r1);

	montg_apoint_to_jpoint(&a, &p);
	montg_double_jpoint(&p, &w3);
	montg_jpoint_to_apoint(&w3, r3.x.v, r3.y.v);

	montg_apoint_to_jpoint(&a, &p);
	montg_double_jpoint_ex(&p, &w2);
	montg_jpoint_to_apoint(&w2, r2.x.v, r2.y.v);

	if (equ_to_AFPoint(&r1, &r2))
	{
		puts("Equal");
	}
	else
	{
		puts("Not Equal");
		return;
	}


	loop = 1000;
	for (i = 0; i < loop; i++)
	{
		get_random_u32_in_mod_n(&k);
		ML_mul_basepoint(&k, &p);
		jacobian_to_affine(&p, &a);

		double_JPoint(&p, &w1);
		jacobian_to_affine(&w1, &r1);

		montg_apoint_to_jpoint(&a, &p);
		montg_double_jpoint_ex(&p, &w2);
		montg_jpoint_to_apoint(&w2, r2.x.v, r2.y.v);

		if (!equ_to_AFPoint(&r2, &r1))
		{
			flag = 1;
			printf("Not Equal at %d .\n", i);

			puts("k = ");
			print_u32(&k);

			puts("r1 = ");
			print_AFPoint(&r1);

			puts("r2 = ");
			print_AFPoint(&r2);

			break;
		}

	}

	if (flag == 0)
	{
		puts("Self test passed!");
	}
}

static void test_ecc_as_mpoint_add()
{
	AFPoint a, b, r1, r2, t;
	JPoint p, q, w1, w2;
	u32 k = { 0xaed66ce184be2329, 0xebe9bbf1f1499052, 0x993e0c873cdba6b3, 0xde47b7061c0d5e24 };
	int i = 0, loop = 100, flag = 0;

	get_random_u32_in_mod_n(&k);
	ML_mul_basepoint(&k, &p);
	jacobian_to_affine(&p, &a);
	get_random_u32_in_mod_n(&k);
	ML_mul_basepoint(&k, &q);
	jacobian_to_affine(&q, &b);

	affine_to_jacobian(&a, &p);
	add_JPoint_and_AFPoint(&p, &b, &w1);
	jacobian_to_affine(&w1, &r1);

	montg_apoint_to_jpoint(&a, &p);
	montg_apoint_to_montg(&b, &t);
	//montg_add_jpoint_and_apoint(&p, &t, &w2);
	montg_add_jpoint_and_apoint_ex(&p, &t, &w2);
	montg_jpoint_to_apoint(&w2, r2.x.v, r2.y.v);

	if (equ_to_AFPoint(&r1, &r2))
	{
		puts("Equal");
	}
	else
	{
		puts("Not Equal");
		return;
	}

	loop = 1000;
	for (i = 0; i < loop; i++)
	{
		get_random_u32_in_mod_n(&k);
		ML_mul_basepoint(&k, &p);
		jacobian_to_affine(&p, &a);
		get_random_u32_in_mod_n(&k);
		ML_mul_basepoint(&k, &q);
		jacobian_to_affine(&q, &b);

		affine_to_jacobian(&a, &p);
		add_JPoint_and_AFPoint(&p, &b, &w1);
		jacobian_to_affine(&w1, &r1);

		montg_apoint_to_jpoint(&a, &p);
		montg_apoint_to_montg(&b, &t);
		montg_add_jpoint_and_apoint_ex(&p, &t, &w2);
		montg_jpoint_to_apoint(&w2, r2.x.v, r2.y.v);

		if (!equ_to_AFPoint(&r2, &r1))
		{
			flag = 1;
			printf("Not Equal at %d .\n", i);

			puts("k = ");
			print_u32(&k);

			puts("r1 = ");
			print_AFPoint(&r1);

			puts("r2 = ");
			print_AFPoint(&r2);

			break;
		}

	}

	if (flag == 0)
	{
		puts("Self test passed!");
	}
}

static void test_ecc_as_mpoint_add2()
{
	AFPoint a, b, r1, r2, t;
	JPoint p, q, w1, w2;
	u32 k = { 0xaed66ce184be2329, 0xebe9bbf1f1499052, 0x993e0c873cdba6b3, 0xde47b7061c0d5e24 };
	int i = 0, loop = 100, flag = 0;

	get_random_u32_in_mod_n(&k);
	ML_mul_basepoint(&k, &p);
	jacobian_to_affine(&p, &a);
	get_random_u32_in_mod_n(&k);
	ML_mul_basepoint(&k, &q);
	jacobian_to_affine(&q, &b);

	//  JPoint is zero test
	montg_set_jpoint_to_zero(&p);
	montg_apoint_to_montg(&b, &t);
	montg_add_jpoint_and_apoint(&p, &t, &w1);
	montg_jpoint_to_apoint(&w1, r1.x.v, r1.y.v);

	montg_set_jpoint_to_zero(&p);
	montg_apoint_to_montg(&b, &t);
	montg_add_jpoint_and_apoint_ex(&p, &t, &w2);
	montg_jpoint_to_apoint(&w2, r2.x.v, r2.y.v);

	if (equ_to_AFPoint(&r1, &r2))
	{
		puts("Equal");
	}
	else
	{
		puts("Not Equal");
	}

	// APoint is zero test
	montg_apoint_to_jpoint(&a, &p);
	memset(&t, 0 , sizeof(t));
	montg_add_jpoint_and_apoint(&p, &t, &w1);
	montg_jpoint_to_apoint(&w1, r1.x.v, r1.y.v);

	montg_apoint_to_jpoint(&a, &p);
	memset(&t, 0, sizeof(t));
	montg_add_jpoint_and_apoint_ex(&p, &t, &w2);
	montg_jpoint_to_apoint(&w2, r2.x.v, r2.y.v);

	if (equ_to_AFPoint(&r1, &r2))
	{
		puts("Equal");
	}
	else
	{
		puts("Not Equal");
		return;
	}
}

static void test_ecc_as_jpoint_add()
{
	AFPoint a, b, r1, r2, r3;
	JPoint p, q, w1, w2, w3;
	u32 k = { 0xaed66ce184be2329, 0xebe9bbf1f1499052, 0x993e0c873cdba6b3, 0xde47b7061c0d5e24 };
	int i = 0, loop = 100, flag = 0;

	get_random_u32_in_mod_n(&k);
	ML_mul_basepoint(&k, &p);
	jacobian_to_affine(&p, &a);
	get_random_u32_in_mod_n(&k);
	ML_mul_basepoint(&k, &q);
	jacobian_to_affine(&q, &b);

	affine_to_jacobian(&a, &p);
	add_JPoint_and_AFPoint(&p, &b, &w1);
	jacobian_to_affine(&w1, &r1);

	montg_apoint_to_jpoint(&a, &p);
	montg_apoint_to_jpoint(&b, &q);
	montg_add_jpoint_ex(&p, &q, &w2);
	montg_jpoint_to_apoint(&w2, r2.x.v, r2.y.v);

	montg_apoint_to_jpoint(&a, &p);
	montg_apoint_to_jpoint(&b, &q);
	montg_add_jpoint(&p, &q, &w3);
	montg_jpoint_to_apoint(&w3, r3.x.v, r3.y.v);

	if (equ_to_AFPoint(&r1, &r2))
	{
		puts("Equal");
	}
	else
	{
		puts("Not Equal");
		return;
	}

	loop = 1000;
	for (i = 0; i < loop; i++)
	{
		get_random_u32_in_mod_n(&k);
		ML_mul_basepoint(&k, &p);
		jacobian_to_affine(&p, &a);
		get_random_u32_in_mod_n(&k);
		ML_mul_basepoint(&k, &q);
		jacobian_to_affine(&q, &b);

		affine_to_jacobian(&a, &p);
		add_JPoint_and_AFPoint(&p, &b, &w1);
		jacobian_to_affine(&w1, &r1);

		montg_apoint_to_jpoint(&a, &p);
		montg_apoint_to_jpoint(&b, &q);
		montg_add_jpoint_ex(&p, &q, &w2);
		montg_jpoint_to_apoint(&w2, r2.x.v, r2.y.v);

		if (!equ_to_AFPoint(&r2, &r1))
		{
			flag = 1;
			printf("Not Equal at %d .\n", i);

			puts("k = ");
			print_u32(&k);

			puts("r1 = ");
			print_AFPoint(&r1);

			puts("r2 = ");
			print_AFPoint(&r2);

			break;
		}

	}

	if (flag == 0)
	{
		puts("Self test passed!");
	}
}

static void test_ecc_as_jpoint_add2()
{
	AFPoint a, b, r1, r2, r3;
	JPoint p, q, w1, w2, w3;
	u32 k = { 0xaed66ce184be2329, 0xebe9bbf1f1499052, 0x993e0c873cdba6b3, 0xde47b7061c0d5e24 };
	int i = 0, loop = 100, flag = 0;

	get_random_u32_in_mod_n(&k);
	ML_mul_basepoint(&k, &p);
	jacobian_to_affine(&p, &b);
	memset(&a, 0, sizeof(a));

	affine_to_jacobian(&a, &p);
	add_JPoint_and_AFPoint(&p, &b, &w1);
	jacobian_to_affine(&w1, &r1);

	montg_apoint_to_jpoint(&a, &p);
	montg_apoint_to_jpoint(&b, &q);
	montg_add_jpoint_ex(&p, &q, &w2);
	montg_jpoint_to_apoint(&w2, r2.x.v, r2.y.v);

	montg_apoint_to_jpoint(&a, &p);
	montg_apoint_to_jpoint(&b, &q);
	montg_add_jpoint(&p, &q, &w3);
	montg_jpoint_to_apoint(&w3, r3.x.v, r3.y.v);

	if (equ_to_AFPoint(&r1, &r2))
	{
		puts("Equal");
	}
	else
	{
		puts("Not Equal");
	}

	get_random_u32_in_mod_n(&k);
	ML_mul_basepoint(&k, &p);
	jacobian_to_affine(&p, &a);
	memset(&b, 0, sizeof(b));

	affine_to_jacobian(&a, &p);
	add_JPoint_and_AFPoint(&p, &b, &w1);
	jacobian_to_affine(&w1, &r1);

	montg_apoint_to_jpoint(&a, &p);
	montg_apoint_to_jpoint(&b, &q);
	montg_add_jpoint_ex(&p, &q, &w2);
	montg_jpoint_to_apoint(&w2, r2.x.v, r2.y.v);

	montg_apoint_to_jpoint(&a, &p);
	montg_apoint_to_jpoint(&b, &q);
	montg_add_jpoint(&p, &q, &w3);
	montg_jpoint_to_apoint(&w3, r3.x.v, r3.y.v);

	if (equ_to_AFPoint(&r1, &r2))
	{
		puts("Equal");
	}
	else
	{
		puts("Not Equal");
	}
}

static void test_ecc_as_jpoint_add3()
{
	AFPoint a, b, r1, r2, r3;
	JPoint w1, w2, w3;

	JPoint p = { 
		{ 0xDFFF50E9EFF82987, 0xEEF7C0BE84FB0129, 0x1FB0F9085EAE6516, 0x0781A1B0698A7DB4, },
		{ 0x8A3F856FBF9B1756, 0xB40DA7EEE43F8783, 0xFF49FF77D90A0B0E, 0x1F6BEF6CDDF51DF3, },
		{ 0xC0FFCC84D8EECBFB, 0x86AE5E822E0696CC, 0xDDAF389F38F43BD8, 0x57282D671D388759, },
	};
	JPoint q = { 
		{ 0xFF3F1A39FF20BA48, 0x1439C60523BFD134, 0xEA1D1AED81FA12FB, 0xCC79521AA1EFBD36, },
		{ 0xA94D4E26FE7CD08D, 0xE7F0EACD94D2E4A7, 0xCF1E7D01C7248196, 0x1D85D2A22B8CF4AA, },
		{ 0xD34334CF3FA89584, 0x6E1C36FD5F3C7DC0, 0xB611E83483BA28D7, 0xC16B63F09D96DB45, },
	};

	montg_add_jpoint_ex(&p, &q, &w2);
	montg_jpoint_to_apoint(&w2, r2.x.v, r2.y.v);

	montg_add_jpoint(&p, &q, &w3);
	montg_jpoint_to_apoint(&w3, r3.x.v, r3.y.v);

	if (equ_to_AFPoint(&r2, &r3))
	{
		puts("Equal");
	}
	else
	{
		puts("Not Equal");
		return;
	}

}

static void test_ecc_as_inv_mod_p()
{
	UINT64 i = 0, flag = 0, loop = 0;
	UINT64 a[4] = { 0xccc7ece6295cb3ee, 0x9f76070d503884f0, 0x146f9bbd20642ccd, 0x87e0373e5b0004eb };
	UINT64 b[4];
	UINT64 r1[4];
	UINT64 r2[4];
	UINT64 r3[4];
	UINT64 real[4] = { 0xd49e2992ee22def0, 0xa9345f32363e0040, 0xcb1fbe85cc35e75f, 0xb5a88d7b2d96ac0c };
	// k = 0x161
	// Before montg_mul: 
	// pr = 0xc67e73115cbdf01c, 0x72853aec0e932adb, 0xd7cd1b780469ab3f, 0xf529bedb5268be65
	// t = 0000000000000000 0000000000000000 0000000080000000 0000000000000000

	srand(time(NULL));

	// Use ourself method to calculate the montgomery inversion
	inv_for_mul(a, b, &SM2_P, &SM2_rhoP);
	montg_to_mod_p(b, r1);  // b = b * 2^256, b Should equal to real[]

	// Use C version 
	MontgInvModp(a, r3);

	// Use Assembly version
	montg_inv_mod_p_ex(a, r2);  // b = a^(-1) * 2^(256) mod(p)

	if (memcmp(real, r2, 32) != 0)
	{
		puts("Not equal!");

		puts("a = ");
		print_u32(a);

		puts("r1 = ");
		print_u32(r1);

		puts("b = ");
		print_u32(b);

		puts("r2 = ");
		print_u32(r2);

		return;
	}
	else
	{
		puts("Equal!");
	}

	loop = 100000;
	for (i = 0; i < loop; i++)
	{
		get_random_u32_in_mod_p(&a);
		
		// Use C version 
		MontgInvModp(a, r3);

		// Use Assembly version
		montg_inv_mod_p_ex(a, r2);  // b = a^(-1) * 2^(256) mod(p)

		if (memcmp(r3, r2, 32) != 0)
		{
			printf("Not Equal at %d .\n", i);

			puts("a = ");
			print_u32(a);

			puts("r1 = ");
			print_u32(r1);

			puts("b = ");
			print_u32(b);

			puts("r2 = ");
			print_u32(r2);

			flag = 1;
			break;
		}
	}

	if (flag == 0)
	{
		puts("Self test passed!");
	}

}

static void test_ecc_as_inv_mod_n()
{
	UINT64 i = 0, flag = 0, loop = 0;
	UINT64 a[4] = { 0x50358362A399DADD, 0xAE2359C7E41B521B, 0xD505051020160706, 0x0D44E44AD3E05FA7 };
	UINT64 r1[4];
	UINT64 r2[4];
	UINT64 r3[4];
	UINT64 real[4] = { 0 };

	srand(time(NULL));

	//get_random_u32_in_mod_n(&a);

	// Use C version 
	MontgInvModn(a, r1);

	// Use Assembly version
	montg_inv_mod_n_ex(a, r2);  // b = a^(-1) * 2^(256) mod(n)

	if (memcmp(r1, r2, 32) != 0)
	{
		puts("Not equal!");

		puts("a = ");
		print_u32(a);

		puts("r1 = ");
		print_u32(r1);

		puts("r2 = ");
		print_u32(r2);

		return;
	}
	else
	{
		puts("Equal!");
	}

	loop = 100000;
	for (i = 0; i < loop; i++)
	{
		get_random_u32_in_mod_n(&a);

		// Use C version 
		MontgInvModn(a, r3);

		// Use Assembly version
		montg_inv_mod_n_ex(a, r2);  // b = a^(-1) * 2^(256) mod(n)

		if (memcmp(r3, r2, 32) != 0)
		{
			printf("Not Equal at %d .\n", i);

			puts("a = ");
			print_u32(a);

			puts("r1 = ");
			print_u32(r1);

			puts("r2 = ");
			print_u32(r2);

			flag = 1;
			break;
		}
	}

	if (flag == 0)
	{
		puts("Self test passed!");
	}

}



void self_main_test_montg_op()
{
	srand(time(NULL));

	//test_to_montg();
	//test_montg_conversion();
	//test_montg_double();
	//test_montg_add();
	//test_montg_point_mult();
	//test_montg_point_mult2();
	//test_montg_jpoint_add_apoint();
	//test_montg_double_and_add(); 
	//test_montg_double_and_add2();
	//test_montg_mix_point_op();
	//test_montg_mix_point_op2();

	//test_montg_naive_point_mul();
	//test_montg_times_base_point();
	//test_montg_P();
	//test_montg_precompute();


	//test_ecc_as_jpoint_double();
	//test_ecc_as_mpoint_add();
	//test_ecc_as_mpoint_add2();
	//test_ecc_as_jpoint_add();
	//test_ecc_as_jpoint_add2();
	//test_ecc_as_jpoint_add3();
	//test_ecc_as_inv_mod_p();
	test_ecc_as_inv_mod_n();

}
*/
