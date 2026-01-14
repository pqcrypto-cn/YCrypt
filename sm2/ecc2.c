#include "include/ecc.h"

#define CACHE_LINE_SIZE (64)
#define MAX_W  (6)

int gsGetScrambleBufferSize(int modulusLen, int w)
{
	// size of resource to store 2^w values of modulusLen*sizeof(UINT64) each
	int size = (1 << w) * modulusLen * sizeof(UINT64);
	// padd it up to CACHE_LINE_SIZE 
	size += (CACHE_LINE_SIZE - (size % CACHE_LINE_SIZE)) % CACHE_LINE_SIZE;
	return size / sizeof(UINT64);
}

inline void gsScramblePut(UINT64* tbl, int idx, const UINT64* val, int vLen, int w)
{
	int width = 1 << w;
	int i, j;
	for (i = 0, j = idx; i<vLen; i++, j += width) 
	{
		tbl[j] = val[i];
	}
}

inline void gsScrambleGet(UINT64* val, int vLen, const UINT64* tbl, int idx, int w)
{
	int width = 1 << w;
	int i, j;
	for (i = 0, j = idx; i<vLen; i++, j += width) 
	{
		val[i] = tbl[j];
	}
}

// tests if MSB(a)==1 
inline UINT64 cpIsMsb_ct(UINT64 a)
{
	return (UINT64)0 - (a >> (sizeof(a) * 8 - 1));
}

// tests if a==0 
inline UINT64 cpIsZero_ct(UINT64 a)
{
	return cpIsMsb_ct(~a & (a - 1));
}

// tests if a==b 
inline UINT64 cpIsEqu_ct(UINT64 a, UINT64 b)
{
	return cpIsZero_ct(a ^ b);
}

inline void gsScrambleGet_sscm(UINT64* val, int vLen, const UINT64* tbl, int idx, int w)
{
	UINT64 mask[1 << MAX_W];

	int width = 1 << w;

	int n, i;
	switch (w) {
	case 6:
		for (n = 0; n<(1 << 6); n++)
			mask[n] = cpIsEqu_ct(n, idx);
		break;
	case 5:
		for (n = 0; n<(1 << 5); n++)
			mask[n] = cpIsEqu_ct(n, idx);
		break;
	case 4:
		for (n = 0; n<(1 << 4); n++)
			mask[n] = cpIsEqu_ct(n, idx);
		break;
	case 3:
		for (n = 0; n<(1 << 3); n++)
			mask[n] = cpIsEqu_ct(n, idx);
		break;
	case 2:
		for (n = 0; n<(1 << 2); n++)
			mask[n] = cpIsEqu_ct(n, idx);
		break;
	default:
		mask[0] = cpIsEqu_ct(0, idx);
		mask[1] = cpIsEqu_ct(1, idx);
		break;
	}

	for (i = 0; i<vLen; i++, tbl += width) {
		UINT64 acc = 0;

		switch (w) {
		case 6:
			for (n = 0; n<(1 << 6); n++)
				acc |= tbl[n] & mask[n];
			break;
		case 5:
			for (n = 0; n<(1 << 5); n++)
				acc |= tbl[n] & mask[n];
			break;
		case 4:
			for (n = 0; n<(1 << 4); n++)
				acc |= tbl[n] & mask[n];
			break;
		case 3:
			for (n = 0; n<(1 << 3); n++)
				acc |= tbl[n] & mask[n];
			break;
		case 2:
			for (n = 0; n<(1 << 2); n++)
				acc |= tbl[n] & mask[n];
			break;
		default:
			acc |= tbl[0] & mask[0];
			acc |= tbl[1] & mask[1];
			break;
		}

		val[i] = acc;
	}
}

// signed encode 
inline void booth_recode(uint8_t* sign, uint8_t* digit, uint8_t in, int w)
{
	uint8_t s = (uint8_t)(~((in >> w) - 1));
	int d = (1 << (w + 1)) - in - 1;
	d = (d & s) | (in & ~s);
	d = (d >> 1) + (d & 1);
	*sign = s & 1;
	*digit = (uint8_t)d;
}

