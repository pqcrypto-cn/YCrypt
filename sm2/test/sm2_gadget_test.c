#include "../include/utils.h"
#include "../include/sm2.h"

void test_raw_mul()
{
	u32 a = { 0x1351534EF350E2BB, 0x14E68D77BC131F7B, 0x6A7171A01A638E75, 0x4F9EA7A816AB7908 };
	u32 b = { 0x141CC66D0595B6F0, 0xC85BF76622E07301, 0x5B261629F8AD4D45, 0x7DE9CF63BC635636 };
	u8 rst[8];
	int i = 0;
	raw_mul(a.v, b.v, rst);

	u8 realrst[8] =
	{
		0x866d99203adc8150, 0xc623d9758ed1332c, 0x3b1dab20b950e375, 0xbc165cad5d713996,
		0x63e9be904aa539b5, 0x7edc6525c6a1f17c, 0x2a99a65d2ec61248, 0x27292fc3f99184ca
	};

	for(i = 0; i < 8; i++)
		printf("%016llx %016llx %016llx \n", rst[i], realrst[i], realrst[i] - rst[i]);

	for (size_t i = 0; i < 8; i++)
	{
		if (rst[i] != realrst[i])
		{
			printf("raw mul error!\n");
			return;
		}
	}

	puts("raw mul successful!\n");
}
void test_raw_pow()
{
	// u32 a = { 0xaed66ce184be2329, 0xebe9bbf1f1499052, 0x993e0c873cdba6b3, 0xde47b7061c0d5e24 };
	// u8 r1[8];
	// u8 r2[8];
	// int i = 0, j = 0, k = 0;

	// for (i = 0; i < 10000; i++)
	// {
	// 	//u32_rand(&a);
	// 	//print_u32(&a);

	// 	raw_mul(a.v, a.v, r1);
	// 	raw_pow(a.v, r2);
	// 	for(j = 0; j < 8; j++)
	// 	{
	// 		if (r1[j] != r2[j])
	// 		{
	// 			printf("pos: %d is not equal\n", j);
	// 			for (k = 0; k < 8; k++)
	// 			{
	// 				printf("%016llx, %016llx, %016llx\n", r1[7 - k], r2[7 - k], r2[7 - k] - r1[7 - k]);

	// 			}
	// 			puts("");
	// 			return;
	// 		}
	// 	}
	// }
	// puts("Raw pow test success!");

	UINT64 a[4] = {0x4ED90CBB66509399, 0xFC20C8432F351896, 0x507E40EC1228D642, 0x3A02E7B0CBAC0ACD};

	UINT64 res[8] = {0};

	raw_pow(a, res);
	printBN(res, 8);

}

