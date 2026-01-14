#ifndef DATATYPE_H
#define DATATYPE_H

#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#define UINT64 unsigned long long
typedef unsigned long long	u8;
typedef unsigned int		u4;
typedef unsigned short		u2;
typedef unsigned char		u1;

typedef struct
{
	u8 v[4];
} u32;

typedef struct
{
	u32 x;
	u32 y;
} AFPoint;

typedef struct
{
	u32 x;
	u32 y;
	u32 z;
} JPoint;

typedef struct SM2SIG
{
    u32 r;
    u32 s;
}SM2SIG;

typedef struct PrivKey
{
    u32 da;
}PrivKey;

typedef AFPoint PubKey;

#endif