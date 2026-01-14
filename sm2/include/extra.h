#ifndef EXTRA_H
#define EXTRA_H

#include "dataType.h"
#include "sm3.h"


void erase_data(void* buf, uint32_t buflen);
void u32_rand(u32* input);
void str_reverse_in_place(u1 *str, int len);
void u1_to_u32(u1 const input[32], u32* result);
int KDF(u1 Z[], u8 zlen, u8 klen, u1 K[]);

#endif