void solinas_reduce_v1(UINT64 a[8], UINT64 result[4])
{
	uint32_t arr[10] = { 0 };
	uint32_t * ptr = (uint32_t *)a;
	UINT64 t = 0, u = 0, *p64;
	uint8_t carry = 0;
	u32* p;

	t = ptr[0];
	t = t + ptr[8] + ptr[9] + ptr[10] + ptr[11] + ptr[12] + 2ll * ptr[13] +  2ll * ptr[14] + 2ll * ptr[15];
	arr[0] = (uint32_t)t;
	t = t >> 32;

	t = t + ptr[1];
	t = t + ptr[9] + ptr[10] + ptr[11] + ptr[12] + ptr[13] + 2ll* ptr[14] + 2ll * ptr[15];
	arr[1] = (uint32_t)t;
	t = t >> 32;

	t = t + ptr[2];
	u = u + ptr[8] + ptr[9] + ptr[13] + ptr[14];  // U need to deal
	arr[2] = (uint32_t)t;
	t = t >> 32;

	t = t + ptr[3];
	t = t + ptr[8] + ptr[11] + ptr[12] + 2ll * ptr[13] + ptr[14] + ptr[15];
	arr[3] = (uint32_t)t;
	t = t >> 32;

	t = t + ptr[4];
	t = t + ptr[9] + ptr[12] + ptr[13] + 2ll * ptr[14] + ptr[15];
	arr[4] = (uint32_t)t;
	t = t >> 32;

	t = t + ptr[5];
	t = t + ptr[10] + ptr[13] + ptr[14] + 2ll * ptr[15];
	arr[5] = (uint32_t)t;
	t = t >> 32;

	t = t + ptr[6];
	t = t + ptr[11] + ptr[14] + ptr[15];
	arr[6] = (uint32_t)t;
	t = t >> 32;

	t = t + ptr[7];
	t = t + ptr[8] + ptr[9] + ptr[10] + ptr[11] + 2ll * ptr[12] + 2ll * ptr[13] + 2ll * ptr[14] + 3ll * ptr[15];
	arr[7] = (uint32_t)t;
	t = t >> 32;

	arr[8] = (uint32_t)t;

	p64 = (UINT64*)arr;

	// Sufficent for substract
	if (p64[4] > 0 || p64[3] > 0 || p64[2] > 0 || p64[1] > u)
	{
		carry = 0;
		carry = _subborrow_u64(carry, p64[1], u, p64 + 1);
		carry = _subborrow_u64(carry, p64[2], 0, p64 + 2);
		carry = _subborrow_u64(carry, p64[3], 0, p64 + 3);
		carry = _subborrow_u64(carry, p64[4], 0, p64 + 4);

		while (p64[4] > 0)
		{
			carry = 0;
			carry = _subborrow_u64(carry, p64[0], SM2_P.v[0], p64 + 0);
			carry = _subborrow_u64(carry, p64[1], SM2_P.v[1], p64 + 1);
			carry = _subborrow_u64(carry, p64[2], SM2_P.v[2], p64 + 2);
			carry = _subborrow_u64(carry, p64[3], SM2_P.v[3], p64 + 3);
			carry = _subborrow_u64(carry, p64[4], 0, p64 + 4);
		}

		p = (u32*)arr;
		while (u32_ge(p, &SM2_P))
		{
			carry = 0;
			carry = _subborrow_u64(carry, p64[0], SM2_P.v[0], p64 + 0);
			carry = _subborrow_u64(carry, p64[1], SM2_P.v[1], p64 + 1);
			carry = _subborrow_u64(carry, p64[2], SM2_P.v[2], p64 + 2);
			carry = _subborrow_u64(carry, p64[3], SM2_P.v[3], p64 + 3);
		}
	}
	else
	{
		carry = 0;
		carry = _addcarryx_u64(carry, SM2_P.v[0], p64[0], p64 + 0);
		carry = _addcarryx_u64(carry, SM2_P.v[1], p64[1], p64 + 1);
		carry = _addcarryx_u64(carry, SM2_P.v[2], p64[2], p64 + 2);
		carry = _addcarryx_u64(carry, SM2_P.v[3], p64[3], p64 + 3);
		carry = _addcarryx_u64(carry, 0, p64[4], p64 + 4);

		carry = 0;
		carry = _subborrow_u64(carry, p64[1], u, p64 + 1);
		carry = _subborrow_u64(carry, p64[2], 0, p64 + 2);
		carry = _subborrow_u64(carry, p64[3], 0, p64 + 3);
	}

	memcpy(result, arr, 32);
}
void solinas_reduce_v2(UINT64 a[8], UINT64 result[4])
{
	uint32_t arr[10] = { 0 };
	uint32_t * ptr = (uint32_t *)a;
	UINT64 t = 0, u = 0, *p64;
	uint8_t carry = 0;
	u32* p;

	t = ptr[0];
	t = t + ptr[8] + ptr[9] + ptr[10] + ptr[11] + ptr[12] + 2ll * ptr[13] + 2ll * ptr[14] + 2ll * ptr[15];
	arr[0] = (uint32_t)t;
	t = t >> 32;

	t = t + ptr[1];
	t = t + ptr[9] + ptr[10] + ptr[11] + ptr[12] + ptr[13] + 2ll * ptr[14] + 2ll * ptr[15];
	arr[1] = (uint32_t)t;
	t = t >> 32;

	t = t + ptr[2];
	u = u + ptr[8] + ptr[9] + ptr[13] + ptr[14];  // U need to deal
	arr[2] = (uint32_t)t;
	t = t >> 32;

	t = t + ptr[3];
	t = t + ptr[8] + ptr[11] + ptr[12] + 2ll * ptr[13] + ptr[14] + ptr[15];
	arr[3] = (uint32_t)t;
	t = t >> 32;

	t = t + ptr[4];
	t = t + ptr[9] + ptr[12] + ptr[13] + 2ll * ptr[14] + ptr[15];
	arr[4] = (uint32_t)t;
	t = t >> 32;

	t = t + ptr[5];
	t = t + ptr[10] + ptr[13] + ptr[14] + 2ll * ptr[15];
	arr[5] = (uint32_t)t;
	t = t >> 32;

	t = t + ptr[6];
	t = t + ptr[11] + ptr[14] + ptr[15];
	arr[6] = (uint32_t)t;
	t = t >> 32;

	t = t + ptr[7];
	t = t + ptr[8] + ptr[9] + ptr[10] + ptr[11] + 2ll * ptr[12] + 2ll * ptr[13] + 2ll * ptr[14] + 3ll * ptr[15];
	arr[7] = (uint32_t)t;
	t = t >> 32;

	arr[8] = (uint32_t)t;

	p64 = (UINT64*)arr;

	carry = 0;
	carry = _subborrow_u64(carry, p64[1], u, p64 + 1);
	carry = _subborrow_u64(carry, p64[2], 0, p64 + 2);
	carry = _subborrow_u64(carry, p64[3], 0, p64 + 3);
	carry = _subborrow_u64(carry, p64[4], 0, p64 + 4);
	if (carry)
	{
		// Too small
		carry = 0;
		carry = _addcarryx_u64(carry, SM2_P.v[0], p64[0], p64 + 0);
		carry = _addcarryx_u64(carry, SM2_P.v[1], p64[1], p64 + 1);
		carry = _addcarryx_u64(carry, SM2_P.v[2], p64[2], p64 + 2);
		carry = _addcarryx_u64(carry, SM2_P.v[3], p64[3], p64 + 3);

	}
	else
	{
		while (p64[4] > 0)
		{
			carry = 0;
			carry = _subborrow_u64(carry, p64[0], SM2_P.v[0], p64 + 0);
			carry = _subborrow_u64(carry, p64[1], SM2_P.v[1], p64 + 1);
			carry = _subborrow_u64(carry, p64[2], SM2_P.v[2], p64 + 2);
			carry = _subborrow_u64(carry, p64[3], SM2_P.v[3], p64 + 3);
			carry = _subborrow_u64(carry, p64[4], 0, p64 + 4);
		}
	}
	if (u32_ge(p64, &SM2_P))
	{
		carry = 0;
		carry = _subborrow_u64(carry, p64[0], SM2_P.v[0], p64 + 0);
		carry = _subborrow_u64(carry, p64[1], SM2_P.v[1], p64 + 1);
		carry = _subborrow_u64(carry, p64[2], SM2_P.v[2], p64 + 2);
		carry = _subborrow_u64(carry, p64[3], SM2_P.v[3], p64 + 3);

	}
		
	memcpy(result, arr, 32);
}

