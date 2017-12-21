#ifndef __TZI_U128_H__
#define __TZI_U128_H__

#include "tzi_lib.h"

#ifdef  __cplusplus
extern "C" {
#endif

typedef unsigned long uint64_t;
typedef unsigned char uint8_t;

#define p2ul(n) ((unsigned long)(n))

static __inline void u128_mov_aligned(void *d, const void *s)
{
	((uint64_t *)d)[0] = ((uint64_t *)s)[0];
	((uint64_t *)d)[1] = ((uint64_t *)s)[1];
}

static __inline void u128_mov(void *d, const void *s)
{
	if ((p2ul(d) | p2ul(s)) % 8)
		tzi_memcpy(d, s, 16);
	else
		u128_mov_aligned(d, s);
}

static __inline void u128_xor_aligned(void *r, const void *a, const void *b)
{
	((uint64_t *)r)[0] = ((uint64_t *)a)[0] ^ ((uint64_t *)b)[0];
	((uint64_t *)r)[1] = ((uint64_t *)a)[1] ^ ((uint64_t *)b)[1];
}

static __inline void u128_xor_unaligned(uint8_t *r, const uint8_t *a,
					const uint8_t *b)
{
	int i;
	for(i = 0; i < 16; ++i)
		r[i] = a[i] ^ b[i];
}

static __inline void u128_xor(void *r, const void *a, const void *b)
{
	if ((p2ul(r) | p2ul(a) | p2ul(b)) % 8)
		u128_xor_unaligned(r, a, b);
	else
		u128_xor_aligned(r, a, b);
}

static __inline void u128_add_word(void  *__r, const void *__lh, uint64_t n)
{
	const uint64_t *lh = __lh;
	uint64_t *r = __r;
	n = (r[0] = lh[0] + n) < lh[0];
	r[1] = lh[1] + n;
}

static __inline void u128_sub_word(void *__r, const void *__lh, uint64_t n)
{
	const uint64_t *lh = __lh;
	uint64_t *r = __r;
	n = (r[0] = lh[0] - n) > lh[0];
	r[1] = lh[1] - n;
}

static __inline void u128_inc(void *r)
{
	int i;
	for(i = 0; i < 16; ++i) {
		if (++((uint8_t *) r)[i])
			break;
	}
}

#ifdef  __cplusplus
}
#endif

#endif /* __TZI_U128_H__ */
