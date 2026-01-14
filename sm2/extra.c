#include "include/extra.h"
#include "include/utils.h"

// Now for test random generation module
//#define RANDOM_GEN_OLD

char hex2char(uint8_t h)
{
	if (h >= 0 && h <= 9)
	{
		return h + '0';
	}
	else if (h >= 10 && h <= 15)
	{
		return h + 'a';
	}
	else
	{
		//return 
		exit(0);
	}
}

// outbuf must be double-length of hex plus 1.  e.g sizeof(outbuf) >= (2 * hex + 1)
int hex2str(const uint8_t* hex, uint32_t hexlen, char* outbuf)
{
	int status = -1;
	uint32_t i = 0;
	uint8_t ch, cl, c;

	if (hex == NULL || hexlen == 0)
	{
		return status;
	}

	for (i = 0; i < hexlen; i++)
	{
		c = hex[i];
		ch = c >> 4;
		cl = c & 0xf;
		outbuf[2 * i] = hex2char(ch);
		outbuf[2 * i + 1] = hex2char(cl);
	}
	outbuf[2 * i] = 0;

	status = 0;
	return status;
}

// Erase buffer by random data
// Generally, the buffer should not be very large, e.g. the key
void erase_data(void* buf, uint32_t buflen)
{
	memset(buf, buflen, buflen);
}

uint32_t rol(const uint32_t value, const size_t bits)
{
	return (value << bits) | (value >> (32 - bits));
}

u8 u8_rand()
{
	u8 r;

	random_fill((u1 *)(&r), 8);

	return r;
}

void u32_rand(u32* input)
{
	//input = { u8_rand(), u8_rand(), u8_rand(), u8_rand() };
	input->v[0] = u8_rand();
	input->v[1] = u8_rand();
	input->v[2] = u8_rand();
	input->v[3] = u8_rand();
}


void str_reverse_in_place(u1 *str, int len)
{
	u1 *p1 = str;
	u1 *p2 = str + len - 1;

	while (p1 < p2)
	{
		u1 tmp = *p1;
		*p1++ = *p2;
		*p2-- = tmp;
	}
}

//fix a bug
void u1_to_u32(u1 const input[32], u32* result)
{
	u1 tmp[32];
	memcpy(tmp, input, 32);
	str_reverse_in_place(tmp, 32);
	//memcpy(&result, input, 32); //this is wrong!
	memcpy(result, tmp, 32);
}

void u4_to_u32(u4 input[8], u32* result)
{
	/*
	result.v[0] = *(u8 *)(input + 0);
	result.v[1] = *(u8 *)(input + 2);
	result.v[2] = *(u8 *)(input + 4);
	result.v[3] = *(u8 *)(input + 6);
	*/

	result->v[0] = ((u8)input[1] << 32) + (u8)input[0];
	result->v[1] = ((u8)input[3] << 32) + (u8)input[2];
	result->v[2] = ((u8)input[5] << 32) + (u8)input[4];
	result->v[3] = ((u8)input[7] << 32) + (u8)input[6];
}


//KDF Key-derived algorithm (for the convenience of processing length, dedicated to the sm2 encryption algorithm), using the sm3 hash algorithm (256-bit output)
int KDF(u1 Z[], u8 zlen, u8 klen, u1 K[])
{
	u4 ct = 0x1;	//32bits
	size_t klenmod = klen % 32;
	size_t loop = klenmod ? (klen / 32 + 1) : (klen / 32);
	u1 ha[11][32] = { 0 }; //Restricted size
	u1 msg_md[200] = { 0 };
	if (loop > 10)
	{
		return -1;
	}
	for (size_t i = 0; i < loop; i++)
	{
		memcpy(msg_md, Z, zlen);
		memcpy(msg_md + zlen, &ct, sizeof(ct));
		str_reverse_in_place(msg_md + zlen, sizeof(ct));
		sm3(msg_md, zlen + 4, ha[i]);
		ct++;
	}
	memcpy(K, ha, klen);
	return 0;
}