void test_solinas_reduce()
{
	//UINT64 a[8] = { 0x1351534EF350E2BB, 0x14E68D77BC131F7B, 0x6A7171A01A638E75, 0x4F9EA7A816AB7908, 0x141CC66D0595B6F0, 0xC85BF76622E07301, 0x5B261629F8AD4D45, 0x7DE9CF63BC635636 };
	UINT64 a[8] = { 
		0xAED66CE184BE2329, 0xEBE9BBF1F1499052, 0x993E0C873CDBA6B3, 0xDE47B7061C0D5E24,
		0xA68BBB43C84D12B3, 0x1F2538097D5A031F, 0x3B45F596FCCBD45D, 0x32AEDB1C0A890D13, };
	UINT64 b[4] = { 0 };
	UINT64 c[4] = { 0 };
	int i = 0, j , k, loop = 1000000;

	a[0] = 1;
	a[1] = 1;
	a[2] = 1;
	a[3] = 1;
	a[4] = 1;
	a[5] = 1;
	a[6] = 1;
	a[7] = 1;

	for (i = 0; i < loop; i++)
	{
		//random_fill(a, 64);
		solinas_reduce(a, b);
		solinas_reduce_asm(a, c);
		//solinas_reduce_v1(a, c);
		//solinas_reduce_v2(a, c);
		if (memcmp(b, c, 32) != 0)
		{
			printf("Error: [%d]\n", i);
			puts("a = ");
			for (k = 0; k < 8; k++)
			{
				if (k == 4)
				{
					puts("");
				}
				printf("0x%016llX, ", a[k]);

			}
			puts("\n");

			puts("result:");
			for (k = 0; k < 4; k++)
			{
				printf("0x%016llX, 0x%016llX, 0x%016llX\n", b[k], c[k], b[k] - c[k]);
			}
			return;
		}
	}
	
	puts("test_solinas_reduce() success!\n");
}

void bench_solinas_reduce()
{
	UINT64 a[8] = {
		0xAED66CE184BE2329, 0xEBE9BBF1F1499052, 0x993E0C873CDBA6B3, 0xDE47B7061C0D5E24,
		0xA68BBB43C84D12B3, 0x1F2538097D5A031F, 0x3B45F596FCCBD45D, 0x32AEDB1C0A890D13, };
	UINT64 b[4] = { 0 };
	UINT64 c[4] = { 0 };
	int i = 0, j, k, loop = 100000000;
	clock_t start, end;
	double diff, speed;

	start = clock();
	for (i = 0; i < loop; i++)
	{
		solinas_reduce(a, b);
	}
	end = clock();
	diff = end - start;
	diff /= CLOCKS_PER_SEC;
	speed = loop / diff;
	printf("solinas_reduce: %f\n", diff);
	printf("solinas_reduce speed: %f\n", speed);

	start = clock();
	for (i = 0; i < loop; i++)
	{
		solinas_reduce_asm(a, b);
	}
	end = clock();
	diff = end - start;
	diff /= CLOCKS_PER_SEC;
	speed = loop / diff;
	printf("solinas_reduce_asm: %f\n", diff);
	printf("solinas_reduce_asm speed: %f\n", speed);


}

void get_15_times_P()
{
	int i = 0;
	UINT64 a[4] = { SM2_P.v[0], SM2_P.v[1],SM2_P.v[2],SM2_P.v[3] };
	UINT64 b[5] = { 0 };
	uint8_t carry = 0;
	
	// i * P
	//     N[0]                N[1]                N[2]                N[3]                N[4]
	//[01] 0FFFFFFFFFFFFFFFFh, 0FFFFFFFF00000000h, 0FFFFFFFFFFFFFFFFh, 0FFFFFFFEFFFFFFFFh, 00000000000000000h
	//[02] 0FFFFFFFFFFFFFFFEh, 0FFFFFFFE00000001h, 0FFFFFFFFFFFFFFFFh, 0FFFFFFFDFFFFFFFFh, 00000000000000001h
	//[03] 0FFFFFFFFFFFFFFFDh, 0FFFFFFFD00000002h, 0FFFFFFFFFFFFFFFFh, 0FFFFFFFCFFFFFFFFh, 00000000000000002h
	//[04] 0FFFFFFFFFFFFFFFCh, 0FFFFFFFC00000003h, 0FFFFFFFFFFFFFFFFh, 0FFFFFFFBFFFFFFFFh, 00000000000000003h
	//[05] 0FFFFFFFFFFFFFFFBh, 0FFFFFFFB00000004h, 0FFFFFFFFFFFFFFFFh, 0FFFFFFFAFFFFFFFFh, 00000000000000004h
	//[06] 0FFFFFFFFFFFFFFFAh, 0FFFFFFFA00000005h, 0FFFFFFFFFFFFFFFFh, 0FFFFFFF9FFFFFFFFh, 00000000000000005h
	//[07] 0FFFFFFFFFFFFFFF9h, 0FFFFFFF900000006h, 0FFFFFFFFFFFFFFFFh, 0FFFFFFF8FFFFFFFFh, 00000000000000006h
	//[08] 0FFFFFFFFFFFFFFF8h, 0FFFFFFF800000007h, 0FFFFFFFFFFFFFFFFh, 0FFFFFFF7FFFFFFFFh, 00000000000000007h
	//[09] 0FFFFFFFFFFFFFFF7h, 0FFFFFFF700000008h, 0FFFFFFFFFFFFFFFFh, 0FFFFFFF6FFFFFFFFh, 00000000000000008h
	//[10] 0FFFFFFFFFFFFFFF6h, 0FFFFFFF600000009h, 0FFFFFFFFFFFFFFFFh, 0FFFFFFF5FFFFFFFFh, 00000000000000009h
	//[11] 0FFFFFFFFFFFFFFF5h, 0FFFFFFF50000000Ah, 0FFFFFFFFFFFFFFFFh, 0FFFFFFF4FFFFFFFFh, 0000000000000000Ah
	//[12] 0FFFFFFFFFFFFFFF4h, 0FFFFFFF40000000Bh, 0FFFFFFFFFFFFFFFFh, 0FFFFFFF3FFFFFFFFh, 0000000000000000Bh
	//[13] 0FFFFFFFFFFFFFFF3h, 0FFFFFFF30000000Ch, 0FFFFFFFFFFFFFFFFh, 0FFFFFFF2FFFFFFFFh, 0000000000000000Ch
	//[14] 0FFFFFFFFFFFFFFF2h, 0FFFFFFF20000000Dh, 0FFFFFFFFFFFFFFFFh, 0FFFFFFF1FFFFFFFFh, 0000000000000000Dh
	//[15] 0FFFFFFFFFFFFFFF1h, 0FFFFFFF10000000Eh, 0FFFFFFFFFFFFFFFFh, 0FFFFFFF0FFFFFFFFh, 0000000000000000Eh
	

	for (i = 1; i < 16; i++)
	{
		carry = 0;
		carry = _addcarryx_u64(carry, a[0], b[0], b + 0);
		carry = _addcarryx_u64(carry, a[1], b[1], b + 1);
		carry = _addcarryx_u64(carry, a[2], b[2], b + 2);
		carry = _addcarryx_u64(carry, a[3], b[3], b + 3);
		carry = _addcarryx_u64(carry,    0, b[4], b + 4);

		printf("[%02d] ", i);
		printf("0%016llXh, 0%016llXh, 0%016llXh, 0%016llXh, 0%016llXh\n", b[0], b[1], b[2], b[3], b[4]);
		//printf("0%016llXh, 0%016llXh, 0%016llXh, 0%016llXh\n", b[0], b[1], b[2], b[3]);
	}
	puts("\n");
}

