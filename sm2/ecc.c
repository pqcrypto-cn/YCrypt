#include "include/ecc.h"

#define ASSIGN_AFFINE_PONIT(point,x,y) point={{(x),(y)}}
#define ASSIGN_JACOBIAN_POINT(point,x,y,z) point={{(x),(y),(z)}}

static AFPoint lowTable[256];
static AFPoint highTable[256];
static AFPoint AF_PTable[8];
static AFPoint AF_neg_PTable[8];

//static AFPoint AF_PTable[16];
//static AFPoint AF_neg_PTable[16];

/// NOTE:
//      To make a function available for inlining across multiple files, you must:
//      place the function in a common header file, for example foo. h.
//      mark the function as extern __inline.
//      #include the header file in each file where the inline function is needed
// inline void CopyAFPoint(AFPoint* src, AFPoint* dst)
// {
// 	memcpy(dst, src, sizeof(AFPoint));
// }
// inline void CopyJPoint(JPoint* src, JPoint* dst)
// {
// 	memcpy(dst, src, sizeof(JPoint));
// }


bool equ_to_AFPoint_one(const AFPoint* point)
{
	return u32_eq_zero(&point->x) && u32_eq_zero(&point->y);
}


bool equ_to_JPoint_one(const JPoint* point)
{
	return u32_eq_one(&point->x) && u32_eq_one(&point->y) && u32_eq_zero(&point->z);
}

bool equ_to_AFPoint(const AFPoint* point1, const AFPoint* point2)
{
	return u32_eq(&point1->x, &point2->x) && u32_eq(&point1->y, &point2->y);
}

bool equ_to_JPoint(const JPoint* point1, const JPoint* point2)
{
	u32 pz, qz, t1, t2;

	pow_mod_p(&point1->z, &pz);
	pow_mod_p(&point2->z, &qz);
	mul_mod_p(&point1->x, &qz, &t1);
	mul_mod_p(&point2->x, &pz, &t2);

	if (!u32_eq(&t1, &t2))
		return false;

	mul_mod_p(&pz, &point1->z, &pz);
	mul_mod_p(&qz, &point2->z, &qz);
	mul_mod_p(&point1->y, &qz, &t1);
	mul_mod_p(&point2->y, &pz, &t2);

	return u32_eq(&t1, &t2);
}

bool is_on_curve(const AFPoint* point)  	// is y^2 = x^3 + ax + b ?
{
	if (equ_to_AFPoint_one(point))
		return true;

	u32 y2, x3, ax, res;
	pow_mod_p(&point->y, &y2);
	pow_mod_p(&point->x, &x3);
	mul_mod_p(&point->x, &x3, &x3);
	mul_mod_p(&SM2_a, &point->x, &ax);
	add_mod_p(&x3, &ax, &res);
	add_mod_p(&res, &SM2_b, &res);

	return u32_eq(&y2, &res);
}

void affine_to_jacobian(const AFPoint* point, JPoint* result)
{
	if (!equ_to_AFPoint_one(point))
	{
		result->x = point->x;
		result->y = point->y;
		result->z.v[0] = 1;
		result->z.v[1] = 0;
		result->z.v[2] = 0;
		result->z.v[3] = 0;
		return;
	}

	*result = JPoint_ZERO;
}

void jacobian_to_affine(const JPoint* point, AFPoint* result)
{
	u32 u, u_square, u_cube;
	if (!u32_eq_zero(&point->z))
	{
		inv_for_mul_mod_p(&point->z, &u);
		mul_mod_p(&u, &u, &u_square);
		mul_mod_p(&u_square, &u, &u_cube);

		mul_mod_p(&point->x, &u_square, &result->x);
		mul_mod_p(&point->y, &u_cube, &result->y);
	}
	else if (equ_to_JPoint_one(point))
	{
		//result = { 0,0 };
		memset(result, 0, sizeof(AFPoint));
	}

}




// Note: this function should
// ALWAYS be called with different point

void add_JPoint_and_AFPoint(const JPoint* point1, const AFPoint* point2, JPoint* result)
{
	if (equ_to_JPoint_one(point1))
	{
		affine_to_jacobian(point2, result);
		return;
	}

	if (equ_to_AFPoint_one(point2))
	{
		*result = *point1;
		return;
	}

	u32 z2, A, B, C2, C3, X1C2, D2, X1C22, tmp;
	pow_mod_p(&point1->z, &z2);
	//print_u32(&z2);
	mul_mod_p(&point2->x, &z2, &A);
	//print_u32(&A);
	mul_mod_p(&point2->y, &z2, &B);
	//print_u32(&B);
	mul_mod_p(&B, &point1->z, &B);
	//print_u32(&B);

	sub_mod_p(&A, &point1->x, &A);
	//print_u32(&A);
	sub_mod_p(&B, &point1->y, &B);
	//print_u32(&B);

	pow_mod_p(&A, &C2);
	//print_u32(&C2);
	// printf("C2 * A: \n");
	// print_u32(&C2);
	// print_u32(&A);
	mul_mod_p(&C2, &A, &C3);
	// printf("C3: \n");
	// print_u32(&C3);
	mul_mod_p(&point1->x, &C2, &X1C2);
	//print_u32(&X1C2);
	pow_mod_p(&B, &D2);
	//print_u32(&D2);

	add_mod_p(&X1C2, &X1C2, &X1C22);
	//print_u32(&X1C22);
	sub_mod_p(&D2, &X1C22, &result->x);
	//print_u32(&result->x);
	sub_mod_p(&result->x, &C3, &result->x);
	//print_u32(&result->x);
	sub_mod_p(&X1C2, &result->x, &tmp);
	//print_u32(&tmp);

	mul_mod_p(&B, &tmp, &B);
	//print_u32(&B);
	mul_mod_p(&point1->y, &C3, &tmp);
	//print_u32(&tmp);
	sub_mod_p(&B, &tmp, &result->y);
	//print_u32(&result->y);

	mul_mod_p(&point1->z, &A, &result->z);
	//print_u32(&result->z);

}


