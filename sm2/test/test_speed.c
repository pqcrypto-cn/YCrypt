#include <stdio.h>
#include <time.h>
#include "../include/sm2.h"
#include "../include/utils.h"
#define MSG_LEN 10000

int bench_sm2()
{
	unsigned char message[MSG_LEN];
	unsigned char IDA[17] = "1234567812345678";
	UINT64 i = 0, loop;
	u1 ZA[32], dgst[32];
	PrivKey privkey;
	PubKey pubkey;
	SM2SIG sig;

	clock_t t1, t2;
	double diff, speed;

	srand((unsigned)time(NULL));
	random_fill(message, MSG_LEN);

	sm2_keypair(&pubkey, &privkey);

	//caculate dgst
	sm2_get_id_digest(ZA, IDA, strlen((char *)IDA), &pubkey);
	sm2_get_message_digest(dgst, ZA, message, MSG_LEN);

	loop = 500000;
	t1 = clock();
	for (i = 0; i < loop; i++)
	{
		sm2_sign_dgst(&sig, dgst, &privkey);
	}
	t2 = clock();
	diff = t2 - t1;
	diff /= CLOCKS_PER_SEC;
	speed = loop / diff;
	printf("total time: %f s\n", diff);
	printf("sm2 sign %f us\n", 1000000 / speed);
	printf("speed: %f times/s\n", speed);
	puts("");

	loop = 200000;
	t1 = clock();
	for (i = 0; i < loop; i++)
	{
		sm2_verify(&sig, dgst, &pubkey);
	}
	t2 = clock();
	diff = t2 - t1;
	diff /= CLOCKS_PER_SEC;
	speed = loop / diff;
	printf("total time: %f s\n", diff);
	printf("sm2 verify %f us\n", 1000000 / speed);
	printf("speed: %f times/s\n", speed);

	return 0;
}

int main() {
	bench_sm2();
	return 0;
}
