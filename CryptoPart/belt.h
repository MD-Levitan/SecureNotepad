
#ifndef __BELT_H_H
#define __BELT_H_H



#define MAX_BLOCK_SIZE 128
#define MAC_BLOCK_SIZE 32


#include "tzi.h"
#include "tzi_u128.h"
#include "tzi_lib.h"
#include <stdint.h>

#define _CHK(__r, __f, __t) do { if (((__r) = (__f)) != 0) goto __t; } while(0)

struct hbelt_ctx {
    uint32_t h[8];
    uint32_t s[4];
    uint32_t c[4];
    uint32_t t[16];
    unsigned char buf[MAX_BLOCK_SIZE * 2];
    unsigned long buf_len;
    
} hbelt;

extern void hbelt_init(struct hbelt_ctx *ctx);
extern void hbelt_step_(struct hbelt_ctx *ctx, const u_char *dat);
extern void hbelt_step(struct hbelt_ctx *ctx, const u_char *in, u_long inl);
extern void hbelt_fini(struct hbelt_ctx *ctx, u_char *md);

extern void belt_block(const void *key, const void *x, void *y);


#define KWR_CTRL_SET_KEY 0x07
#define KWR_CTRL_SET_IV  0x08
struct kwr_ctx {
    uint32_t key[8];
    unsigned char iv[MAC_BLOCK_SIZE];
    unsigned char buf[MAC_BLOCK_SIZE];
    unsigned long buf_len;
};

extern int belt_kwr_ctrl(struct kwr_ctx *ctx, int cmd, void *dat, u_long len);
extern int belt_kwr_estep(struct kwr_ctx *ctx, u_char *out, const u_char *in, u_long inl);
extern int belt_kwr_dstep(struct kwr_ctx *ctx, u_char *out, const u_char *in, u_long inl);
extern void belt_kwr_init(struct kwr_ctx *ctx);
extern void belt_kwr_clear(struct kwr_ctx *ctx);
extern u_long belt_kwr_len(u_long mlen, int e);





#endif //__BELT_H_H
