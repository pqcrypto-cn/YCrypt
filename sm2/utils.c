#include "include/utils.h"

void printBN(UINT64* a, size_t size) {
	for (size_t i = 0; i < size; i++) {
		printf("0x%llx\n", a[i]);
	}
	puts("");
}

void print_msg(const char *msg)
{
	puts(msg);
}

void err_msg(const char *msg)
{
	puts(msg);
}

void print_line()
{
	print_msg("--------------------------------------------------------------------------------------------");
}

void print_uint32(const uint32_t* data, size_t len)
{
	size_t i = 0;
	for (i = 0; i < len; ++i)
	{
		printf("0x%08X ", data[i]);
		if ((i + 1) % 4 == 0)
		{
			puts("");
		}
	}
	if (len % 4 != 0)
	{
		puts("");
	}
	puts("");
}

void print_uchar(const uint8_t* data, size_t len)
{
	size_t i = 0;
	for (i = 0; i < len; ++i)
	{
		printf("0x%02X ", data[i]);
		if ((i + 1) % 16 == 0)
		{
			puts("");
		}
	}
	if (len % 16 != 0)
	{
		puts("");
	}

	puts("");
}

void print_uchar_limited(const uint8_t* data, size_t len)
{
	size_t limited = 4 * 16;
	size_t i = 0;
	for (i = 0; i < len && i < limited; ++i)
	{
		printf("0x%02X ", data[i]);
		if ((i + 1) % 16 == 0)
		{
			puts("");
		}
	}
	if (i % 16 != 0)
	{
		puts("");
	}

	if (len > limited)
	{
		puts("......");
	}

	puts("");
}


// Generate random data in len
void gen_rand_data(uint8_t* data, uint32_t len)
{
	uint32_t i = 0;
	for (i = 0; i < len; i++)
	{
		data[i] = (uint8_t)(rand());
	}
}

//#define U256_DISPLAY
void print_u32(const u32* input)
{
#ifdef U256_DISPLAY
	printf("%016llx%016llx%016llx%016llx\n", (UINT64)input->v[3], (UINT64)input->v[2], (UINT64)input->v[1], (UINT64)input->v[0]);
#else
	printf("0x%016llX, 0x%016llX, 0x%016llX, 0x%016llX\n", (UINT64)input->v[0], (UINT64)input->v[1], (UINT64)input->v[2], (UINT64)input->v[3]);
#endif
}

void print_u64(const u8 input[8])
{
	for (u4 i = 0; i < 8; i++)
	{
		printf("0x%016llX, ", (UINT64)input[i]);
	}
	printf("\n");

}

void print_u1(const u1* data, u4 len)
{
	for (u4 i = 0; i < len; i++)
	{
		printf("%02X", data[i]);
		if ((i % 16) == 15 && (i != (len - 1)))
		{
			puts("");
		}
	}
	puts("");
}

void print_affine_point(const AFPoint* point)
{
	printf("%016llX %016llX %016llX %016llX\n", (UINT64)point->x.v[3], (UINT64)point->x.v[2], (UINT64)point->x.v[1], (UINT64)point->x.v[0]);
	printf("%016llX %016llX %016llX %016llX\n", (UINT64)point->y.v[3], (UINT64)point->y.v[2], (UINT64)point->y.v[1], (UINT64)point->y.v[0]);
}

void print_jacobian_point(const JPoint* point)
{
	printf("0x%016llX, 0x%016llX, 0x%016llX, 0x%016llX\n", (UINT64)point->x.v[3], (UINT64)point->x.v[2], (UINT64)point->x.v[1], (UINT64)point->x.v[0]);
	printf("0x%016llX, 0x%016llX, 0x%016llX, 0x%016llX\n", (UINT64)point->y.v[3], (UINT64)point->y.v[2], (UINT64)point->y.v[1], (UINT64)point->y.v[0]);
	printf("0x%016llX, 0x%016llX, 0x%016llX, 0x%016llXx\n", (UINT64)point->z.v[3], (UINT64)point->z.v[2], (UINT64)point->z.v[1], (UINT64)point->z.v[0]);
}


