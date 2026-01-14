#include "include/sm2.h"

extern const u32 SM2_a;
extern const u32 SM2_b;
extern const u32 SM2_N;
extern const u32 SM2_P;
extern const AFPoint SM2_G;


void sm2_keypair(PubKey* pubkey, PrivKey *privkey) {
	//generate private key
	get_random_u32_in_mod_n(&(privkey->da));
	//generate public key
	sm2_get_public_key(privkey, pubkey);
}

void sm2_get_public_key(const PrivKey *privkey, PubKey* pubkey)
{
	JPoint rst_jacobian;
	ML_mul_basepoint(&(privkey->da), &rst_jacobian);
	jacobian_to_affine(&rst_jacobian, pubkey);
	// montg_times_base_point(&(privkey->da), &rst_jacobian);
	// montg_jpoint_to_apoint(&rst_jacobian, pubkey->x.v, pubkey->y.v);
}

// Get ZA=Hash256(ENTLA || id || a || b || xG || yG || xA || yA)
// result contains ZA.
int sm2_get_id_digest(
	u1 result[32], 
	const u1 *id, 
	size_t id_len, 
	const PubKey* pubkey)
{
	if (id_len>50)
	{
		return -1;
	}
	//u32 little-endian
	u1 total[250];
	u1 charlist[192];
	// 2-byte id length in bits 
	//ZA=Hash256(ENTLA || id || a || b || xG || yG || xA || yA)
	total[0] = ((id_len * 8) >> 8) % 256;
	total[1] = (id_len * 8) % 256;
	memcpy(total + 2, id, id_len);
	//low bit...high bit, need reverse
	memcpy(charlist, &SM2_a, 32);
	str_reverse_in_place(charlist, 32);

	memcpy(charlist + 32, &SM2_b, 32);
	str_reverse_in_place(charlist + 32, 32);

	memcpy(charlist + 64, &(SM2_G.x), 32);
	str_reverse_in_place(charlist + 64, 32);

	memcpy(charlist + 96, &(SM2_G.y), 32);
	str_reverse_in_place(charlist + 96, 32);

	memcpy(charlist + 128, &(pubkey->x), 32);
	str_reverse_in_place(charlist + 128, 32);

	memcpy(charlist + 160, &(pubkey->y), 32);
	str_reverse_in_place(charlist + 160, 32);

	memcpy(total + 2 + id_len, charlist, 192);
	sm3(total, 2 + id_len + 192, result);
	return 0;
}

// result = Hash256(ZA || msg) 
int sm2_get_message_digest(
	u1 result[32],
	const u1 ZA[32], 
	const u1 *msg, 
	size_t msg_len)
{
	SM3_CTX ctx;
	sm3_init(&ctx);
	sm3_update(&ctx, ZA, 32);
	sm3_update(&ctx, msg, msg_len);
	sm3_final(&ctx, result);
	return 0;
}

// Success: return 1.
// Fail: return 0.
int sm2_sign_dgst(
	SM2SIG *sig, 
	u1 dgst[32], 
	const PrivKey* privkey)
{
	u32 r = { 0,0,0,0 };
	u32 s = { 0,0,0,0 };
	u32 k;

	u32 e, tmp, inversion_da_and_1, k_subtract_rda;
	JPoint rand_JPoint;
	AFPoint rand_AFPoint;

	u1_to_u32(dgst, &e);
	mod_n(&e, &e);

	while (u32_eq_zero(&s))
	{
		get_random_u32_in_mod_n(&k);

		montg_times_base_point(&k, &rand_JPoint);
		montg_jpoint_to_apoint(&rand_JPoint, rand_AFPoint.x.v, NULL);

		add_mod_n(&e, &(rand_AFPoint.x), &r);
		add_mod_n(&r, &k, &tmp);

		if (u32_eq_zero(&r) || u32_eq_zero(&tmp))
		{
			continue;
		}

		// Calculate s = (1+da)^-1 * (k-r*da);
		inversion_da_and_1.v[0] = 1;
		inversion_da_and_1.v[1] = 0;
		inversion_da_and_1.v[2] = 0;
		inversion_da_and_1.v[3] = 0;
		add_mod_n(&(privkey->da), &(inversion_da_and_1), &(inversion_da_and_1));
		inv_for_mul_mod_n(&inversion_da_and_1, &inversion_da_and_1);
		
		mul_mod_n(&r, &(privkey->da), &k_subtract_rda);
		sub_mod_n(&k, &k_subtract_rda, &k_subtract_rda);
		mul_mod_n(&inversion_da_and_1, &k_subtract_rda, &s);
	}
	sig->r = r;
	sig->s = s;

	return 1;
}

