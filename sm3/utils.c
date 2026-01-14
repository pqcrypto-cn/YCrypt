#include "include/utils.h"

/**************************
*   Hex utils from GmSSL
***************************/
static int hexchar2int(char c)
{
	if      ('0' <= c && c <= '9') return c - '0';
	else if ('a' <= c && c <= 'f') return c - 'a' + 10;
	else if ('A' <= c && c <= 'F') return c - 'A' + 10;
	else return -1;
}

int hex2bin(const char *in, size_t inlen, uint8_t *out)
{
	int c;
	if (inlen % 2) {
		return -1;
	}

	while (inlen) {
		if ((c = hexchar2int(*in++)) < 0) {
			return -1;
		}
		*out = (uint8_t)c << 4;
		if ((c = hexchar2int(*in++)) < 0) {
			return -1;
		}
		*out |= (uint8_t)c;
		inlen -= 2;
		out++;
	}
	return 1;
}

int hex_to_bytes(const char *in, size_t inlen, uint8_t *out, size_t *outlen)
{
	*outlen = inlen/2;
	return hex2bin(in, inlen, out);
}