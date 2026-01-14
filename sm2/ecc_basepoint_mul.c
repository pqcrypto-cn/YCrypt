#include "include/ecc.h"
#include "include/sm2_table.h"

static void print_APoint(const AFPoint* point)
{
	const UINT64* x = point->x.v;
	const UINT64* y = point->y.v;
	printf("0x%016llX, 0x%016llX, 0x%016llX, 0x%016llX,\n", x[0], x[1], x[2], x[3]);
	printf("0x%016llX, 0x%016llX, 0x%016llX, 0x%016llX,\n", y[0], y[1], y[2], y[3]);
	puts("");
}

static void print_JPoint(const JPoint* point)
{
	const UINT64* x = point->x.v;
	const UINT64* y = point->y.v;
	const UINT64* z = point->z.v;
	printf("0x%016llX, 0x%016llX, 0x%016llX, 0x%016llX,\n", x[0], x[1], x[2], x[3]);
	printf("0x%016llX, 0x%016llX, 0x%016llX, 0x%016llX,\n", y[0], y[1], y[2], y[3]);
	printf("0x%016llX, 0x%016llX, 0x%016llX, 0x%016llX,\n", z[0], z[1], z[2], z[3]);
	puts("");
}

static void get_point_for_64bit()
{
	// int i = 0;
	u32 a3 = { 0, 0, 0, 1 }; // 2^(64*3)
	u32 a2 = { 0, 0, 1, 0 }; // 2^(64*2)
	u32 a1 = { 0, 1, 0, 0 }; // 2^(64*1)
	// u32 a0 = { 1, 0, 0, 0 }; // 2^(64*0)

	JPoint jr1, jr2, jr3;
	AFPoint r1, r2, r3;

	{
		//2 ^ (64 * 1) * G =
		//0xE18BD546B5824517, 0x673891D791CAA486, 0xBA220B99DF9F9A14, 0x95AFBD1155C1DA54,
		//0x8E4450EB334ACDCB, 0xC3C7D1898A53F20D, 0x2EEE750F4053017C, 0xE8A6D82C517388C2,

		//2 ^ (64 * 2) * G =
		//0x0000000000000003, 0x00007FFA187309B6, 0xBC000000FFF4C9D9, 0x00007FFA00000031,
		//0x00007FFA18805AF8, 0x00007FFA683F9860, 0x0000000000000000, 0x00007FFA18712F70,

		//2 ^ (64 * 3) * G =
		//0xBC002E2CDE15D828, 0x00000000004552B3, 0x0000000000000000, 0x00007FFA683F9860,
		//0x0000EFDDC196937E, 0x00007FFA64ECDE00, 0x000000000045749F, 0x00007FFA64ECDE00,
	}
	

	// Prepare for base point multiplication
	gen_tables();

	// Multiply base point
	times_basepoint(&a1, &jr1);
	times_basepoint(&a2, &jr2);
	times_basepoint(&a3, &jr3);

	// Convert Jacobian point to AFPoint
	jacobian_to_affine(&jr1, &r1);
	jacobian_to_affine(&jr1, &r1);
	jacobian_to_affine(&jr1, &r1);

	puts("2^(64*1) * G = ");
	print_APoint(&r1);

	puts("2^(64*2) * G = ");
	print_APoint(&r2);

	puts("2^(64*3) * G = ");
	print_APoint(&r3);

}