/*
bool is_a_minus_3()
{
	u32 tmp;
	//u32_add(SM2_a, { 3,0,0,0 }, tmp);
	u32 t1 = { 3, 0, 0, 0 };
	u32_add(&SM2_a, &t1, &tmp);
	return u32_eq(&tmp, &SM2_P);
}

#define MINIS
void double_JPoint(const JPoint* pt, JPoint* result)
{
	if (equ_to_JPoint_one(pt))
	{
		*result = *pt;
		return;
	}

	u32 pz2, lambda1, py2, py_2, py2_2, py2_4, lambda2, l2_2, py4_4, lambda3, l1l1, m1, m2;
	pow_mod_p(&pt->z, &pz2);//pz^2
	u32 tmp1, tmp2;


	// if (is_a_minus_3)
	// {
	// 	//if a == -3 then
	// 	//3*x^2 + a*z^4 == 3*x^2 - 3*z^4 == 3*(x^2-z^4) == 3*(x-z^2)*(x+z^2)
	// 	//x-z^2
	// 	sub_mod_p(pt.x, pz2, tmp1);
	// 	//x+z^2
	// 	add_mod_p(pt.x, pz2, tmp2);
	// 	//3*(x-z^2)*(x+z^2)
	// 	mul_mod_p(tmp1, tmp2, tmp1);
	// 	double_mod_p(tmp1, tmp2);
	// 	add_mod_p(tmp1, tmp2, lambda1);
	// }
	// else
	// {
	// 	pow_mod_p(pt.x, px2); // px2 = px^2
	// 	pow_mod_p(pz2, pz4);	// pz4 = pz^4
	// 	double_mod_p(px2, px2_2); // px2_2 = 2px^2
	// 	add_mod_p(px2_2, px2, px2_3); // px2_3 = 3px^2
	// 	mul_mod_p(SM2_a, pz4, lambda1);
	// 	add_mod_p(px2_3, lambda1, lambda1);
	// }


#ifdef MINIS
	//x-z^2
	sub_mod_p(&pt->x, &pz2, &tmp1);
	//x+z^2
	add_mod_p(&pt->x, &pz2, &tmp2);
	//3*(x-z^2)*(x+z^2)
	mul_mod_p(&tmp1, &tmp2, &tmp1);

	//double_mod_p(&tmp1, &tmp2);
	//add_mod_p(&tmp1, &tmp2, &lambda1);

	mul_by_3_mod_p(&tmp1, &lambda1); // *3

#else
	pow_mod_p(pt.x, px2); // px2 = px^2
	pow_mod_p(pz2, pz4);	// pz4 = pz^4
	double_mod_p(px2, px2_2); // px2_2 = 2px^2
	add_mod_p(px2_2, px2, px2_3); // px2_3 = 3px^2
	mul_mod_p(SM2_a, pz4, lambda1);
	add_mod_p(px2_3, lambda1, lambda1);
#endif // MINIS


	// l1 = 3*px^2+a*pz^4
	pow_mod_p(&pt->y, &py2); // py2 = py^2

	double_mod_p(&pt->y, &py_2); // py_2 = 2*py
	double_mod_p(&py2, &py2_2); // py2_2 = 2*py^2
	double_mod_p(&py2_2, &py2_4); // py2_4 = 4*py^2
	mul_mod_p(&py2_4, &pt->x, &lambda2); // l2 = 4*px*py^2

	double_mod_p(&lambda2, &l2_2); // l2 = 2*l2
	pow_mod_p(&py2_2, &py4_4); // py4_4 = 4*py^4
	double_mod_p(&py4_4, &lambda3); // l3 = 8^py^4

	pow_mod_p(&lambda1, &l1l1); // l1l1 = l1^2
	sub_mod_p(&l1l1, &l2_2, &result->x); // x3 = l1^2 - 2*l2
	sub_mod_p(&lambda2, &result->x, &m1); // m1 = l2 - x3

	mul_mod_p(&lambda1, &m1, &m2); // m2 = l1*(l2-x3)
	sub_mod_p(&m2, &lambda3, &result->y); // y = l1*(l2-x3)-l3

	mul_mod_p(&py_2, &pt->z, &result->z); // z = 2*py*pz
}
*/

// Use Intel-IPP version
void double_JPoint(const JPoint* pt, JPoint* result)
{
	u32 S, U, M;
	if (equ_to_JPoint_one(pt))
	{
		*result = *pt;
		return;
	}

	double_mod_p(&(pt->y), &S);  // S = 2Y
	pow_mod_p(&(pt->z), &U);       // U = Z^2

	pow_mod_p(&S, &M);           // M = 4Y^2
	mul_mod_p(&S, &(pt->z), &(result->z));  // rZ = 2YZ

	pow_mod_p(&M, &(result->y));          // rY = 16Y^4

	mul_mod_p(&M, &(pt->x), &S);      // S = 4XY^2
	div_by_2_mod_p(&(result->y), &(result->y)); // rY = 8Y^4

	// Get lambda1
	add_mod_p(&U, &(pt->x), &M);          // M = X+Z^2
	sub_mod_p(&(pt->x), &U, &U);          // U = X-Z^2
	mul_mod_p(&M, &U, &M);                 // N = (X+Z^2)*(X-Z^2) = X^2-Z^4
	mul_by_3_mod_p(&M, &M);              // M = 3*(X^2-Z^4)

	double_mod_p(&S, &U);                // U = 8XY^2
	pow_mod_p(&M, &(result->x));             // rX = M^2
	sub_mod_p(&(result->x), &U, &(result->x));  // rX = M^2-U

	sub_mod_p(&S, &(result->x), &S);      // S = 4XY^2-rX
	mul_mod_p(&S, &M, &S);                 // S = M*(4XY^2 - rX)
	sub_mod_p(&S, &(result->y), &(result->y)); // rY = M(4XY^2-rX) -8Y^4
}