void print_AFPoint(AFPoint* p)
{
	int i = 0;
	puts("x = ");
	for (i = 0; i < 4; i++)
	{
		printf("0x%016llX, ", p->x.v[i]);
	}
	puts("");

	puts("y = ");
	for (i = 0; i < 4; i++)
	{
		printf("0x%016llX, ", p->y.v[i]);
	}
	puts("");
	puts("");
}

void print_JPoint(JPoint* p)
{
	int i = 0;
	puts("x = ");
	for (i = 0; i < 4; i++)
	{
		printf("0x%016llX, ", p->x.v[i]);
	}
	puts("");

	puts("y = ");
	for (i = 0; i < 4; i++)
	{
		printf("0x%016llX, ", p->y.v[i]);
	}
	puts("");

	puts("z = ");
	for (i = 0; i < 4; i++)
	{
		printf("0x%016llX, ", p->z.v[i]);
	}
	puts("");
	puts("");
}


void random_fill_non_zero(uint8_t * buffer, size_t len)
{
	uint32_t i = 0;

	for (i = 0; i < len; i++)
	{
		do
		{
			buffer[i] = rand() & 0xff;

		} while (buffer[i] == 0);
	}
}

void random_fill(uint8_t * buffer, size_t len)
{
	UINT64 i = 0, len2, res;
	uint16_t* p2 = (uint16_t*)buffer;

	srand((unsigned)time(NULL));
	
	len2 = len / 2;
	res = len & 1;

	for (i = 0; i < len2; i++)
	{
		p2[i] = (uint16_t)rand();
	}

	// The last byte
	if (res)
	{
		*(uint8_t*)(&p2[i]) = (uint8_t)rand();
	}
}

int get_file_size(char* filename, uint32_t* outlen)
{
	int status = -1;
	uint32_t filesize = 0;
	FILE* f = NULL;

	do
	{
		if (filename == NULL)
		{
			break;
		}

		f = fopen(filename, "rb");
		if (f == NULL)
		{
			// Open file faild
			break;
		}

		// Set the file pointer to the end
		if (fseek(f, 0, SEEK_END) != 0)
		{
			break;
		}

		// Get the file size
		filesize = ftell(f);
		if (outlen != NULL)
		{
			*outlen = filesize;
		}

		// success
		status = 0;

	} while (0);

	if (f != NULL) fclose(f);
	f = NULL;

	return status;
}

int read_from_file(char* filename, uint8_t* buf, uint32_t buflen)
{
	int status = -1;
	uint32_t filesize = 0;
	FILE* f = NULL;


	do
	{
		if (filename == NULL || buf == NULL)
		{
			break;
		}

		// Validate file size
		status = get_file_size(filename, &filesize);
		if (status != 0 || filesize < buflen)
		{
			break;
		}

		f = fopen(filename, "rb");
		if (f == NULL)
		{
			// Open file faild
			break;
		}

		filesize = fread(buf, 1, buflen, f);
		if (filesize != buflen)
		{
			// Write file failed
			break;
		}

		// success
		status = 0;

	} while (0);

	if (f != NULL) fclose(f);
	f = NULL;

	return status;
}

int write_to_file(char* filename, uint8_t* buf, uint32_t buflen)
{
	int status = -1;
	uint32_t filesize = 0;
	FILE* f = NULL;

	do
	{
		if (filename == NULL || buf == NULL)
		{
			break;
		}

		f = fopen(filename, "wb");
		if (f == NULL)
		{
			// Open file faild
			break;
		}

		filesize = fwrite(buf, 1, buflen, f);
		if (filesize != buflen)
		{
			// Write file failed
			break;
		}

		// success
		status = 0;

	} while (0);

	if (f != NULL) fclose(f);
	f = NULL;

	return status;
}

int delete_file(char* filename)
{
	int status = -1;

	status = remove(filename);

	return status;
}

bool is_file_exist(char* filename)
{
	bool status = false;
	FILE* f = NULL;

	// If we can open this file, then this file is exists
	f = fopen(filename, "rb");
	if (f != NULL)
	{
		status = true;
		fclose(f);
		f = NULL;
	}

	return status;
}