static void get_first_P3()
{
	AFPoint p3 =
	{
		0xBC002E2CDE15D828, 0x00000000004552B3, 0x0000000000000000, 0x00007FFA683F9860,
		0x0000EFDDC196937E, 0x00007FFA64ECDE00, 0x000000000045749F, 0x00007FFA64ECDE00,
	};
	int i = 0;
	u32 k = { 1, 0, 0, 0 };  // k = 1
	JPoint jr;
	AFPoint r;


	{
		//2 ^ 0 * p3 =
		//0xBC002E2CDE15D828, 0x00000000004552B3, 0x0000000000000000, 0x00007FFA683F9860,
		//0x0000EFDDC196937E, 0x00007FFA64ECDE00, 0x000000000045749F, 0x00007FFA64ECDE00,

		//2 ^ 1 * p3 =
		//0x2F7C70BC95BB4392, 0x43DAE0125CFC0716, 0xEB4164AE054B7346, 0x171B90C4495B17D4,
		//0x6BAE4652ED1414C0, 0xB95D705F3A6E3395, 0xEB8A6BBCF69D7CBC, 0x539AEA385F8A76C9,

		//2 ^ 2 * p3 =
		//0xDB7632254D5F8670, 0xC44E99503C3DED33, 0xC319AE4EDFF73FF2, 0x90D68E2C97CAE069,
		//0xAA9807BEB2AD3C9A, 0x53298AA4601CB18E, 0xDCCEABA7037AC3B4, 0x768FC45383E15FCA,

		//2 ^ 3 * p3 =
		//0xC8B7C461C9178D7D, 0x8733460EA25A041B, 0xEC9E4AA578A62EEB, 0xEF39D1736A2332C6,
		//0x8255295F455319FE, 0x73CF9D2C361488B4, 0x124BC70EB761F54E, 0x8C297ACF5E65E763,

		//2 ^ 4 * p3 =
		//0xE855DF703AF749B8, 0x51A267C85570AFC7, 0xA96D253005D22C5E, 0xCEA82C72529D24A5,
		//0x66FAC093AADE30B3, 0x0DD674A298B4D090, 0xCE500617B883B6A2, 0x72ECBA744F18F35B,

		//2 ^ 5 * p3 =
		//0x0ECB8DF0855440E2, 0x698B486A13CC2422, 0xAF4FF67557389BE9, 0x54D200075046F0CB,
		//0x461DA611431AB78F, 0x3B9AAD36A2B55EEF, 0xD1E7C6478E0EC35F, 0x0FF12E83D44D2D38,

		//2 ^ 6 * p3 =
		//0xCBE40C9469DE40CD, 0x703256B414B9DEFF, 0x34AA39A6DC358862, 0xAB9AB3215B6480B8,
		//0x3F922843CD43EDA7, 0xA0448D9553F5EACA, 0xC46851FC95993DC5, 0x3EABFD9B63A4E7DA,

		//2 ^ 7 * p3 =
		//0x91E654B7F9E3EE3D, 0x8A85A00F5AA4CB2F, 0xB9F480D89720ACCD, 0x720F2534AA95352D,
		//0xC3D20AB403D2B7A4, 0x3CA7D4E395BC739C, 0x72769C2FF817EEE8, 0x31132FCD8625E780,
	}

	for (i = 0; i <= 7; i++)
	{
		times_point(&p3, &k, &jr);
		jacobian_to_affine(&jr, &r);

		printf("2^%d * p3 = \n", i);
		print_APoint(&r);

		k.v[0] <<= 1;   // Mul 2
	}
	puts("");
}

// Each point =  2^(8*m) * [2^(64*n) * P]; // n = 0, 1, 2, 3; m = 0, 1, 2 ...7
static void get_256_items_for_each_point(AFPoint* point, AFPoint result[256])
{
	int i = 0;
	u32 k = { 1, 0, 0, 0 };  // k = 1
	JPoint jr;
	AFPoint r, *pr = result;
	int pointSize = sizeof(AFPoint);  // 64 Bytes

	//puts("Point = ");
	//print_APoint(point);

	// The first item is zero
	memset(&(result[0]), 0, pointSize);

	for (i = 1; i < 256; i++)
	{
		times_point(point, &k, &jr);
		jacobian_to_affine(&jr, &r);

		//printf("%d * point = \n", i);
		//print_APoint(&r);

		// Save
		memcpy(pr + i, &r, pointSize);

		k.v[0] += 1;   //  add 1
	}
	//puts("");

}