void add_JPoint(const JPoint* point1, const JPoint* point2, JPoint* result)
{
	if (equ_to_JPoint_one(point1))
	{
		*result = *point2;
		return;
	}
	if (equ_to_JPoint_one(point2))
	{
		*result = *point1;
		return;
	}

	u32 pz2, qz2, pz3, qz3, lambda1, lambda2;

	pow_mod_p(&point1->z, &pz2);
	pow_mod_p(&point2->z, &qz2);
	mul_mod_p(&pz2, &point1->z, &pz3);
	mul_mod_p(&qz2, &point2->z, &qz3);
	mul_mod_p(&point1->x, &qz2, &lambda1);
	mul_mod_p(&point2->x, &pz2, &lambda2);


	u32 lambda4, lambda5;
	mul_mod_p(&point1->y, &qz3, &lambda4);
	mul_mod_p(&point2->y, &pz3, &lambda5);
	//P1=P2
	if (u32_eq(&lambda1, &lambda2) && u32_eq(&lambda4, &lambda5))
	{
		double_JPoint(point1, result);
	}
	//P1!=P2
	else
	{
		u32 lambda3, lambda6, lambda7, lambda8, l6l6, l7l3l3, x, lambda9, l9l6, l8l3l3, l8l3l3l3;
		add_mod_p(&lambda4, &lambda5, &lambda8);
		//P1=-P2
		if (u32_eq(&lambda1, &lambda2) && u32_eq_zero(&lambda8))
		{
			*result = JPoint_ZERO;
			return;
		}
		sub_mod_p(&lambda1, &lambda2, &lambda3);
		sub_mod_p(&lambda4, &lambda5, &lambda6);
		add_mod_p(&lambda1, &lambda2, &lambda7);
		pow_mod_p(&lambda6, &l6l6);
		pow_mod_p(&lambda3, &l7l3l3);
		mul_mod_p(&lambda7, &l7l3l3, &l7l3l3);
		sub_mod_p(&l6l6, &l7l3l3, &x);
		add_mod_p(&x, &x, &lambda9);
		sub_mod_p(&l7l3l3, &lambda9, &lambda9);
		mul_mod_p(&lambda9, &lambda6, &l9l6);
		pow_mod_p(&lambda3, &l8l3l3);
		mul_mod_p(&lambda8, &l8l3l3, &l8l3l3);
		mul_mod_p(&l8l3l3, &lambda3, &l8l3l3l3);

		u32 y, z;
		sub_mod_p(&l9l6, &l8l3l3l3, &y);
		mul_mod_p(&y, &SM2_inv2P, &y);
		mul_mod_p(&point1->z, &point2->z, &z);
		mul_mod_p(&z, &lambda3, &z);

		result->x = x;
		result->y = y;
		result->z = z;
	}



}

void JPoint_sub_AFPoint(const JPoint* point1, const AFPoint* point2, JPoint* result)
{
	AFPoint tmp;
	tmp.x = point2->x;
	//point2 + tmp = o
	u32_sub(&SM2_P, &point2->y, &tmp.y);
	add_JPoint_and_AFPoint(point1, &tmp, result);
}

bool is_AFPoint_reciprocal(const AFPoint* point1, const AFPoint* point2)
{
	u32 inversion_y;
	neg_mod_p(&(point2->y), &inversion_y);
	//inv_for_add(&point2->y, &inversion_y, &SM2_P);
	return u32_eq(&point1->x, &point2->x) && u32_eq(&point1->y, &inversion_y);
}

void add_AFPoint(const AFPoint* point1, const AFPoint* point2, AFPoint* result)
{
	if (equ_to_AFPoint_one(point1) || equ_to_AFPoint_one(point2))
	{
		u32_add(&(point1->x), &(point2->x), &(result->x));
		u32_add(&(point1->y), &(point2->y), &(result->y));
	}
	else if (is_AFPoint_reciprocal(point1, point2))
	{
		memset(result, 0, sizeof(AFPoint));
	}
	else
	{
		u32 lambda;
		if (u32_eq(&point1->x, &point2->x))
		{
			u32 x2, tx2, dx, dy, dyi;

			//x2 = x^2
			mul_mod_p(&point1->x, &point1->x, &x2);

			// tx2 = 3x^2
			double_mod_p(&x2, &tx2);
			add_mod_p(&tx2, &x2, &tx2);

			// dx = 3x^2+a;
			add_mod_p(&tx2, &SM2_a, &dx);
			add_mod_p(&point1->y, &point2->y, &dy);
			inv_for_mul_mod_p(&dy, &dyi);
			div_mod_p(&dx, &dy, &lambda);
		}
		else
		{
			u32 s1, s2;
			sub_mod_p(&point2->y, &point1->y, &s1);
			sub_mod_p(&point2->x, &point1->x, &s2);
			div_mod_p(&s1, &s2, &lambda);
		}

		u32 lambda2, tmp1, X, Y;
		pow_mod_p(&lambda, &lambda2);
		add_mod_p(&point1->x, &point2->x, &tmp1);
		sub_mod_p(&lambda2, &tmp1, &X);
		sub_mod_p(&point1->x, &X, &tmp1);
		mul_mod_p(&lambda, &tmp1, &tmp1);
		sub_mod_p(&tmp1, &point1->y, &Y);

		result->x = X;
		result->y = Y;
	}
}

