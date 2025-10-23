
#ifndef _SYSENDIAN_H_
#define _SYSENDIAN_H_

#include <stdint.h>

#define be32dec libcperciva_be32dec
#define be32enc libcperciva_be32enc
#define be64enc libcperciva_be64enc
#define le32dec libcperciva_le32dec
#define le32enc libcperciva_le32enc

static inline uint32_t be32dec(const void *pp) {
  const uint8_t *p = (uint8_t const *)pp;

  return ((uint32_t)(p[3]) + ((uint32_t)(p[2]) << 8) +
          ((uint32_t)(p[1]) << 16) + ((uint32_t)(p[0]) << 24));
}

static inline void be32enc(void *pp, uint32_t x) {
  uint8_t *p = (uint8_t *)pp;

  p[3] = x & 0xff;
  p[2] = (x >> 8) & 0xff;
  p[1] = (x >> 16) & 0xff;
  p[0] = (x >> 24) & 0xff;
}

static inline void be64enc(void *pp, uint64_t x) {
  uint8_t *p = (uint8_t *)pp;

  p[7] = x & 0xff;
  p[6] = (x >> 8) & 0xff;
  p[5] = (x >> 16) & 0xff;
  p[4] = (x >> 24) & 0xff;
  p[3] = (x >> 32) & 0xff;
  p[2] = (x >> 40) & 0xff;
  p[1] = (x >> 48) & 0xff;
  p[0] = (x >> 56) & 0xff;
}

static inline uint32_t le32dec(const void *pp) {
  const uint8_t *p = (uint8_t const *)pp;

  return ((uint32_t)(p[0]) + ((uint32_t)(p[1]) << 8) +
          ((uint32_t)(p[2]) << 16) + ((uint32_t)(p[3]) << 24));
}

static inline void le32enc(void *pp, uint32_t x) {
  uint8_t *p = (uint8_t *)pp;

  p[0] = x & 0xff;
  p[1] = (x >> 8) & 0xff;
  p[2] = (x >> 16) & 0xff;
  p[3] = (x >> 24) & 0xff;
}

#endif
