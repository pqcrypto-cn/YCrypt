
#include "include/sm3.h"

#define cpu_to_be32(v) (((v)>>24) | (((v)>>8)&0xff00) | (((v)<<8)&0xff0000) | ((v)<<24))

void sm3_init(SM3_CTX *ctx)
{
	ctx->digest[0] = 0x7380166F;
	ctx->digest[1] = 0x4914B2B9;
	ctx->digest[2] = 0x172442D7;
	ctx->digest[3] = 0xDA8A0600;
	ctx->digest[4] = 0xA96F30BC;
	ctx->digest[5] = 0x163138AA;
	ctx->digest[6] = 0xE38DEE4D;
	ctx->digest[7] = 0xB0FB0E4E;

	ctx->nblocks = 0;
	ctx->num = 0;
}

void sm3_update(SM3_CTX *ctx, const uint8_t * data, uint32_t data_len)
{
	if (ctx->num)
	{
		uint32_t left = SM3_BLOCK_SIZE - ctx->num;
		if (data_len < left)
		{
			memcpy(ctx->block + ctx->num, data, data_len);
			ctx->num += data_len;
			return;
		}
		else
		{
			memcpy(ctx->block + ctx->num, data, left);
			sm3_compress(ctx->digest, ctx->block);
			ctx->nblocks++;
			data += left;
			data_len -= left;
		}
	}
	while (data_len >= SM3_BLOCK_SIZE)
	{
		sm3_compress(ctx->digest, data);
		ctx->nblocks++;
		data += SM3_BLOCK_SIZE;
		data_len -= SM3_BLOCK_SIZE;
	}
	ctx->num = data_len;
	if (data_len)
	{
		memcpy(ctx->block, data, data_len);
	}
}

void sm3_final(SM3_CTX *ctx, uint8_t digest[SM3_DIGEST_LENGTH])
{
	uint32_t i = 0;
	uint32_t * pdigest = (uint32_t *)digest;
	uint32_t * count = (uint32_t *)(ctx->block + SM3_BLOCK_SIZE - 8);

	ctx->block[ctx->num] = 0x80;

	if (ctx->num + 9 <= SM3_BLOCK_SIZE)
	{
		memset(ctx->block + ctx->num + 1, 0, SM3_BLOCK_SIZE - ctx->num - 9);
	}
	else
	{
		memset(ctx->block + ctx->num + 1, 0, SM3_BLOCK_SIZE - ctx->num - 1);
		sm3_compress(ctx->digest, ctx->block);
		memset(ctx->block, 0, SM3_BLOCK_SIZE - 8);
	}

	count[0] = (uint32_t)(cpu_to_be32((ctx->nblocks) >> 23));
	count[1] = (uint32_t)(cpu_to_be32((ctx->nblocks << 9) + (ctx->num << 3)));

	sm3_compress(ctx->digest, ctx->block);

	//print_msg("before convert endian digest:");
	//print_uint32(ctx->digest, 8);
	//print_msg("");

	for (i = 0; i < sizeof(ctx->digest) / sizeof(ctx->digest[0]); ++i)
	{
		pdigest[i] = cpu_to_be32(ctx->digest[i]);
	}
}


#define ROTATELEFT(X,n)  (((X)<<(n%32)) | ((X)>>(32-(n%32))))

#define P0(x) ((x) ^  ROTATELEFT((x),9)  ^ ROTATELEFT((x),17))
#define P1(x) ((x) ^  ROTATELEFT((x),15) ^ ROTATELEFT((x),23))

#define FF0(x,y,z) ( (x) ^ (y) ^ (z) )
#define FF1(x,y,z) (((x) & (y)) | ( (x) & (z)) | ( (y) & (z)))

#define GG0(x,y,z) ( (x) ^ (y) ^ (z) )
#define GG1(x,y,z) (((x) & (y)) | ( (~(x)) & (z)) )