void get_15_times_rhoP()
{
	int i = 0, k;
	UINT64 a[8] = { SM2_rhoP.v[0], SM2_rhoP.v[1],SM2_rhoP.v[2],SM2_rhoP.v[3] };
	UINT64 b[8] = { 0 };
	UINT64 c[8] = { 2 };
	uint8_t carry = 0;

	// i * rhoP
	//     N[0]                N[1]                N[2]                N[3]                N[4]
	//[01] 00000000000000001h, 000000000FFFFFFFFh, 00000000000000000h, 00000000100000000h, 00000000000000000h
	//[02] 00000000000000002h, 000000001FFFFFFFEh, 00000000000000000h, 00000000200000000h, 00000000000000000h
	//[03] 00000000000000003h, 000000002FFFFFFFDh, 00000000000000000h, 00000000300000000h, 00000000000000000h
	//[04] 00000000000000004h, 000000003FFFFFFFCh, 00000000000000000h, 00000000400000000h, 00000000000000000h
	//[05] 00000000000000005h, 000000004FFFFFFFBh, 00000000000000000h, 00000000500000000h, 00000000000000000h
	//[06] 00000000000000006h, 000000005FFFFFFFAh, 00000000000000000h, 00000000600000000h, 00000000000000000h
	//[07] 00000000000000007h, 000000006FFFFFFF9h, 00000000000000000h, 00000000700000000h, 00000000000000000h
	//[08] 00000000000000008h, 000000007FFFFFFF8h, 00000000000000000h, 00000000800000000h, 00000000000000000h
	//[09] 00000000000000009h, 000000008FFFFFFF7h, 00000000000000000h, 00000000900000000h, 00000000000000000h
	//[10] 0000000000000000Ah, 000000009FFFFFFF6h, 00000000000000000h, 00000000A00000000h, 00000000000000000h
	//[11] 0000000000000000Bh, 00000000AFFFFFFF5h, 00000000000000000h, 00000000B00000000h, 00000000000000000h
	//[12] 0000000000000000Ch, 00000000BFFFFFFF4h, 00000000000000000h, 00000000C00000000h, 00000000000000000h
	//[13] 0000000000000000Dh, 00000000CFFFFFFF3h, 00000000000000000h, 00000000D00000000h, 00000000000000000h
	//[14] 0000000000000000Eh, 00000000DFFFFFFF2h, 00000000000000000h, 00000000E00000000h, 00000000000000000h
	//[15] 0000000000000000Fh, 00000000EFFFFFFF1h, 00000000000000000h, 00000000F00000000h, 00000000000000000h

	printf("[%02d] ", 0);
	printf("0%016llXh, 0%016llXh, 0%016llXh, 0%016llXh, 0%016llXh\n", a[0], a[1], a[2], a[3], a[4]);
	k = 2;
	for (i = 0; i < 32; i++)
	{
		c[0] *= 2;
	}
	raw_mul(a, c, a);
	printf("[%02d] ", 32);
	printf("0%016llXh, 0%016llXh, 0%016llXh, 0%016llXh, 0%016llXh\n", a[0], a[1], a[2], a[3], a[4]);
	printf("[a] << 32 \n");
	printf("[%02d] ", 1);
	printf("0%016llXh, 0%016llXh, 0%016llXh, 0%016llXh, 0%016llXh\n", a[0], a[1], a[2], a[3], a[4]);

	for (i = 1; i < 16; i++)
	{
		carry = 0;
		carry = _addcarryx_u64(carry, a[0], b[0], b + 0);
		carry = _addcarryx_u64(carry, a[1], b[1], b + 1);
		carry = _addcarryx_u64(carry, a[2], b[2], b + 2);
		carry = _addcarryx_u64(carry, a[3], b[3], b + 3);
		carry = _addcarryx_u64(carry, a[4], b[4], b + 4);

		printf("[%02d] ", i);
		printf("0%016llXh, 0%016llXh, 0%016llXh, 0%016llXh, 0%016llXh\n", b[0], b[1], b[2], b[3], b[4]);
		//printf("0%016llXh, 0%016llXh, 0%016llXh, 0%016llXh\n", b[0], b[1], b[2], b[3]);
	}
	puts("\n");
}

