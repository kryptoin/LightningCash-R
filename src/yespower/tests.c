
#include <stdio.h>
#include <string.h>

#include "yespower.h"

#undef TEST_PBKDF2_SHA256

#ifdef TEST_PBKDF2_SHA256
#include <assert.h>

#include "sha256.h"

static void print_PBKDF2_SHA256_raw(const char *passwd, size_t passwdlen,
    const char *salt, size_t saltlen, uint64_t c, size_t dkLen)
{
	uint8_t dk[64];
	size_t i;

	assert(dkLen <= sizeof(dk));

	printf("PBKDF2_SHA256(\"%s\", \"%s\", %llu, %llu) = ",
	    passwd, salt, (unsigned long long)c, (unsigned long long)dkLen);

	PBKDF2_SHA256((const uint8_t *) passwd, passwdlen,
	    (const uint8_t *) salt, saltlen, c, dk, dkLen);

	for (i = 0; i < dkLen; i++)
		printf("%02x%c", dk[i], i < dkLen - 1 ? ' ' : '\n');
}

static void print_PBKDF2_SHA256(const char *passwd,
    const char *salt, uint64_t c, size_t dkLen)
{
	print_PBKDF2_SHA256_raw(passwd, strlen(passwd), salt, strlen(salt), c,
	    dkLen);
}
#endif

static const char *pers_bsty_magic = "BSTY";

static void print_yespower(yespower_version_t version, uint32_t N, uint32_t r,
    const char *pers)
{
	yespower_params_t params = {
		.version = version,
		.N = N,
		.r = r,
		.pers = (const uint8_t *)pers,
		.perslen = pers ? strlen(pers) : 0
	};
	uint8_t src[80];
	yespower_binary_t dst;
	size_t i;

	const char *q = (pers && pers != pers_bsty_magic) ? "\"": "";
	printf("yespower(%u, %u, %u, %s%s%s) = ", (unsigned int)version, N, r,
	    q, pers ? pers : "NULL", q);

	for (i = 0; i < sizeof(src); i++)
		src[i] = i * 3;

	if (pers == pers_bsty_magic) {
		params.pers = src;
		params.perslen = sizeof(src);
	}

	if (yespower_tls(src, sizeof(src), &params, &dst)) {
		puts("FAILED");
		return;
	}

	for (i = 0; i < sizeof(dst); i++)
		printf("%02x%c", dst.uc[i], i < sizeof(dst) - 1 ? ' ' : '\n');
}

static void print_yespower_loop(yespower_version_t version, const char *pers)
{
	uint32_t N, r;
	uint8_t src[80];
	yespower_binary_t dst, xor = {{0}};
	size_t i;

	printf("XOR of yespower(%u, ...) = ", (unsigned int)version);

	for (i = 0; i < sizeof(src); i++)
		src[i] = i * 3;

	for (N = 1024; N <= 4096; N <<= 1) {
		for (r = 8; r <= 32; r++) {
			yespower_params_t params = {
				.version = version,
				.N = N,
				.r = r,
				.pers = (const uint8_t *)pers,
				.perslen = pers ? strlen(pers) : 0
			};
			if (yespower_tls(src, sizeof(src), &params, &dst)) {
				puts("FAILED");
				return;
			}
			for (i = 0; i < sizeof(xor); i++)
				xor.uc[i] ^= dst.uc[i];
		}
	}

	for (i = 0; i < sizeof(xor); i++)
		printf("%02x%c", xor.uc[i], i < sizeof(xor) - 1 ? ' ' : '\n');
}

int main(void)
{
	setvbuf(stdout, NULL, _IOLBF, 0);

#ifdef TEST_PBKDF2_SHA256
	print_PBKDF2_SHA256("password", "salt", 1, 20);
	print_PBKDF2_SHA256("password", "salt", 2, 20);
	print_PBKDF2_SHA256("password", "salt", 4096, 20);
	print_PBKDF2_SHA256("password", "salt", 16777216, 20);
	print_PBKDF2_SHA256("passwordPASSWORDpassword",
	    "saltSALTsaltSALTsaltSALTsaltSALTsalt", 4096, 25);
	print_PBKDF2_SHA256_raw("pass\0word", 9, "sa\0lt", 5, 4096, 16);
#if 0
	print_PBKDF2_SHA256("password", "salt", 1, 32);
	print_PBKDF2_SHA256("password", "salt", 2, 32);
	print_PBKDF2_SHA256("password", "salt", 4096, 32);
	print_PBKDF2_SHA256("password", "salt", 16777216, 32);
	print_PBKDF2_SHA256("passwordPASSWORDpassword",
	    "saltSALTsaltSALTsaltSALTsaltSALTsalt", 4096, 40);
	print_PBKDF2_SHA256("password", "salt", 4096, 16);
	print_PBKDF2_SHA256("password", "salt", 1, 20);
	print_PBKDF2_SHA256("password", "salt", 2, 20);
	print_PBKDF2_SHA256("password", "salt", 4096, 20);
	print_PBKDF2_SHA256("password", "salt", 16777216, 20);
	print_PBKDF2_SHA256("password", "salt", 4096, 25);
	print_PBKDF2_SHA256("password", "salt", 4096, 16);
#endif
#endif

	print_yespower(YESPOWER_0_5, 2048, 8, "Client Key");

	print_yespower(YESPOWER_0_5, 2048, 8, pers_bsty_magic);

	print_yespower(YESPOWER_0_5, 4096, 16, "Client Key");

	print_yespower(YESPOWER_0_5, 4096, 24, "Jagaricoin");
	print_yespower(YESPOWER_0_5, 4096, 32, "WaviBanana");
	print_yespower(YESPOWER_0_5, 2048, 32, "Client Key");
	print_yespower(YESPOWER_0_5, 1024, 32, "Client Key");

	print_yespower(YESPOWER_0_5, 2048, 8, NULL);

	print_yespower(YESPOWER_1_0, 2048, 8, NULL);
	print_yespower(YESPOWER_1_0, 4096, 16, NULL);
	print_yespower(YESPOWER_1_0, 4096, 32, NULL);
	print_yespower(YESPOWER_1_0, 2048, 32, NULL);
	print_yespower(YESPOWER_1_0, 1024, 32, NULL);

	print_yespower(YESPOWER_1_0, 1024, 32, "personality test");

	print_yespower_loop(YESPOWER_0_5, "Client Key");
	print_yespower_loop(YESPOWER_1_0, NULL);

	return 0;
}