void sm3_compress(uint32_t digest[8], const uint8_t block[SM3_BLOCK_SIZE])
{
	uint32_t j = 0;
	uint32_t W[68] = { 0 }, W1[64] = { 0 };
	const uint32_t *pblock = (const uint32_t *)block;

	uint32_t A = digest[0];
	uint32_t B = digest[1];
	uint32_t C = digest[2];
	uint32_t D = digest[3];
	uint32_t E = digest[4];
	uint32_t F = digest[5];
	uint32_t G = digest[6];
	uint32_t H = digest[7];
	uint32_t SS1, SS2, TT1, TT2, T[64];


	for (j = 0; j < 16; j++)
	{
		W[j] = cpu_to_be32(pblock[j]);
	}

	//static int counter = 0;
	//printf("[%d] block:\n", counter++);
	//print_uchar(block, SM3_BLOCK_SIZE);
	//print_msg("current digest:");
	//print_uint32(digest, 8);
	//print_msg("");

	// Message expeand
	for (j = 16; j < 68; j++)
	{
		W[j] = P1(W[j - 16] ^ W[j - 9] ^ ROTATELEFT(W[j - 3], 15)) ^ ROTATELEFT(W[j - 13], 7) ^ W[j - 6];
	}
	for (j = 0; j < 64; j++)
	{
		W1[j] = W[j] ^ W[j + 4];
	}

	// Message compress
	for (j = 0; j < 16; j++)
	{
		T[j] = 0x79CC4519;
		SS1 = ROTATELEFT((ROTATELEFT(A, 12) + E + ROTATELEFT(T[j], j)), 7);
		SS2 = SS1 ^ ROTATELEFT(A, 12);
		TT1 = FF0(A, B, C) + D + SS2 + W1[j];
		TT2 = GG0(E, F, G) + H + SS1 + W[j];
		D = C;
		C = ROTATELEFT(B, 9);
		B = A;
		A = TT1;
		H = G;
		G = ROTATELEFT(F, 19);
		F = E;
		E = P0(TT2);
	}

	for (j = 16; j < 64; j++)
	{
		T[j] = 0x7A879D8A;
		SS1 = ROTATELEFT((ROTATELEFT(A, 12) + E + ROTATELEFT(T[j], j)), 7);
		SS2 = SS1 ^ ROTATELEFT(A, 12);
		TT1 = FF1(A, B, C) + D + SS2 + W1[j];
		TT2 = GG1(E, F, G) + H + SS1 + W[j];
		D = C;
		C = ROTATELEFT(B, 9);
		B = A;
		A = TT1;
		H = G;
		G = ROTATELEFT(F, 19);
		F = E;
		E = P0(TT2);
	}

	digest[0] ^= A;
	digest[1] ^= B;
	digest[2] ^= C;
	digest[3] ^= D;
	digest[4] ^= E;
	digest[5] ^= F;
	digest[6] ^= G;
	digest[7] ^= H;

	//static int counter = 0;
	//printf("[%d] block:\n", counter++);
	//print_uchar(block, SM3_BLOCK_SIZE);
	//puts("current digest:");
	//print_uchar((uint8_t*)digest, 32);
	//puts("");

}

void sm3(const uint8_t *msg, uint32_t msglen, uint8_t dgst[SM3_DIGEST_LENGTH])
{
	SM3_CTX ctx;

	sm3_init(&ctx);
	sm3_update(&ctx, msg, msglen);
	sm3_final(&ctx, dgst);

	memset(&ctx, 0, sizeof(SM3_CTX));
}




#define IPAD	0x36
#define OPAD	0x5C

void sm3_hmac_init(SM3_HMAC_CTX *ctx, const uint8_t *key, uint32_t key_len)
{
	int i = 0;

	if (key_len <= SM3_BLOCK_SIZE)
	{
		memcpy(ctx->key, key, key_len);
		memset(ctx->key + key_len, 0, SM3_BLOCK_SIZE - key_len);
	}
	else
	{
		sm3_init(&ctx->sm3_ctx);
		sm3_update(&ctx->sm3_ctx, key, key_len);
		sm3_final(&ctx->sm3_ctx, ctx->key);
		memset(ctx->key + SM3_DIGEST_LENGTH, 0, SM3_BLOCK_SIZE - SM3_DIGEST_LENGTH);
	}
	for (i = 0; i < SM3_BLOCK_SIZE; i++)
	{
		ctx->key[i] ^= IPAD;
	}

	sm3_init(&ctx->sm3_ctx);
	sm3_update(&ctx->sm3_ctx, ctx->key, SM3_BLOCK_SIZE);
}

void sm3_hmac_update(SM3_HMAC_CTX *ctx, const uint8_t *data, uint32_t data_len)
{
	sm3_update(&ctx->sm3_ctx, data, data_len);
}

void sm3_hmac_final(SM3_HMAC_CTX *ctx, uint8_t mac[SM3_HMAC_SIZE])
{
	int i = 0;

	for (i = 0; i < SM3_BLOCK_SIZE; i++)
	{
		ctx->key[i] ^= (IPAD ^ OPAD);
	}
	sm3_final(&ctx->sm3_ctx, mac);
	sm3_init(&ctx->sm3_ctx);
	sm3_update(&ctx->sm3_ctx, ctx->key, SM3_BLOCK_SIZE);
	sm3_update(&ctx->sm3_ctx, mac, SM3_DIGEST_LENGTH);
	sm3_final(&ctx->sm3_ctx, mac);
}

void sm3_hmac(const uint8_t *data, uint32_t data_len, const uint8_t *key, uint32_t key_len, uint8_t mac[SM3_HMAC_SIZE])
{
	SM3_HMAC_CTX ctx;
	sm3_hmac_init(&ctx, key, key_len);
	sm3_hmac_update(&ctx, data, data_len);
	sm3_hmac_final(&ctx, mac);
	memset(&ctx, 0, sizeof(ctx));
}


