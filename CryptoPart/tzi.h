#ifndef __TZI_H__
#define __TZI_H__

#include "tzi_extra.h"

#ifdef  __cplusplus
extern "C" {
#endif

#define TZI_MD_MAX_BLOCK_SIZE 128
#define TZI_MAC_MAX_KEY_LEN 32
#define TZI_BC_MAC_BLOCK_SIZE 32


extern void tzi_init(void *(*)(unsigned long), void (*)(void *));

extern int (*entropy_f)(void *, unsigned char *, unsigned long);
extern void *entropy_p;


extern const struct md_desc bash256;
extern const struct md_desc bash384;
extern const struct md_desc bash512;
extern const struct md_desc belt_mac;
extern const struct bc_desc belt_ecb;
extern const struct bc_desc belt_cbc;
extern const struct bc_desc belt_cfb;
extern const struct bc_desc belt_ctr;
extern const struct bc_desc belt_dwr;
extern const struct bc_desc belt_kwr;
extern const struct rng_desc brng_ctr;
extern const struct rng_desc brng_hmac;

#ifdef  __cplusplus
}
#endif

#endif /* __TZI_H__ */
