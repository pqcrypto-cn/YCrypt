#include "include/ecc.h"
#include "include/sm2_table.h"

__attribute__((unused))
static void print_APoint(const AFPoint* point)
{
	const UINT64* x = point->x.v;
	const UINT64* y = point->y.v;
	printf("0x%016llX, 0x%016llX, 0x%016llX, 0x%016llX,\n", x[0], x[1], x[2], x[3]);
	printf("0x%016llX, 0x%016llX, 0x%016llX, 0x%016llX,\n", y[0], y[1], y[2], y[3]);
	puts("");
}

__attribute__((unused))
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

void ML_mul_basepoint(const u32* k, JPoint* result)
{
	int i = 0, j = 0;
	JPoint Tr = JPoint_ZERO;
	JPoint T = JPoint_ZERO;
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
			pTable = (AFPoint*)&(g_AFTable_for_base_point_mul[i][j][byte]);
			add_JPoint_and_AFPoint(&T, pTable, &T);
		}
		add_JPoint(&Tr, &T, &Tr);
	}

	// Save the result
	memcpy(result, &Tr, sizeof(JPoint));
}