// Each point was encoded into montgomery domain
// Each point =  2^(8*m) * [2^(64*n) * P]; // n = 0, 1, 2, 3; m = 0, 1, 2 ...7
static void montg_get_256_items_for_each_point(AFPoint* point, AFPoint result[256])
{
	int i = 0;
	u32 k = { 1, 0, 0, 0 };  // k = 1
	JPoint jr;
	AFPoint r, *pr = result;
	int pointSize = sizeof(AFPoint);  // 64 Bytes

	//puts("Point = ");
	//print_APoint(point);

	// The first item is zero
	memset(&(result[0]), 0, pointSize);

	for (i = 1; i < 256; i++)
	{
		//times_point(point, &k, &jr);
		//jacobian_to_affine(&jr, &r);
		montg_naive_times_point(point, &k, &jr);
		montg_jpoint_to_apoint(&jr, r.x.v, r.y.v);
		montg_apoint_to_montg(&r, &r);

		//printf("%d * point = \n", i);
		//print_APoint(&r);

		// Save
		memcpy(pr + i, &r, pointSize);

		k.v[0] += 1;   //  add 1
	}
	//puts("");

}

// Each point = 2^(64*n) * P // n = 0, 1, 2, 3
static void get_8_items_for_each_point(AFPoint* point, AFPoint result[8][256])
{
	int i = 0;
	u32 k = { 1, 0, 0, 0 };  // k = 1
	JPoint jr;
	AFPoint r;
	uint8_t* pData = (uint8_t*)(&(k.v[0]));

	puts("Point = ");
	print_APoint(point);

	
	// 8 items
	for (i = 0; i < 8; i++)
	{
		k.v[0] = 0;
		pData[i] = 1;  // 2^(8*i)

		times_point(point, &k, &jr);
		jacobian_to_affine(&jr, &r);

		//printf("2^(8*%d) * point = \n", i);
		//print_APoint(&r);

		// Get child 256 items
		get_256_items_for_each_point(&r, result[i]);

	}
	//puts("");

}

// Each point was encoded into montgomery domain
// Each point = 2^(64*n) * P // n = 0, 1, 2, 3
static void montg_get_8_items_for_each_point(AFPoint* point, AFPoint result[8][256])
{
	int i = 0;
	u32 k = { 1, 0, 0, 0 };  // k = 1
	JPoint jr;
	AFPoint r;
	uint8_t* pData = (uint8_t*)(&(k.v[0]));

	puts("Point = ");
	print_APoint(point);


	// 8 items
	for (i = 0; i < 8; i++)
	{
		k.v[0] = 0;
		pData[i] = 1;  // 2^(8*i)

		//times_point(point, &k, &jr);
		//jacobian_to_affine(&jr, &r);
		montg_naive_times_point(point, &k, &jr);
		montg_jpoint_to_apoint(&jr, r.x.v, r.y.v);
		montg_apoint_to_montg(&r, &r);

		//printf("2^(8*%d) * point = \n", i);
		//print_APoint(&r);

		// Get child 256 items
		montg_get_256_items_for_each_point(&r, result[i]);

	}
	//puts("");

}

// point = G
static void get_4_items_for_base_point(const AFPoint* point, AFPoint result[4][8][256])
{
	int i = 0;
	u32 k = { 1, 0, 0, 0 };  // k = 1
	JPoint jr;
	AFPoint r;

	puts("Point = ");
	print_APoint(point);


	// 4 items
	for (i = 0; i < 4; i++)
	{
		memset(&k, 0, 32);
		k.v[i] = 1;   //  2^(64*i)

		times_point(point, &k, &jr);
		jacobian_to_affine(&jr, &r);

		printf("2^(64*%d) * point = \n", i);
		print_APoint(&r);

		// Get child 8 items
		get_8_items_for_each_point(&r, result[i]);

	}
	//puts("");

}