void test_point_mul()
{
	u32 k = 
	{
		0x2e803cdc77305b2f, 0x037dc3936597bd34, 0xbe7dfd57166758a2, 0xd0fc7af8e84e1287,
	};
	AFPoint P;
	JPoint R, Q = 
	{
		0x36FAA03431A47119, 0xF171A29C24088160, 0x55DF209F5F63A2F5, 0x58FEC3C3B83E2AF9,
		0xD9CD93AB0FDD4515, 0x9A29AF96B96FCAA6, 0x102929A06EA5A0EA, 0x98452FA8B8D5F670,
		0xB70AB9C2F4D7096E, 0xF4F65B6D9A6BBE83, 0xCF6894163F7BF685, 0x9D1B6D7A82564337,
	};

	puts("k:");
	print_u32(&k);

	puts("P:");
	print_affine_point(&P);

	puts("Q:");
	print_JPoint(&Q);


	jacobian_to_affine(&Q, &P);
	// R = k * P
	times_point(&P, &k, &R);

	puts("Result:");
	print_JPoint(&R);

}
void bench_point_mul()
{
	u32 sig[2], k;

	AFPoint P; 
	JPoint R, Q;
	size_t i = 0, loop = 100000;
	clock_t t1 = 0, t2 = 0;
	double diff = 0, speed = 0;

	srand((unsigned)time(NULL));
	gen_tables();

	//generate private key
	get_random_u32_in_mod_n(&k);
	times_basepoint(&k, &Q);
	jacobian_to_affine(&Q, &P);
	//get_random_u32_in_mod_p(&(P.x));
	//get_random_u32_in_mod_p(&(P.y));

	// bench

	// Base Point Mul
	t1 = clock();
	for (i = 0; i < loop; i++)
	{
		// R = k * G
		times_basepoint(&k, &R);
	}
	t2 = clock();
	diff = t2 - t1;
	diff = diff / CLOCKS_PER_SEC;
	speed = loop / diff;
	printf("Mul base point time: %lf s\n", diff / loop);
	printf("speed: %lf\n\n", speed);

	// ML-Version Base Point Mul
	t1 = clock();
	for (i = 0; i < loop; i++)
	{
		// R = k * G
		ML_mul_basepoint(&k, &R);
	}
	t2 = clock();
	diff = t2 - t1;
	diff = diff / CLOCKS_PER_SEC;
	speed = loop / diff;
	printf("ML-Version mul base point time: %lf s\n", diff / loop);
	printf("speed: %lf\n\n", speed);

	// Point Mul
	t1 = clock();
	for (i = 0; i < loop; i++)
	{
		// Default On JACOBIAN coordinate
		// R = k * P
		times_point(&P, &k, &R);
	}
	t2 = clock();
	diff = t2 - t1;
	diff = diff / CLOCKS_PER_SEC;
	speed = loop / diff;
	printf("Mul point time: %lf s\n", diff / loop);
	printf("speed: %lf\n\n", speed);


	// Point Add
	//affine_to_jacobian(&P, &Q);
	get_random_u32_in_mod_p(&k);
	times_basepoint(&k, &Q);
	
	loop *= 100;
	t1 = clock();
	for (i = 0; i < loop; i++)
	{
		// R = P + Q
		add_JPoint_and_AFPoint(&Q, &P, &R);
	}
	t2 = clock();
	diff = t2 - t1;
	diff = diff / CLOCKS_PER_SEC;
	speed = loop / diff;
	printf("Point add time: %lf s\n", diff / loop);
	printf("speed: %lf\n\n", speed);


	// Point Double
	t1 = clock();
	for (i = 0; i < loop; i++)
	{
		// R = 2 * Q
		double_JPoint(&Q, &R);
	}
	t2 = clock();
	diff = t2 - t1;
	diff = diff / CLOCKS_PER_SEC;
	speed = loop / diff;
	printf("Point double time: %lf s\n", diff / loop);
	printf("speed: %lf\n\n", speed);


}

void test_mul_mod_p()
{
	// size_t i, j, k, loop = 1000000;
	// UINT64 a[4] = { 0 };
	// UINT64 b[4] = { 0 };
	// UINT64 r1[4] = { 0 };
	// UINT64 r2[4] = { 0 };

	// srand((unsigned)time(NULL));
	// for (j = 0; j < loop; j++)
	// {
	// 	random_fill(a, 32);
	// 	random_fill(b, 32);

	// 	sm2p_mong_mul(a, b, r1);
	// 	montg_mul_mod_p(a, b, r2);

	// 	if (memcmp(r1, r2, 32) != 0)
	// 	{
	// 		printf("Error: [%d]\n", j);
	// 		puts("a = ");
	// 		for (k = 0; k < 4; k++)
	// 		{
	// 			if (k == 4) puts("");
	// 			printf("0x%016llX, ", a[k]);
	// 		}
	// 		puts("\n");
	// 		puts("b = ");
	// 		for (k = 0; k < 4; k++)
	// 		{
	// 			if (k == 4) puts("");
	// 			printf("0x%016llX, ", b[k]);
	// 		}
	// 		puts("\n");

	// 		puts("result:");
	// 		for (i = 0; i < 4; i++)
	// 		{
	// 			printf("%016llx %016llx %016llx \n", r1[i], r2[i], r1[i] - r2[i]);
	// 		}
	// 		break;
	// 	}
	// }
	// puts("test_mul_mod_p() end!");

	UINT64 a[4] = {0x01E6BB441730B7B0, 0x90B3D9BD63042E7C, 0xDACA6D041C87B482, 0x1BA39F090EB120F4};
	UINT64 b[4] = {0x4ED90CBB66509399, 0xFC20C8432F351896, 0x507E40EC1228D642, 0x3A02E7B0CBAC0ACD};

	UINT64 res[4] = {0};

	mul_mod_p(a, b, res);
	printBN(res, 4);


}

void bench_mul_mod_p()
{
	size_t i, j, k, loop = 100000000;
	UINT64 a[4] = { 0 };
	UINT64 b[4] = { 0 };
	UINT64 r1[4] = { 0 };
	UINT64 r2[4] = { 0 };
	clock_t t1 = 0, t2 = 0;
	double diff = 0, speed = 0;

	srand((unsigned)time(NULL));
	random_fill(a, 32);
	random_fill(b, 32);


	// sm2p_mong_mul
	t1 = clock();
	for (i = 0; i < loop; i++)
	{
		sm2p_mong_mul(a, b, r1);
	}
	t2 = clock();
	diff = t2 - t1;
	diff = diff / CLOCKS_PER_SEC;
	speed = loop / diff;
	printf("sm2p_mong_mul time: %lf s\n", diff / loop);
	printf("speed: %lf\n\n", speed);

	// montg_mul_mod_p
	t1 = clock();
	for (i = 0; i < loop; i++)
	{
		montg_mul_mod_p(a, b, r2);
	}
	t2 = clock();
	diff = t2 - t1;
	diff = diff / CLOCKS_PER_SEC;
	speed = loop / diff;
	printf("montg_mul_mod_p time: %lf s\n", diff / loop);
	printf("speed: %lf\n\n", speed);

}

