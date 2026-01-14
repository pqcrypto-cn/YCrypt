
#ifndef UTILS_H
#define UTILS_H

#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

#include "dataType.h"
#include "ecc.h"


// For C language
// static const uint8_t true = 1;
// static const uint8_t false = 0;
// typedef uint8_t bool;

void printBN(UINT64* a, size_t size);

void print_msg(const char *msg);

void err_msg(const char *msg);

void print_line();

void print_uint32(const uint32_t* data, size_t len);

void print_uchar(const uint8_t* data, size_t len);

void print_uchar_limited(const uint8_t* data, size_t len);


// Generate random data in len
void gen_rand_data(uint8_t* data, uint32_t len);

void print_u32(const u32* input);

void print_affine_point(const AFPoint* point);
void print_jacobian_point(const JPoint* point);
void print_AFPoint(AFPoint* p);
void print_JPoint(JPoint* p);


void random_fill(uint8_t* buffer, size_t len);

void random_fill_non_zero(uint8_t * buffer, size_t len);

int get_file_size(char* filename, uint32_t* outlen);

int read_from_file(char* filename, uint8_t* buf, uint32_t buflen);

int write_to_file(char* filename, uint8_t* buf, uint32_t buflen);

int delete_file(char* filename);

bool is_file_exist(char* filename);

#endif  // end of utility.h