void AFPoint_neg(const AFPoint* point, AFPoint* result)
{
	result->x = point->x;
	u32_sub(&SM2_P, &point->y, &result->y);
	//inv_for_add(point.y, result.y, SM2_P);
}

void JPoint_neg(const JPoint* point, JPoint* result)
{
	result->x = point->x;
	u32_sub(&SM2_P, &point->y, &result->y);
	//inv_for_add(point.y, result.y, SM2_P);
	result->z = point->z;
}

//computing the NAF of a positive integer k, w=5,2**5=32
//need to consider carry
void get_naf(const u32* pk, int naf_k[257])
{
	bool over_flag = 0;
	u32 k;
	memcpy(&k, pk, sizeof(k));
	for (size_t i = 0; i < 257; i++)
	{
		if (k.v[0] & 1) //k is odd
		{
			//k mods (2**5), |k|<(2**4)
			u8 tmp = k.v[0] & 0xF;
			over_flag = 0;
			if (k.v[0] & 0x10)//mod 2^5. the result < 2^4 
			{
				naf_k[i] = 0 - tmp;
				//over_flag = u32_add(k, { tmp,0,0,0 }, k);
				u32 t1 = { tmp, 0, 0, 0 };
				over_flag = u32_add(&k, &t1, &k);
			}
			else
			{
				naf_k[i] = tmp;
				//k->v[0] = __ull_rshift(k->v[0], 5);
				//k->v[0] = __ll_lshift(k->v[0], 5);
				k.v[0] &= 0xFFFFFFFFFFFFFFE0;
			}

		}
		else
		{
			naf_k[i] = 0;
		}
		u32_shr(&k);
		if (over_flag)
		{
			k.v[3] |= 0x8000000000000000;
		}
	}
}


// Computing the NAF of a positive integer k
// w = 2
void get_naf_w2(const u32* pk, int8_t naf_w3[257])
{
	int i = 0;
	int8_t ki;
	u32 x = { 0 }, k;

	memcpy(&k, pk, sizeof(k));
	for (i = 0; i < 257; i++)
	{
		ki = ((int8_t)(k.v[0])) & 0x3;   // k mod (2^2)

		if ((ki & 1) != 0)  // k is odd
		{
			if (ki >= 2) // -2^1 < ki < 2^2
			{
				ki -= 4; // ki = ki - 2^2
			}
			if (ki >= 0)
			{
				x.v[0] = ki;
				u32_sub(&k, &x, &k);
			}
			else  // ki < 0
			{
				x.v[0] = -ki;
				u32_add(&k, &x, &k);
			}
			naf_w3[i] = ki;
		}
		else
		{
			naf_w3[i] = 0;
		}
		u32_shr(&k);   // k = k / 2
	}
}

// Computing the NAF of a positive integer k
// w = 5
void get_naf_w5(const u32* pk, int8_t naf_w5[257])
{
	int i = 0;
	int8_t ki;
	uint8_t t;
	u32 x = { 0 }, k;

	memcpy(&k, pk, sizeof(k));
	static const int8_t ki_table[32] =  // Look up table for ki at w = 5
	{
		0,  1, 0,  3, 0,  5, 0,  7, 0,  9, 0,  11, 0,  13, 0,  15,
		0, -1, 0, -3, 0, -5, 0, -7, 0, -9, 0, -11, 0, -13, 0, -15,
	};

	for (i = 0; i < 257; i++)
	{
		t = ((uint8_t)(k.v[0])) & 0x1F ;  // k mod (2^5)
		ki = ki_table[t];                  // ki = k mod (2^5)
		naf_w5[i] = ki;
		if (ki != 0)  // k is odd
		{
			if (ki > 0)
			{
				k.v[0] -= ki;
			}
			else  // ki < 0
			{
				x.v[0] = -ki; // > 0
				u32_add(&k, &x, &k);
			}
		}
		u32_shr(&k);   // k = k / 2
	}
}

// Computing the NAF of a positive integer k
// w = 5
void get_naf_w5_2(const u32* pk, int8_t naf_w5[257])
{
	int i = 0;
	int8_t ki;
	u32 x = { 0 }, k;

	memcpy(&k, pk, sizeof(k));
	for (i = 0; i < 257; i++)
	{
		ki = ((int8_t)(k.v[0])) & 0x1F;  // k mod (2^5)

		//naf_w5[i] = ki;
		if ((ki & 1) != 0)  // k is odd
		{
			if (ki >= 16) //  -2^4 < ki < 2^4
			{
				ki -= 32; // ki - 2^5
			}
			if (ki >= 0)
			{
				x.v[0] = ki;
				u32_sub(&k, &x, &k);
			}
			else  // ki < 0
			{
				x.v[0] = -ki;
				u32_add(&k, &x, &k);
			}
			naf_w5[i] = ki;
		}
		else
		{
			naf_w5[i] = 0;
		}
		u32_shr(&k);   // k = k / 2
	}
}

// Computing the NAF of a positive integer k
// w = 3
void get_naf_w3(const u32* pk, int8_t naf_w3[257])
{
	int i = 0;
	int8_t ki;
	u32 x = { 0 }, k;

	memcpy(&k, pk, sizeof(k));
	for (i = 0; i < 257; i++)
	{
		ki = ((int8_t)(k.v[0])) & 0x7;   // k mod (2^3)

										  //naf_w3[i] = ki;
		if ((ki & 1) != 0)  // k is odd
		{
			if (ki >= 4) // -2^2 < ki < 2^2
			{
				ki -= 8; // ki - 2^3
			}
			if (ki >= 0)
			{
				x.v[0] = ki;
				u32_sub(&k, &x, &k);
			}
			else  // ki < 0
			{
				x.v[0] = -ki;
				u32_add(&k, &x, &k);
			}
			naf_w3[i] = ki;
		}
		else
		{
			naf_w3[i] = 0;
		}
		u32_shr(&k);   // k = k / 2
	}
}