void test_sqr_mod_p()
{
	size_t i, j, k, loop = 1000000;
	UINT64 a[4] = { 0 };
	UINT64 r1[4] = { 0 };
	UINT64 r2[4] = { 0 };

	srand((unsigned)time(NULL));
	for (j = 0; j < loop; j++)
	{
		random_fill(a, 32);

		sm2p_mong_pow(a, r1);
		montg_sqr_mod_p(a, r2);

		if (memcmp(r1, r2, 32) != 0)
		{
			printf("Error: [%d]\n", j);
			puts("a = ");
			for (k = 0; k < 4; k++)
			{
				if (k == 4) puts("");
				printf("0x%016llX, ", a[k]);
			}
			puts("\n");

			puts("result:");
			for (i = 0; i < 4; i++)
			{
				printf("%016llx %016llx %016llx \n", r1[i], r2[i], r1[i] - r2[i]);
			}
			break;
		}
	}
	puts("test_sqr_mod_p() end!");
}

void bench_sqr_mod_p()
{
	size_t i, j, k, loop = 100000000;
	UINT64 a[4] = { 0 };
	UINT64 r1[4] = { 0 };
	UINT64 r2[4] = { 0 };
	clock_t t1 = 0, t2 = 0;
	double diff = 0, speed = 0;

	srand((unsigned)time(NULL));
	random_fill(a, 32);


	// sm2p_mong_mul
	t1 = clock();
	for (i = 0; i < loop; i++)
	{
		sm2p_mong_pow(a, r1);
	}
	t2 = clock();
	diff = t2 - t1;
	diff = diff / CLOCKS_PER_SEC;
	speed = loop / diff;
	printf("sm2p_mong_pow time: %lf s\n", diff / loop);
	printf("speed: %lf\n\n", speed);

	// montg_mul_mod_p
	t1 = clock();
	for (i = 0; i < loop; i++)
	{
		montg_sqr_mod_p(a, r2);
	}
	t2 = clock();
	diff = t2 - t1;
	diff = diff / CLOCKS_PER_SEC;
	speed = loop / diff;
	printf("montg_sqr_mod_p time: %lf s\n", diff / loop);
	printf("speed: %lf\n\n", speed);

}

void test_montg_op_mod_p()
{
	size_t i, j, k, loop = 1000000;
	UINT64 a[4] = { 0 };
	UINT64 b[4] = { 0 };
	UINT64 x[4] = { 0 };
	UINT64 y[4] = { 0 };
	UINT64 r1[4] = { 0 };
	UINT64 r2[4] = { 0 };

	srand((unsigned)time(NULL));
	for (j = 0; j < loop; j++)
	{
		get_random_u32_in_mod_p(a);
		get_random_u32_in_mod_p(b);

		// Mongomery standard operation
		{
			// x = a * 2^(256)
			montg_to_mod_p(a, x);
			// y = b * 2^(256)
			montg_to_mod_p(b, y);

			// r1 = x * y * 2^(-256) = a * 2^(256) * b * 2^(256) * 2^(-256) = a * b * 2^(256)
			montg_mul_mod_p(x, y, r1);

			// r1 = r1 * 2^(-256) =  a * b * 2^(256) * 2^(-256) = a * b
			montg_back_mod_p(r1, r1);
		}
		
		// Mongomery non-standard operation
		{
			// r2 =  a * b * 2^(-256)
			sm2p_mong_mul(a, b, r2);

			// r2 =  r2 * SM2_H * 2^(-256) = r2 *  2^(512) * 2^(-256) = r2 * 2^(256)  = a * b * 2^(-256) * 2^(256) = a * b
			sm2p_mong_mul(r2, SM2_H.v, r2);
		}

		if (memcmp(r1, r2, 32) != 0)
		{
			printf("Error: [%d]\n", j);
			puts("a = ");
			for (k = 0; k < 4; k++)
			{
				if (k == 4) puts("");
				printf("0x%016llX, ", a[k]);
			}
			puts("\n");

			puts("result:");
			for (i = 0; i < 4; i++)
			{
				printf("%016llx %016llx %016llx \n", r1[i], r2[i], r1[i] - r2[i]);
			}
			break;
		}
	}
	puts("test_sqr_mod_p() end!");
}


void test_double_point()
{
	size_t i, j, loop = 1000000;
	JPoint a, b, c;
	u32 k = { 10, 0,0,0 };
	bool ret = false;
	srand((unsigned)time(NULL));


	// Ensure this point is on the curve
	//get_random_u32_in_mod_p(&k);
	gen_tables();  // In order to use the function of times_basepoint()
	times_basepoint(&k, &a);
	print_JPoint(&a);

	for (i = 0; i < loop; i++)
	{
		double_JPoint(&a, &b);
		double_JPoint2(&a, &c);

		ret = equ_to_JPoint(&b, &c);
		if (ret == false)
		{
			puts("Not equal!");
			break;
		}
		//else
		//{
		//	puts("Equal!");
		//}
	}
	
	puts("test_double_point end!");
}
void test_double_JPoint()
{
	size_t i, j, loop = 1000000;

	u32 k = { 10, 0,0,0 };
	bool ret = false;
	srand((unsigned)time(NULL));
	JPoint src = 
	{
		0x36FAA03431A47119, 0xF171A29C24088160, 0x55DF209F5F63A2F5, 0x58FEC3C3B83E2AF9,
		0xD9CD93AB0FDD4515, 0x9A29AF96B96FCAA6, 0x102929A06EA5A0EA, 0x98452FA8B8D5F670,
		0xB70AB9C2F4D7096E, 0xF4F65B6D9A6BBE83, 0xCF6894163F7BF685, 0x9D1B6D7A82564337,
	};
	JPoint dst = { 0 };

	puts("Source:");
	print_JPoint(&src);

	double_JPoint(&src, &dst);

	puts("Destination:");
	print_JPoint(&dst);
	
}