// sscm version 
void setupTable(UINT64* pTbl, const UINT64* pPdata)
{
	int pointLen = 12;
	//int pointLen32 = pointLen*sizeof(UINT64)/sizeof(ipp32u);
	UINT64 buf[3 * 12];

	const int npoints = 3;
	UINT64* A = buf;
	UINT64* B = A + pointLen;
	UINT64* C = B + pointLen;
	AFPoint data2;

	jacobian_to_affine(pPdata, &data2);

	// Table[0]
	// Table[0] is implicitly (0,0,0) {point at infinity}, therefore no need to store it
	// All other values are actually stored with an offset of -1

	// Table[1] ( =[1]p )
	//cpScatter32((Ipp32u*)pTbl, 16, 0, (Ipp32u*)pPdata, pointLen32);
	gsScramblePut(pTbl, (1 - 1), pPdata, pointLen, (5 - 1));

	// Table[2] ( =[2]p )
	double_JPoint(pPdata, A);
	//gfec_point_double(A, pPdata, pEC);
	//cpScatter32((Ipp32u*)pTbl, 16, 1, (Ipp32u*)A, pointLen32);
	gsScramblePut(pTbl, (2 - 1), A, pointLen, (5 - 1));

	// Table[3] ( =[3]p )
	add_JPoint_and_AFPoint(A, &data2, B);  // Faster than add_JPoint()
	//add_JPoint(A, pPdata, B);
	//gfec_point_add(B, A, pPdata, pEC);
	//cpScatter32((Ipp32u*)pTbl, 16, 2, (Ipp32u*)B, pointLen32);
	gsScramblePut(pTbl, (3 - 1), B, pointLen, (5 - 1));

	// Table[4] ( =[4]p )
	double_JPoint(A, A);
	//gfec_point_double(A, A, pEC);
	//cpScatter32((Ipp32u*)pTbl, 16, 3, (Ipp32u*)A, pointLen32);
	gsScramblePut(pTbl, (4 - 1), A, pointLen, (5 - 1));

	// Table[5] ( =[5]p )
	add_JPoint_and_AFPoint(A, &data2, C);
	//add_JPoint(A, pPdata, C);
	//gfec_point_add(C, A, pPdata, pEC);
	//cpScatter32((Ipp32u*)pTbl, 16, 4, (Ipp32u*)C, pointLen32);
	gsScramblePut(pTbl, (5 - 1), C, pointLen, (5 - 1));

	// Table[10] ( =[10]p )
	double_JPoint(C, C);
	//gfec_point_double(C, C, pEC);
	//cpScatter32((Ipp32u*)pTbl, 16, 9, (Ipp32u*)C, pointLen32);
	gsScramblePut(pTbl, (10 - 1), C, pointLen, (5 - 1));

	// Table[11] ( =[11]p )
	add_JPoint_and_AFPoint(C, &data2, C);
	//add_JPoint(C, pPdata, C);
	//gfec_point_add(C, C, pPdata, pEC);
	//cpScatter32((Ipp32u*)pTbl, 16, 10, (Ipp32u*)C, pointLen32);
	gsScramblePut(pTbl, (11 - 1), C, pointLen, (5 - 1));

	// Table[6] ( =[6]p )
	double_JPoint(B, B);
	//gfec_point_double(B, B, pEC);
	//cpScatter32((Ipp32u*)pTbl, 16, 5, (Ipp32u*)B, pointLen32);
	gsScramblePut(pTbl, (6 - 1), B, pointLen, (5 - 1));

	// Table[7] ( =[7]p )
	add_JPoint_and_AFPoint(B, &data2, C);
	//add_JPoint(B, pPdata, C);
	//gfec_point_add(C, B, pPdata, pEC);
	//cpScatter32((Ipp32u*)pTbl, 16, 6, (Ipp32u*)C, pointLen32);
	gsScramblePut(pTbl, (7 - 1), C, pointLen, (5 - 1));

	// Table[14] ( =[14]p )
	double_JPoint(C, C);
	//gfec_point_double(C, C, pEC);
	//cpScatter32((Ipp32u*)pTbl, 16, 13, (Ipp32u*)C, pointLen32);
	gsScramblePut(pTbl, (14 - 1), C, pointLen, (5 - 1));

	// Table[15] ( =[15]p )
	add_JPoint_and_AFPoint(C, &data2, C);
	//add_JPoint(C, pPdata, C);
	//gfec_point_add(C, C, pPdata, pEC);
	//cpScatter32((Ipp32u*)pTbl, 16, 14, (Ipp32u*)C, pointLen32);
	gsScramblePut(pTbl, (15 - 1), C, pointLen, (5 - 1));

	// Table[12] ( =[12]p )
	double_JPoint(B, B);
	//gfec_point_double(B, B, pEC);
	//cpScatter32((Ipp32u*)pTbl, 16, 11, (Ipp32u*)B, pointLen32);
	gsScramblePut(pTbl, (12 - 1), B, pointLen, (5 - 1));

	// Table[13] ( =[13]p )
	add_JPoint_and_AFPoint(B, &data2, B);
	//add_JPoint(B, pPdata, B);
	//gfec_point_add(B, B, pPdata, pEC);
	//cpScatter32((Ipp32u*)pTbl, 16, 12, (Ipp32u*)B, pointLen32);
	gsScramblePut(pTbl, (13 - 1), B, pointLen, (5 - 1));

	// Table[8] ( =[8]p )
	double_JPoint(A, A);
	//gfec_point_double(A, A, pEC);
	//cpScatter32((Ipp32u*)pTbl, 16, 7, (Ipp32u*)A, pointLen32);
	gsScramblePut(pTbl, (8 - 1), A, pointLen, (5 - 1));

	// Table[9] ( =[9]p )
	add_JPoint_and_AFPoint(A, &data2, B);
	//add_JPoint(A, pPdata, B);
	//gfec_point_add(B, A, pPdata, pEC);
	//cpScatter32((Ipp32u*)pTbl, 16, 8, (Ipp32u*)B, pointLen32);
	gsScramblePut(pTbl, (9 - 1), B, pointLen, (5 - 1));

	// Table[16] ( =[16]p )
	double_JPoint(A, A);
	//gfec_point_double(A, A, pEC);
	//cpScatter32((Ipp32u*)pTbl, 16, 15, (Ipp32u*)A, pointLen32);
	gsScramblePut(pTbl, (16 - 1), A, pointLen, (5 - 1));

}