// Computing the NAF of a positive integer k
// w = 4
void get_naf_w4(const u32* pk, int8_t naf_w3[257])
{
	int i = 0;
	int8_t ki;
	u32 x = { 0 }, k;

	memcpy(&k, pk, sizeof(k));
	for (i = 0; i < 257; i++)
	{
		ki = ((int8_t)(k.v[0])) & 0xF;   // k mod (2^4)

		if ((ki & 1) != 0)  // k is odd
		{
			if (ki >= 8) // -2^3 < ki < 2^3
			{
				ki -= 16; // ki = ki - 2^4
			}
			if (ki >= 0)
			{
				x.v[0] = ki;
				u32_sub(&k, &x, &k);
			}
			else  // ki < 0
			{
				x.v[0] = -ki;
				u32_add(&k, &x, &k);
			}
			naf_w3[i] = ki;
		}
		else
		{
			naf_w3[i] = 0;
		}
		u32_shr(&k);   // k = k / 2
	}
}



// Just two item for each table, aka. PT[1] and PT[3]
// Positive table store at PT
// Negtive  table store at NT
void precompute_ptable_for_w3(const AFPoint* point, AFPoint PT[4], AFPoint NT[4])
{
	JPoint r, sqr;

	affine_to_jacobian(point, &r);
	CopyAFPoint(point, PT + 1);                 // 1 P
	AFPoint_neg(PT + 1, NT + 1);                // -1 P
	
	double_JPoint(&r, &sqr);
	add_JPoint_and_AFPoint(&sqr, point, &r);    // 3 P
	jacobian_to_affine(&r, PT+3);               // 3 P
	AFPoint_neg(PT + 3, NT + 3);                // -3 P
}

// Just four item for each table, aka. PT[1],PT[3],PT[5],PT[7]
// Positive table store at PT
// Negtive  table store at NT
void precompute_ptable_for_w4(const AFPoint* point, AFPoint PT[8], AFPoint NT[8])
{
	JPoint r, sqr;

	affine_to_jacobian(point, &r);
	CopyAFPoint(point, PT + 1);                 // 1 P
	AFPoint_neg(PT + 1, NT + 1);                // -1 P

	double_JPoint(&r, &sqr);
	add_JPoint_and_AFPoint(&sqr, point, &r);    // 3 P
	jacobian_to_affine(&r, PT + 3);             // 3 P
	AFPoint_neg(PT + 3, NT + 3);                // -3 P

	add_JPoint_and_AFPoint(&sqr, PT + 3, &r);   // 5 P
	jacobian_to_affine(&r, PT + 5);             // 5 P
	AFPoint_neg(PT + 5, NT + 5);                // -5 P

	add_JPoint_and_AFPoint(&sqr, PT + 5, &r);   // 7 P
	jacobian_to_affine(&r, PT + 7);             // 7 P
	AFPoint_neg(PT + 7, NT + 7);                // -7 P
}

void precompute_ptable_for_w5(const AFPoint* point)
{
	//P[i]=(2*i+1)P,i=0,1,..7, get 1,3,5,...15P
	size_t i = 0;
	JPoint p, p_squre;

	affine_to_jacobian(point, &p);
	double_JPoint(&p, &p_squre);
	AF_PTable[0] = *point;
	AFPoint_neg(point, AF_neg_PTable);
	for (i = 1; i < 8; i++)
	{
		add_JPoint(&p, &p_squre, &p);
		jacobian_to_affine(&p, AF_PTable + i);
		AFPoint_neg(AF_PTable + i, AF_neg_PTable + i);
	}
}

void precompute_ptable_for_w5_all_jpoint(const AFPoint* point, JPoint PT[16], JPoint NT[16])
{
	// Get 1P, 3P, 5P, 7P, 9P, 11P, 13P, 15P
	// P[i] = iP, i is odd
	// P[i] = 0,  i is even
	size_t i = 0;
	JPoint r = JPoint_ZERO, dr;

	// Fix bug in previous version of this function
	affine_to_jacobian(point, &r);  // r = 1 P
	double_JPoint(&r, &dr);         // dr = 2 P

	CopyJPoint(&r, PT + 1);         // 1 P
	JPoint_neg(&r, NT + 1);         // -1 P

	for (i = 3; i < 16; i += 2)
	{
		add_JPoint(&r, &dr, &r);
		CopyJPoint(&r, PT + i);
		JPoint_neg(&r, NT + i);
	}
}



// Note: this function return A Jacob Point

// NAF method for w = 5 (ALL OPERATION IN JACOBIAN COORDINATE)
void times_point_naf_w5_all_jpoint(const AFPoint* point, const u32* times, JPoint* result)
{
	int i = 0;
	int8_t ki = 0;
	u32 k = *times;
	int8_t naf_k[257];
	JPoint Q = JPoint_ZERO;
	static JPoint PT[16] = { 0 };
	static JPoint NT[16] = { 0 };

	precompute_ptable_for_w5_all_jpoint(point, PT, NT);
	get_naf_w5_2(&k, naf_k);
	for (i = 256; i >= 0; i--)
	{
		double_JPoint(&Q, &Q);
		ki = naf_k[i];
		if (ki)
		{
			if (ki > 0)
			{
				add_JPoint(&Q, PT + ki, &Q);
			}
			else
			{
				add_JPoint(&Q, NT + (-ki), &Q);
			}
		}
	}
	*result = Q;
}