// Success: return 1.
// Fail: return 0.
int sm2_sign(
	SM2SIG *sig,
	const u1 *msg,
	size_t msg_len, 
	const u1 *id, 
	size_t id_len,
	const PubKey* pubkey,
	const PrivKey* privkey)
{
	u1 ZA[32], dgst[32];
	if (!is_on_curve(pubkey))
	{
		printf("public key is not on the curve!");
		return 0;
	}
	//caculate dgst
	sm2_get_id_digest(ZA, id, id_len, pubkey);
	sm2_get_message_digest(dgst, ZA, msg, msg_len);

	return sm2_sign_dgst(sig, dgst, privkey);
}


int sm2_verify_old(u1 dgst[32], const PubKey* pubkey, u32* r, u32* s)
{
	// verify that Q is indeed on the curve
	// to prevent false curve attack
	u32 e, t, R;
	JPoint tmp1 = JPoint_ZERO, point1_jacobian = JPoint_ZERO;
	AFPoint point1;

	if (!is_on_curve(pubkey))
	{
		printf("Public key not on curve!\n");
		return -1;
	}

	if (u32_ge(r, &SM2_N) || u32_eq_zero(r) || u32_ge(s, &SM2_N) || u32_eq_zero(s))
		return -1;

	add_mod_n(r, s, &t);
	if (u32_eq_zero(&t))
		return -1;

	u1_to_u32(dgst, &e);

	//times_basepoint(s, &tmp1);
	ML_mul_basepoint(s, &tmp1);

	times_point(pubkey, &t, &point1_jacobian);

	add_JPoint(&tmp1, &point1_jacobian, &point1_jacobian);
	jacobian_to_affine(&point1_jacobian, &point1);

	//mod(&e, &SM2_N, &SM2_rhoN);
	//mod(&point1.x, &SM2_N, &SM2_rhoN);
	mod_n(&e, &e);
	mod_n(&point1.x, &point1.x);
	add_mod_n(&e, &(point1.x), &R);

	return u32_eq(&R, r);
}

// Success: return 1.
// Fail: return 0.
int sm2_verify(
	const SM2SIG *signature, 
	const u1 dgst[32], 
	const AFPoint * pubkey)
{
	// verify that Q is indeed on the curve
	// to prevent false curve attack
	u32 e, t, R;
	JPoint tmp1 = JPoint_ZERO, point1_jacobian = JPoint_ZERO;

	if (!is_on_curve(pubkey))
	{
		printf("Public key not on curve!\n");
		return 0;
	}

	const u32* r = &(signature->r);
	const u32* s = &(signature->s);

	if (u32_ge(r, &SM2_N) || u32_eq_zero(r) || u32_ge(s, &SM2_N) || u32_eq_zero(s))
		return 0;

	add_mod_n(r, s, &t);
	if (u32_eq_zero(&t))
		return 0;

	u1_to_u32(dgst, &e);

	montg_times_base_point(s, &tmp1);
	montg_times_point_naf_w3(pubkey, &t, &point1_jacobian);

	montg_add_jpoint(&tmp1, &point1_jacobian, &point1_jacobian);
	montg_jpoint_to_apoint(&point1_jacobian, t.v, NULL); // Get x coordinate;

	mod_n(&e, &e);
	mod_n(&t, &t);
	add_mod_n(&e, &t, &R);

	return u32_eq(&R, r);
}

// Success: return 1.
// Fail: return 0.
int sm2_verify_msg(
	const SM2SIG *sig,
	const u1 *msg,
	size_t msg_len, 
	const u1 *id, 
	size_t id_len,
	const PubKey* pubkey)
{
	u1 ZA[32], dgst[32];

	//caculate dgst
	sm2_get_id_digest(ZA, id, id_len, pubkey);
	sm2_get_message_digest(dgst, ZA, msg, msg_len);

	return sm2_verify(sig, dgst, pubkey);
}