// replace under mask: dst[] = replaceFlag? src[] : dst[]
inline void cpMaskedReplace_ct(UINT64* dst, const UINT64* src, int len, UINT64 replaceMask)
{
	UINT64 dstMask = ~replaceMask;
	int n;
	for (n = 0; n < len; n++)
	{
		dst[n] = (src[n] & replaceMask) ^ (dst[n] & dstMask);
	}
}



// Mimic Intel IPP
void times_point2(const JPoint* point, const u32* times, JPoint* result)
{
	const UINT64 scalarBitSize = 256;
	uint8_t* pScalar8 = (uint8_t*)times->v;
	int pointLen = 12;  //  3 * 4  // Each JPoint has 3 coordinate, each coordinate has 4 elements (256bit)

	// optimal size of window
	const int window_size = 5;

	UINT64 buf[3 *12];

	// pre-computed table 
	UINT64 pTable[16 * 12] = { 0 }; // 16 Jacobian point
	setupTable(pTable, point);

	int elemLen = 4;  // 4 UINT64

	UINT64* pHy = buf;

	UINT64* pTdata = buf + pointLen; /* points from the pool */
	UINT64* pHdata = buf + 2 * pointLen;

	int wvalue;
	uint8_t digit, sign;
	int mask = (1 << (window_size + 1)) - 1;
	int bit = scalarBitSize - (scalarBitSize%window_size);

	// first window 
	if (bit) 
	{
		wvalue = *((uint16_t*)&pScalar8[(bit - 1) / 8]);
		wvalue = (wvalue >> ((bit - 1) % 8)) & mask;
	}
	else
	{
		wvalue = 0;
	}
	booth_recode(&sign, &digit, (uint8_t)wvalue, window_size);
	gsScrambleGet_sscm(pTdata, pointLen, pTable, digit - 1, 5 - 1);

	for (bit -= window_size; bit >= window_size; bit -= window_size) 
	{
		//gfec_point_double(pTdata, pTdata, pEC); /* probablyit's better to have separate calls */
		//gfec_point_double(pTdata, pTdata, pEC); /* instead of gfec_point_double_k() */
		//gfec_point_double(pTdata, pTdata, pEC);
		//gfec_point_double(pTdata, pTdata, pEC);
		//gfec_point_double(pTdata, pTdata, pEC);
		double_JPoint(pTdata, pTdata);
		double_JPoint(pTdata, pTdata);
		double_JPoint(pTdata, pTdata);
		double_JPoint(pTdata, pTdata);
		double_JPoint(pTdata, pTdata);

		wvalue = *((uint16_t*)&pScalar8[(bit - 1) / 8]);
		wvalue = (wvalue >> ((bit - 1) % 8)) & mask;
		booth_recode(&sign, &digit, (uint8_t)wvalue, window_size);
		gsScrambleGet_sscm(pHdata, pointLen, pTable, digit - 1, 5 - 1);

		neg_mod_p((u32*)(pHdata + elemLen), (u32*)pHy);
		//negF(pHy, pHdata + elemLen, pGFE);
		cpMaskedReplace_ct(pHdata + elemLen, pHy, elemLen, ~cpIsZero_ct(sign));
		add_JPoint(pTdata, pHdata, pTdata);
		//gfec_point_add(pTdata, pTdata, pHdata, pEC);
	}

	// last window 
	//gfec_point_double(pTdata, pTdata, pEC);
	//gfec_point_double(pTdata, pTdata, pEC);
	//gfec_point_double(pTdata, pTdata, pEC);
	//gfec_point_double(pTdata, pTdata, pEC);
	//gfec_point_double(pTdata, pTdata, pEC);
	double_JPoint(pTdata, pTdata);
	double_JPoint(pTdata, pTdata);
	double_JPoint(pTdata, pTdata);
	double_JPoint(pTdata, pTdata);
	double_JPoint(pTdata, pTdata);

	wvalue = *((uint16_t*)&pScalar8[0]);
	wvalue = (wvalue << 1) & mask;
	booth_recode(&sign, &digit, (uint8_t)wvalue, window_size);
	gsScrambleGet_sscm(pHdata, pointLen, pTable, digit - 1, 5 - 1);

	//negF(pHy, pHdata + elemLen, pGFE);
	neg_mod_p((u32*)(pHdata + elemLen), (u32*)pHy);
	cpMaskedReplace_ct(pHdata + elemLen, pHy, elemLen, ~cpIsZero_ct(sign));
	add_JPoint(pTdata, pHdata, result);
	//gfec_point_add(pTdata, pTdata, pHdata, pEC);
	//cpGFpElementCopy(pRdata, pTdata, pointLen);

}