// Binary method
void times_point_bin(const AFPoint* point, const u32* times, JPoint* result)
{
	JPoint T = JPoint_ZERO; // should be double_JPoint(T, T);
	const u8 l1 = 1;
	int i = 0, blocki = 0;

	for (blocki = 3; blocki >= 0; blocki--)
	{
		for (i = 63; i >= 0; i--)
		{
			// add_JPoint(T, T, T);
			double_JPoint(&T, &T);

			if ((times->v[blocki] & (l1 << i)) != 0) //this place can't use 1 to replace l1
			{
				add_JPoint_and_AFPoint(&T, point, &T);
			}
		}
	}
	*result = T;
}

// NAF method for w = 2
void times_point_naf_w2(const AFPoint* point, const u32* times, JPoint* result)
{
	int i = 0;
	int8_t ki = 0;
	u32 k = *times;
	int8_t naf_k[257];
	JPoint Q = JPoint_ZERO;
	static AFPoint PT[4];  // PT[0] = -1 P, PT[2] = 1 P,

	CopyAFPoint(point, PT + 2);
	AFPoint_neg(point, PT + 0);

	get_naf_w2(&k, naf_k);
	for (i = 256; i >= 0; i--)
	{
		double_JPoint(&Q, &Q);
		ki = naf_k[i];
		if (ki)
		{
			ki += 1;
			add_JPoint_and_AFPoint(&Q, PT + ki, &Q);
		}
	}
	*result = Q;
}

// NAF method
void times_point_naf_w5(const AFPoint* point, const u32* times, JPoint* result)
{
	int i = 0;
	int now_num = 0;
	u32 k = *times;
	int8_t naf_k[257];
	JPoint Q = JPoint_ZERO;

	precompute_ptable_for_w5(point);
	get_naf_w5_2(&k, naf_k);
	for (i = 256; i >= 0; i--)
	{
		double_JPoint(&Q, &Q);
		now_num = naf_k[i];
		if (now_num)
		{
			if (now_num > 0)
			{
				add_JPoint_and_AFPoint(&Q, &(AF_PTable[(now_num - 1) / 2]), &Q);
			}
			else
			{
				add_JPoint_and_AFPoint(&Q, &(AF_neg_PTable[(-now_num - 1) / 2]), &Q);
			}
		}
	}
	*result = Q;
}

// NAF method for w = 3
void times_point_naf_w3(const AFPoint* point, const u32* times, JPoint* result)
{
	int i = 0;
	int now_num = 0;
	u32 k = *times;
	int8_t naf_k[257];
	JPoint Q = JPoint_ZERO;
	static AFPoint PT[4] = { 0 };
	static AFPoint NT[4] = { 0 };

	precompute_ptable_for_w3(point, PT, NT);
	get_naf_w3(&k, naf_k);
	for (i = 256; i >= 0; i--)
	{
		double_JPoint(&Q, &Q);
		now_num = naf_k[i];
		if (now_num)
		{
			if (now_num > 0)
			{
				add_JPoint_and_AFPoint(&Q, PT + now_num, &Q);
			}
			else
			{
				add_JPoint_and_AFPoint(&Q, NT - now_num, &Q);
			}
		}
	}
	*result = Q;
}

// NAF method for w = 4
void times_point_naf_w4(const AFPoint* point, const u32* times, JPoint* result)
{
	int i = 0;
	int8_t ki = 0;
	u32 k = *times;
	int8_t naf_k[257];
	JPoint Q = JPoint_ZERO;
	static AFPoint PT[8] = { 0 };
	static AFPoint NT[8] = { 0 };

	precompute_ptable_for_w4(point, PT, NT);
	get_naf_w4(&k, naf_k);
	for (i = 256; i >= 0; i--)
	{
		double_JPoint(&Q, &Q);
		ki = naf_k[i];
		if (ki)
		{
			if (ki > 0)
			{
				add_JPoint_and_AFPoint(&Q, PT + ki, &Q);
			}
			else
			{
				add_JPoint_and_AFPoint(&Q, NT + (-ki), &Q);
			}
		}
	}
	*result = Q;
}


size_t get_u8_bit(u8 input, size_t i)
{
	return (size_t)((input >> i) & 1);
}

size_t to_index(const u32* input, size_t i)
{
	return get_u8_bit(input->v[0], i)
		+ (get_u8_bit(input->v[0], 32 + i) << 1)
		+ (get_u8_bit(input->v[1], i) << 2)
		+ (get_u8_bit(input->v[1], 32 + i) << 3)
		+ (get_u8_bit(input->v[2], i) << 4)
		+ (get_u8_bit(input->v[2], 32 + i) << 5)
		+ (get_u8_bit(input->v[3], i) << 6)
		+ (get_u8_bit(input->v[3], 32 + i) << 7);
}

// Speed up using Fixed-base comb
// described in "Software Implementation of the NIST Elliptic
// Curves Over Prime Fields" by M Brown et. al.

//write into tables
void gen_tables()
{
	static bool firstrun = true;
	int i = 0;
	if (firstrun) {
		// code here
		JPoint Gj;
		JPoint powG[256];
		size_t j = 0;
		JPoint T1 = JPoint_ZERO, T2 = JPoint_ZERO;
		size_t k = 0;
		affine_to_jacobian(&SM2_G, &Gj);
		powG[0] = Gj;

		for(i = 1; i < 256; i++)
		{
			double_JPoint(powG + i - 1, powG + i);
		}

		// find the desired values
		for(i = 0; i < 256; i++)
		{
			j = i;
			T1 = JPoint_ZERO, T2 = JPoint_ZERO;
			k = 0;
			while (j)
			{
				if ((j & 1))
				{
					// T = T + 2^{32p}G
					add_JPoint(&T1, powG + (k << 5), &T1);
					add_JPoint(&T2, powG + (k << 5) + (1 << 4), &T2);
				}
				j >>= 1;
				k += 1;
			}
			jacobian_to_affine(&T1, lowTable + i);
			jacobian_to_affine(&T2, highTable + i);
		}
		firstrun = false;
	}

}