// Each point was encoded into montgomery domain
// point = G
static void montg_get_4_items_for_base_point(const AFPoint* point, AFPoint result[4][8][256])
{
	int i = 0;
	u32 k = { 1, 0, 0, 0 };  // k = 1
	JPoint jr;
	AFPoint r;

	puts("Point = ");
	print_APoint(point);


	// 4 items
	for (i = 0; i < 4; i++)
	{
		memset(&k, 0, 32);
		k.v[i] = 1;   //  2^(64*i)

		//times_point(point, &k, &jr);
		//jacobian_to_affine(&jr, &r);
		montg_naive_times_point(point, &k, &jr);
		montg_jpoint_to_apoint(&jr, r.x.v, r.y.v);
		montg_apoint_to_montg(&r, &r);

		printf("2^(64*%d) * point = \n", i);
		print_APoint(&r);

		// Get child 8 items
		montg_get_8_items_for_each_point(&r, result[i]);

	}
	//puts("");

}

// Output the table to file
static void output_table_to_file(char filename[100], AFPoint result[4][8][256])
{
	int i, j, k;
	FILE* f = NULL;
	char buf[1000];
	int size = 0;
	AFPoint *p;
	UINT64 *x, *y;

	f = fopen(filename, "w");

	memset(buf, 0, sizeof(buf));
	sprintf(buf, "{\n");
	size = 2;
	fwrite(buf, 1, size, f);

	for (i = 0; i < 4; i++)
	{
		memset(buf, 0, sizeof(buf));
		sprintf(buf, "    {\n");
		size = 4 + 2;
		fwrite(buf, 1, size, f);
		for (j = 0; j < 8; j++)
		{
			memset(buf, 0, sizeof(buf));
			sprintf(buf, "        {\n");
			size = 8 + 2;
			fwrite(buf, 1, size, f);

			for (k = 0; k < 256; k++)
			{
				p = &(result[i][j][k]);
				x = p->x.v;
				y = p->y.v;

				memset(buf, 0, sizeof(buf));
				sprintf(buf, 
					"            {{0x%016llX, 0x%016llX, 0x%016llX, 0x%016llX, }, {0x%016llX, 0x%016llX, 0x%016llX, 0x%016llX, }},\n", 
					x[0], x[1], x[2], x[3],
					y[0], y[1], y[2], y[3]);
				size = 12 + 20 * 8 + 10;
				fwrite(buf, 1, size, f);
			}
			memset(buf, 0, sizeof(buf));
			sprintf(buf, "        },\n");
			size = 8 + 3;
			fwrite(buf, 1, size, f);
		}
		memset(buf, 0, sizeof(buf));
		sprintf(buf, "    },\n");
		size = 4 + 3;
		fwrite(buf, 1, size, f);
	}

	memset(buf, 0, sizeof(buf));
	sprintf(buf, "}\n");
	size = 2;
	fwrite(buf, 1, size, f);

	fclose(f);

}

// Gernerate the table
static void generate_big_table_for_basepoint_multiplication()
{
	// Size = 4 * 8 * 256 * 64 bytes = 524288 bytes = 512 KB
	AFPoint table[4][8][256] = { 0 };

	// File name to save the result
	char filename[100] = "Material\\result.txt";

	get_4_items_for_base_point(&SM2_G, table);

	output_table_to_file(filename, table);

}

// Gernerate the table for montgomery domain
static void montg_generate_big_table_for_basepoint_multiplication()
{
	// Size = 4 * 8 * 256 * 64 bytes = 524288 bytes = 512 KB
	AFPoint table[4][8][256] = { 0 };
	AFPoint zG;

	// File name to save the result
	char filename[100] = "Material\\montg_result.txt";

	montg_apoint_to_montg(&SM2_G, &zG);
	montg_get_4_items_for_base_point(&zG, table);

	output_table_to_file(filename, table);

}

void ML_mul_basepoint(const u32* k, JPoint* result)
{
	int i = 0, j = 0;
	JPoint Tr = JPoint_ZERO;
	JPoint T = JPoint_ZERO;
	JPoint tt;
	uint8_t* pByte = NULL, byte;
	AFPoint *pTable = NULL;

	for (i = 0; i < 4; i++)
	{
		pByte = (uint8_t*)(&(k->v[i]));

		// Reset T to JPoint_ZERO
		memset(&T, 0, sizeof(T));
		T.x.v[0] = 1;
		T.y.v[0] = 1;

		for (j = 0; j < 8; j++)
		{
			byte = pByte[j];
			// printf("byte: 0x%x\n", byte);
			pTable = (AFPoint*)&(g_AFTable_for_base_point_mul[i][j][byte]);
			// printf("i: %d, j: %d pTable:\n", i, j);
			// print_AFPoint(pTable);
			add_JPoint_and_AFPoint(&T, pTable, &T);
			// printf("i: %d, j: %d add res:\n", i, j);
			// print_JPoint(&T);
		}

		add_JPoint(&Tr, &T, &Tr);

	}

	// Save the result
	memcpy(result, &Tr, sizeof(JPoint));
}

