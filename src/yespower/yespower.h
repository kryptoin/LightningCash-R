
#ifndef _YESPOWER_H_
#define _YESPOWER_H_

#include <stdint.h>
#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	void *base, *aligned;
	size_t base_size, aligned_size;
} yespower_region_t;

typedef yespower_region_t yespower_local_t;

typedef enum { YESPOWER_0_5 = 5, YESPOWER_1_0 = 10 } yespower_version_t;

typedef struct {
	yespower_version_t version;
	uint32_t N, r;
	const uint8_t *pers;
	size_t perslen;
} yespower_params_t;

typedef struct {
	unsigned char uc[32];
} yespower_binary_t;

extern int yespower_init_local(yespower_local_t *local);

extern int yespower_free_local(yespower_local_t *local);

extern int yespower(yespower_local_t *local,
    const uint8_t *src, size_t srclen,
    const yespower_params_t *params, yespower_binary_t *dst);

extern int yespower_tls(const uint8_t *src, size_t srclen,
    const yespower_params_t *params, yespower_binary_t *dst);

#ifdef __cplusplus
}
#endif

#endif

