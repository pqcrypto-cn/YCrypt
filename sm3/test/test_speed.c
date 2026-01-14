#include <time.h>
#include "../include/sm3.h"

#define LARGE_BUFFER_SIZE (100 * 1024 * 1024UL) // 100 MB

void sm3_benchmark()
{
	puts("======== Bench SM3 ========");
	size_t i = 0;
	clock_t start = 0, end = 0;
	double diff = 0, speed = 0;
	uint8_t digest[32] = { 0 };

	uint8_t *g_in = (uint8_t*)malloc(LARGE_BUFFER_SIZE*8);

	for (i = 0; i < 5*LARGE_BUFFER_SIZE; i++)
	{
		g_in[i] = i & 0xFF;
	}

	start = clock();
	sm3(g_in, 5*LARGE_BUFFER_SIZE, digest);
	end = clock();
	diff = end - start;
	diff = diff / CLOCKS_PER_SEC;
	speed = (5.0 * LARGE_BUFFER_SIZE / 1024 / 1024) / diff;
	printf("start = %lu\n", start);
	printf("end = %lu\n", end);
	printf("diff = %.03f\n", diff);

	printf("Hash %ldMB data in %5.3f second!\n", (5* LARGE_BUFFER_SIZE / 1024 / 1024), diff);
	printf("SM3 speed: %.03f MB/sec\n\n", speed);
}


void sm3_hmac_benchmark() {

	puts("======== Bench SM3-HMAC ========");

    clock_t start = 0, end = 0;
    double diff = 0, speed = 0;
    u1 key[] = "777";
	size_t keylen = strlen((char *)key);
    u1 mac[32] = {0};

    // for (i = 0; i < LARGE_BUFFER_SIZE; i++) {
    //     data[i] = i & 0xFF;
    // }

	uint8_t *g_in = (uint8_t*)malloc(LARGE_BUFFER_SIZE*8);


    start = clock();
    sm3_hmac(g_in, 5*LARGE_BUFFER_SIZE, key, keylen, mac);
    end = clock();
    diff = end - start;
    diff = diff / CLOCKS_PER_SEC;
    speed = (5.0 * LARGE_BUFFER_SIZE / 1024 / 1024) / diff;
    
    printf("start = %lu\n", start);
    printf("end = %lu\n", end);
    printf("diff = %.03f\n", diff);

    printf("HMAC %ldMB data with a %ld-byte key in %5.3f seconds!\n", (5 * LARGE_BUFFER_SIZE / 1024 / 1024), keylen, diff);
    printf("SM3-HMAC speed: %.03f MB/sec\n\n", speed);
}

int main()
{
	sm3_benchmark();
	sm3_hmac_benchmark();
	return 0;
}