/*
static void self_check_for_mul_basepoint()
{
	u32 k = { 0 };
	JPoint r1, r2;
	bool ret = 0;
	int i = 0;
	bool flag = false;

	srand((unsigned)time(NULL));
	gen_tables();

	for (i = 0; i < 1000; i++)
	{

		get_random_u32_in_mod_n(&k);

		//puts("k = ");
		//print_u32(&k);

		ML_mul_basepoint(&k, &r1);
		times_basepoint(&k, &r2);

		ret = equ_to_JPoint(&r1, &r2);
		if (ret == false)
		{
			puts("Not equal");

			puts("r1 = ");
			print_JPoint(&r1);

			puts("r2 = ");
			print_JPoint(&r2);

			flag = true;
			break;
		}
	}

	if (flag == true)
	{
		puts("self_check_for_mul_basepoint() failed!");
	}
	else
	{
		puts("self_check_for_mul_basepoint() success!");
	}

}

static void bench_mul_basepoint()
{
	u32 k = { 0 };
	JPoint r1, r2;
	size_t i = 0, loop;
	clock_t t1, t2;
	double diff, speed;

	srand((unsigned)time(NULL));
	gen_tables();

	get_random_u32_in_mod_n(&k);


	loop = 1000000;

	t1 = clock();
	for (i = 0; i < loop; i++)
	{
		ML_mul_basepoint(&k, &r1);
	}
	t2 = clock();
	diff = t2 - t1;
	diff /= CLOCKS_PER_SEC;
	speed = loop / diff;
	printf("ML_mul_basepoint times: %f s\n", 1 / speed);
	printf("speed: %f\n", speed);

	t1 = clock();
	for (i = 0; i < loop; i++)
	{
		times_basepoint(&k, &r2);
	}
	t2 = clock();
	diff = t2 - t1;
	diff /= CLOCKS_PER_SEC;
	speed = loop / diff;
	printf("times_basepoint times: %f s\n", 1 / speed);
	printf("speed: %f\n", speed);


}


static void test2()
{
	u32 k = { 0xc8, 0,0,0 };
	JPoint r1, r2;
	bool ret = 0;
	AFPoint ar;

	gen_tables();


	times_basepoint(&k, &r1);

	puts("r1 = ");
	print_JPoint(&r1);

	jacobian_to_affine(&r1, &ar);
	print_APoint(&ar);

}

static void test3()
{
	bool ret = false;
	JPoint x = JPoint_ZERO;
	AFPoint a = 
	{
		{ 0xEAE3D9A9D13A42ED, 0x2B2308F6484E1B38, 0x3DB7B24888C21F3A, 0xB692E5B574D55DA9, },
		{ 0xD186469DE295E5AB, 0xDB61AC1773438E6D, 0x5A924F85544926F9, 0xA175051B0F3FB613, },
	};
	JPoint y, z;

	affine_to_jacobian(&a, &z);

	add_JPoint_and_AFPoint(&x, &a, &y);

	ret = equ_to_JPoint(&y, &z);
	if (ret == true)
	{
		puts("Equal");
	}
	else
	{
		puts("Not Equal");
	}

}

void self_main_for_base_poit_mul_test()
{
	//get_point_for_64bit();
	//get_first_P3();
	
	
	//generate_big_table_for_basepoint_multiplication();
	montg_generate_big_table_for_basepoint_multiplication();

	//self_check_for_mul_basepoint();
	//bench_mul_basepoint();


	//test2();
	//test3();
}
*/