void bench_double_point()
{
	size_t i, j, k, loop = 100000000;
	JPoint a, b, c;
	clock_t t1 = 0, t2 = 0;
	double diff = 0, speed = 0;

	srand((unsigned)time(NULL));
	affine_to_jacobian(&SM2_G, &a);


	// double_JPoint
	t1 = clock();
	for (i = 0; i < loop; i++)
	{
		double_JPoint(&a, &b);
	}
	t2 = clock();
	diff = t2 - t1;
	diff = diff / CLOCKS_PER_SEC;
	speed = loop / diff;
	printf("double_JPoint time: %lf s\n", diff / loop);
	printf("speed: %lf\n\n", speed);

	// double_JPoint2
	t1 = clock();
	for (i = 0; i < loop; i++)
	{
		double_JPoint2(&a, &b);
	}
	t2 = clock();
	diff = t2 - t1;
	diff = diff / CLOCKS_PER_SEC;
	speed = loop / diff;
	printf("double_JPoint2 time: %lf s\n", diff / loop);
	printf("speed: %lf\n\n", speed);

}

void bench_JPoint_add()
{
	u32 k;
	AFPoint T;
	JPoint R, P, Q;
	size_t i = 0, loop = 10000000;
	clock_t t1 = 0, t2 = 0;
	double diff = 0, speed = 0;

	srand((unsigned)time(NULL));
	gen_tables();

	//generate P, Q
	get_random_u32_in_mod_n(&k);
	times_basepoint(&k, &P);

	get_random_u32_in_mod_n(&k);
	times_basepoint(&k, &Q);

	// bench
	t1 = clock();
	for (i = 0; i < loop; i++)
	{
		// R = P + Q
		add_JPoint(&Q, &P, &R);
	}
	t2 = clock();
	diff = t2 - t1;
	diff = diff / CLOCKS_PER_SEC;
	speed = loop / diff;
	printf("JPoint add time: %lf s\n", diff / loop);
	printf("speed: %lf\n\n", speed);


	// bench
	t1 = clock();
	for (i = 0; i < loop; i++)
	{
		// R = P + Q
		jacobian_to_affine(&P, &T);
		add_JPoint_and_AFPoint(&Q, &T, &R);
	}
	t2 = clock();
	diff = t2 - t1;
	diff = diff / CLOCKS_PER_SEC;
	speed = loop / diff;
	printf("JPoint2 add time: %lf s\n", diff / loop);
	printf("speed: %lf\n\n", speed);

	// bench
	t1 = clock();
	jacobian_to_affine(&P, &T);
	for (i = 0; i < loop; i++)
	{
		// R = P + Q
		add_JPoint_and_AFPoint(&Q, &T, &R);
	}
	t2 = clock();
	diff = t2 - t1;
	diff = diff / CLOCKS_PER_SEC;
	speed = loop / diff;
	printf("JPoint add AFPoint time: %lf s\n", diff / loop);
	printf("speed: %lf\n\n", speed);

}


void test_times_point2()
{
	u32  k;
	AFPoint P;
	JPoint R1, R2, Q;
	size_t i = 0, loop = 1000;
	clock_t t1 = 0, t2 = 0;
	bool ret = false;

	UINT64 buf_k[4] = 
	{ 
		0x2e803cdc77305b2f, 0x037dc3936597bd34, 0xbe7dfd57166758a2, 0xd0fc7af8e84e1287 
	};
	UINT64 buf_Q[3*4] = 
	{ 
		0x36faa03431a47119, 0xf171a29c24088160, 0x55df209f5f63a2f5, 0x58fec3c3b83e2af9,
		0xd9cd93ab0fdd4515, 0x9a29af96b96fcaa6, 0x102929a06ea5a0ea, 0x98452fa8b8d5f670,
		0xb70ab9c2f4d7096e, 0xf4f65b6d9a6bbe83, 0xcf6894163f7bf685, 0x9d1b6d7a82564337,
	};

	srand((unsigned)time(NULL));
	gen_tables();

	// Get random k, P, Q
	get_random_u32_in_mod_n(&k);
	times_basepoint(&k, &Q);
	jacobian_to_affine(&Q, &P);

	// Fix k and Q
	memcpy(&k, buf_k, 4*8);
	memcpy(&Q, buf_Q, 3*4*8);

	puts("k = ");
	print_u32(&k);

	puts("P = ");
	print_affine_point(&P);

	puts("Q = ");
	print_JPoint(&Q);


	// Default On JACOBIAN coordinate
	// R2 = k * Q
	times_point2(&Q, &k, &R1);

	puts("R1 = ");
	print_JPoint(&R1);

	montg_back_mod_p(R1.x.v, R2.x.v);
	montg_back_mod_p(R1.y.v, R2.y.v);
	montg_back_mod_p(R1.z.v, R2.z.v);

	puts("R2 = ");
	print_JPoint(&R2); 

}

void test_times_point()
{
	u32  k;
	AFPoint P;
	JPoint R1, R2, Q;
	size_t i = 0, loop = 1000;
	clock_t t1 = 0, t2 = 0;
	bool ret = false;

	srand((unsigned)time(NULL));
	gen_tables();

	// Get random k and P
	get_random_u32_in_mod_n(&k);
	times_basepoint(&k, &Q);
	jacobian_to_affine(&Q, &P);

	puts("k = ");
	print_u32(&k);

	puts("P = ");
	print_affine_point(&P);
	
	puts("Q = ");
	print_JPoint(&Q);


	// Default On JACOBIAN coordinate
	// R1 = k * P
	times_point(&P, &k, &R1);

	// R2 = k * P
	times_point2(&Q, &k, &R2);
	montg_back_mod_p(R2.x.v, R2.x.v);
	montg_back_mod_p(R2.y.v, R2.y.v);
	montg_back_mod_p(R2.z.v, R2.z.v);

	ret = equ_to_JPoint(&R1, &R2);
	if (ret == false)
	{
		puts("Not equal!");

		puts("R1 (times_point)= ");
		print_JPoint(&R1);

		puts("R2 (times_point2)= ");
		print_JPoint(&R2);
	}
	else
	{
		puts("Equal!");
	}

}