// before using this function, you need use gen_Tables() to genarate tables.
void times_basepoint(const u32* times, JPoint* result)
{
	JPoint T = JPoint_ZERO;

	for (int i = 15; i >= 0; i--)
	{
		double_JPoint(&T, &T);
		size_t indexLow = to_index(times, i);
		size_t indexHigh = to_index(times, i + 16);
		add_JPoint_and_AFPoint(&T, lowTable + indexLow, &T);
		add_JPoint_and_AFPoint(&T, highTable + indexHigh, &T);
	}

	*result = T;
}

/*
// Inner test function
static void test_naf()
{
	u32 k = { 0xDB5C2F3268B73059, 0x7F6A21BAE5F2D1F5, 0xECEC3F8910938B20, 0x5A4E38911AB7D687 };
	u32  k1, k2, k3, k4, k5;
	int i = 0;
	int naf1[257] = { 0 };
	char naf2[257] = { 0 };
	char naf3[257] = { 0 };
	char naf4[257] = { 0 };
	char naf5[257] = { 0 };

	srand((unsigned)time(NULL));
	//get_random_u32_in_mod_n(&k);

	memcpy(&k1, &k, sizeof(u32));
	memcpy(&k2, &k, sizeof(u32));
	memcpy(&k3, &k, sizeof(u32));
	memcpy(&k4, &k, sizeof(u32));
	memcpy(&k5, &k, sizeof(u32));

	get_naf(&k1, naf1);
	get_naf_w5(&k2, naf2);
	get_naf_w3(&k3, naf3);
	get_naf_w4(&k4, naf4);
	get_naf_w5_2(&k5, naf5);

	for (i = 0; i < 257; i++)
	{
		if (i % 5 == 0) puts("");
		printf("[%2d]%3d  <--> %3d  <--> %3d  <--> %3d\n", i, naf1[i], naf2[i], naf3[i], naf4[i]);
		//if (naf1[i] != naf2[i])
		//{
		//	printf("[NOT EQUAL] : i = %d\n", i);
		//	printf("k:  0x%016llX, 0x%016llX, 0x%016llX, 0x%016llX\n", k.v[0], k.v[1], k.v[2], k.v[3]);
		//	puts("");
		//	//break;
		//}
	}
	puts("END!");
}

static void test_naf_w3()
{
	u32 k = { 0xDB5C2F3268B73059, 0x7F6A21BAE5F2D1F5, 0xECEC3F8910938B20, 0x5A4E38911AB7D687 };
	int i = 0;
	char naf[257] = { 0 };


	get_naf_w3(&k, naf);

	for (i = 0; i < 257; i++)
	{
		if (i % 3 == 0) puts("");
		printf("[%2d]%3d  <--> %3d\n", i, 0, naf[i]);
	}
	puts("END!");
}

static void test_naf_w4()
{
	u32 k = { 0xDB5C2F3268B73059, 0x7F6A21BAE5F2D1F5, 0xECEC3F8910938B20, 0x5A4E38911AB7D687 };
	int i = 0;
	char naf[257] = { 0 };


	get_naf_w4(&k, naf);

	for (i = 0; i < 257; i++)
	{
		if (i % 3 == 0) puts("");
		printf("[%2d]%3d  <--> %3d\n", i, 0, naf[i]);
	}
	puts("END!");
}

static void test_times_point()
{
	u32  k = { 0x03D127A0327EC400, 0x21C61D2891CB940A, 0x4DCCE9C93A58C040, 0x547586B29CD2397E };

	AFPoint P = 
	{
		0x6BCDB5B691ACDCA2, 0xF5E3A48ADB06E7D1, 0xE5B9F26CF2817C7E, 0x9FBD939ACF4E0490,
		0xE879F713CF7B2F83, 0xB8F5A4080703F0E9, 0xB15126BE5755D975, 0x1FFD50F8BE1A17E2,
	};
	JPoint R1, R2, Q, Pj;
	bool ret = false;

	//srand((unsigned)time(NULL));
	//gen_tables();
	// Get random k and P
	//get_random_u32_in_mod_n(&k);
	//times_basepoint(&k, &Pj);
	//jacobian_to_affine(&Pj, &P);

	times_point_bin(&P, &k, &R1);
	//times_point_naf_w2(&P, &k, &R2);
	//times_point_naf_w3(&P, &k, &R2);
	//times_point_naf_w4(&P, &k, &R2);
	times_point_naf_w5_all_jpoint(&P, &k, &R2);

	ret = equ_to_JPoint(&R1, &R2);
	if (ret == false)
	{
		puts("Not equal");

		puts("k = ");
		print_u32(&k);

		puts("P = ");
		print_AFPoint(&P);

		puts("R1 = ");
		print_JPoint(&R1);

		puts("R2 = ");
		print_JPoint(&R2);

	}
	else
	{
		puts("Equal");
	}
}

static void bench_times_point()
{
	u32  k;

	AFPoint P;
	JPoint R, Q, Pj;
	size_t i = 0, loop = 100000;
	clock_t t1 = 0, t2 = 0;
	double diff = 0, speed = 0;

	srand((unsigned)time(NULL));
	gen_tables();

	// Get random k and P
	get_random_u32_in_mod_n(&k);
	times_basepoint(&k, &Pj);
	jacobian_to_affine(&Pj, &P);

	// bench
	// Point Mul
	t1 = clock();
	for (i = 0; i < loop; i++)
	{
		// Default On JACOBIAN coordinate
		// R = k * P
		times_point_bin(&P, &k, &R);
	}
	t2 = clock();
	diff = t2 - t1;
	diff = diff / CLOCKS_PER_SEC;
	speed = loop / diff;
	printf("times_point_bin time: %lf s\n", diff / loop);
	printf("speed: %lf\n\n", speed);

	// bench
	// Point Mul
	
	t1 = clock();
	for (i = 0; i < loop; i++)
	{
		// Default On JACOBIAN coordinate
		// R = k * P
		times_point_naf_w2(&P, &k, &R);
	}
	t2 = clock();
	diff = t2 - t1;
	diff = diff / CLOCKS_PER_SEC;
	speed = loop / diff;
	printf("times_point_naf_w2 time: %lf s\n", diff / loop);
	printf("speed: %lf\n\n", speed);

	t1 = clock();
	for (i = 0; i < loop; i++)
	{
		// Default On JACOBIAN coordinate
		// R = k * P
		times_point_naf_w3(&P, &k, &R);
	}
	t2 = clock();
	diff = t2 - t1;
	diff = diff / CLOCKS_PER_SEC;
	speed = loop / diff;
	printf("times_point_naf_w3 time: %lf s\n", diff / loop);
	printf("speed: %lf\n\n", speed);

	t1 = clock();
	for (i = 0; i < loop; i++)
	{
		// Default On JACOBIAN coordinate
		// R = k * P
		times_point_naf_w4(&P, &k, &R);
	}
	t2 = clock();
	diff = t2 - t1;
	diff = diff / CLOCKS_PER_SEC;
	speed = loop / diff;
	printf("times_point_naf_w4 time: %lf s\n", diff / loop);
	printf("speed: %lf\n\n", speed);

	t1 = clock();
	for (i = 0; i < loop; i++)
	{
		// Default On JACOBIAN coordinate
		// R = k * P
		times_point_naf_w5(&P, &k, &R);
	}
	t2 = clock();
	diff = t2 - t1;
	diff = diff / CLOCKS_PER_SEC;
	speed = loop / diff;
	printf("times_point_naf_w5 time: %lf s\n", diff / loop);
	printf("speed: %lf\n\n", speed);
	
	t1 = clock();
	for (i = 0; i < loop; i++)
	{
		// Default On JACOBIAN coordinate
		// R = k * P
		times_point_naf_w5_all_jpoint(&P, &k, &R);
	}
	t2 = clock();
	diff = t2 - t1;
	diff = diff / CLOCKS_PER_SEC;
	speed = loop / diff;
	printf("times_point_naf_w5_all_jpoint time: %lf s\n", diff / loop);
	printf("speed: %lf\n\n", speed);
}

static void test_all_jpoint()
{
	AFPoint point = 
	{ 
		0x6BCDB5B691ACDCA2, 0xF5E3A48ADB06E7D1, 0xE5B9F26CF2817C7E, 0x9FBD939ACF4E0490,
		0xE879F713CF7B2F83, 0xB8F5A4080703F0E9, 0xB15126BE5755D975, 0x1FFD50F8BE1A17E2, 
	};
	static JPoint PT[16] = { 0 };
	static JPoint NT[16] = { 0 };
	int i = 0;
	JPoint r, dr;

	precompute_ptable_for_w5_all_jpoint(&point, PT, NT);
	affine_to_jacobian(&point, &r);
	double_JPoint(&r, &dr);

	for (i = 3; i < 16; i += 2)
	{
		add_JPoint(&r, &dr, &r);
		if (equ_to_JPoint(&r, PT + i) == false)
		{
			printf("ERROR at i = %d\n", i);
			break;
		}
	}
	puts("test_all_jpoint() END!");
}

static void test_point_convert()
{
	AFPoint p = 
	{
		0x6BCDB5B691ACDCA2, 0xF5E3A48ADB06E7D1, 0xE5B9F26CF2817C7E, 0x9FBD939ACF4E0490,
		0xE879F713CF7B2F83, 0xB8F5A4080703F0E9, 0xB15126BE5755D975, 0x1FFD50F8BE1A17E2,
	};
	JPoint s = JPoint_ZERO, q;
	AFPoint r;
	size_t i = 0, loop = 10000000;
	clock_t t1 = 0, t2 = 0;
	double diff = 0, speed = 0;

	affine_to_jacobian(&p, &q);

	t1 = clock();
	for (i = 0; i < loop; i++)
	{
		// Convert affine coordinate to jacobian coordinate
		jacobian_to_affine(&q, &r);
	}
	t2 = clock();
	diff = t2 - t1;
	diff = diff / CLOCKS_PER_SEC;
	speed = loop / diff;
	printf("jacobian_to_affine time: %lf s\n", diff / loop);
	printf("speed: %lf\n\n", speed);

	t1 = clock();
	for (i = 0; i < loop; i++)
	{
		// Convert affine coordinate to jacobian coordinate
		add_JPoint_and_AFPoint(&s, &p, &r);
	}
	t2 = clock();
	diff = t2 - t1;
	diff = diff / CLOCKS_PER_SEC;
	speed = loop / diff;
	printf("add_JPoint_and_AFPoint time: %lf s\n", diff / loop);
	printf("speed: %lf\n\n", speed);

}


void self_main_test_point_mul()
{
	//test_naf();
	//test_naf_w3();
	//test_naf_w4();
	//test_all_jpoint();
	//test_point_convert();

	//test_times_point();
	bench_times_point();

}
*/