void bench_times_point()
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
		times_point(&P, &k, &R);
	}
	t2 = clock();
	diff = t2 - t1;
	diff = diff / CLOCKS_PER_SEC;
	speed = loop / diff;
	printf("times_point time: %lf s\n", diff / loop);
	printf("speed: %lf\n\n", speed);

	// bench
	// Point Mul
	t1 = clock();
	for (i = 0; i < loop; i++)
	{
		// Default On JACOBIAN coordinate
		// R = k * P
		times_point2(&Pj, &k, &R);
	}
	t2 = clock();
	diff = t2 - t1;
	diff = diff / CLOCKS_PER_SEC;
	speed = loop / diff;
	printf("times_point2 time: %lf s\n", diff / loop);
	printf("speed: %lf\n\n", speed);

}

void test_montg_back()
{
	UINT64 a[3*4] = 
	{
		0x765C374A041A88E9, 0x7FE8A1DD4E2566EF, 0x716E0EA356495A50, 0xB2CB79EE73181B6D,
		0x85EEE75BF5115DEA, 0x6711F39D60AF9798, 0x0FC16C3E3B58FF2D, 0x16871A3FFC6751BB,
		0x9B41D8F2873C0060, 0xA0176751C4116998, 0xBF70284B23D8B590, 0x336A5AED137BBDB2,
	};
	UINT64 r[3 * 4] = { 0 };

	montg_back_mod_p(a , r);
	montg_back_mod_p(a + 4, r + 4);
	montg_back_mod_p(a + 8, r + 8);

	puts("Result:");
	print_JPoint(r);
}


/*
void test_times_P()
{
	size_t i = 0, j = 0, k = 0, loop = 10000000;
	uint8_t carry = 0;
	UINT64 r[8] = { 0 };
	UINT64 base[8] = { 0 };
	UINT64 prebase[8] = { 0 };
	uint32_t offset = 0;
	// P = { 0xFFFFFFFFFFFFFFFF, 0xFFFFFFFF00000000, 0xFFFFFFFFFFFFFFFF, 0xFFFFFFFEFFFFFFFF };

	FILE* f = fopen("xP.txt", "w");
	//memcpy(r, SM2_P.v, 32);

	for (k = 0; k < 8; k++)
	{
		memset(base, 0, sizeof(base));
		memcpy(base, SM2_P.v, 32);
		if (k > 0)
		{
			offset = k * 8;
			base[4] = __shiftleft128(base[3], base[4], offset);
			base[3] = __shiftleft128(base[2], base[3], offset);
			base[2] = __shiftleft128(base[1], base[2], offset);
			base[1] = __shiftleft128(base[0], base[1], offset);
			base[0] = __ll_lshift(base[0], offset);

			//carry = 0;
			//carry = _addcarryx_u64(carry, base[0], prebase[0], base + 0);
			//carry = _addcarryx_u64(carry, base[1], prebase[1], base + 1);
			//carry = _addcarryx_u64(carry, base[2], prebase[2], base + 2);
			//carry = _addcarryx_u64(carry, base[3], prebase[3], base + 3);
			//carry = _addcarryx_u64(carry, base[4], prebase[4], base + 4);
			//carry = _addcarryx_u64(carry, base[5],         0, base + 5);
		}

		memcpy(prebase, base, sizeof(base));
		memset(r, 0, sizeof(r));

		for (i = 0; i < 256; i++)
		{
			// Add
			carry = 0;
			carry = _addcarryx_u64(carry, r[0], base[0], r + 0);
			carry = _addcarryx_u64(carry, r[1], base[1], r + 1);
			carry = _addcarryx_u64(carry, r[2], base[2], r + 2);
			carry = _addcarryx_u64(carry, r[3], base[3], r + 3);
			carry = _addcarryx_u64(carry, r[4], base[4], r + 4);
			carry = _addcarryx_u64(carry, r[5],       0, r + 5);

			fprintf(f, "[%04d]: 0x%016llX, 0x%016llX, 0x%016llX, 0x%016llX, 0x%016llX\n", i, r[4], r[3], r[2], r[1], r[0]);
		}

		fprintf(f, "\n\n");
	}

	fclose(f);
}
*/

void test_montg_inv_mod_n_ex() {
	UINT64 a[4] = {0x667EEB532EE03681, 0xEE8C369F729CDCB6, 0x10570E23664E918D, 0x01A5B14ABA92A216};

	UINT64 res[4] = {0};

	montg_inv_mod_n_ex(a, res);
	printBN(res, 4);

	MontgInvModn(a, res);
	printBN(res, 4);

	montg_inv_mod_p_ex(a, res);
	printBN(res, 4);

	MontgInvModp(a, res);
	printBN(res, 4);

}

int main(int argc, char* argv[])
{
	//test_raw_mul();
	//test_raw_pow();
	//test_solinas_reduce();
	//bench_solinas_reduce();
	//get_15_times_P();
	//get_15_times_rhoP();
	//bench_point_mul();
	//test_mul_mod_p();
	// bench_mul_mod_p();
	//test_sqr_mod_p();
	//bench_sqr_mod_p();
	//test_montg_op_mod_p();
	//test_double_point();
	//bench_double_point();
	//bench_JPoint_add();
	//test_point_mul();
	//test_montg_back();
	//test_Verify();
	//self_main_for_base_poit_mul_test();

	//test_times_point();
	//test_times_point2();
	//bench_times_point();
	//test_double_JPoint();

	//test_times_P();
	//self_main_test_point_mul();

	//self_main_for_bench();
	//self_main_for_inv_in_montg();
	//self_main_test_montg_op();

	//self_main_for_signer_verifier();
	//inter_validation_controlor(argc, argv);


	sm2_self_check();
	bench_sm2();
	// sm2_correctness();
	//test_mul_mod_p();
	// test_raw_pow();
	// test_montg_inv_mod_n_ex();
	return 0;
}
