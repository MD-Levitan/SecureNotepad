#ifndef __TZI_MPI_H__
#define __TZI_MPI_H__

#include <stdint.h>
#include "tzi.h"
#include "tzi_lib.h"
#include "belt.h"

#ifdef  __cplusplus
extern "C" {
#endif

#define MP_CHK(__r, __f, __t) do { if (((__r) = (__f)) != 0) goto __t; } while(0)

#if defined(_WIN32)

#define __SIZEOF_LONG__ 4
typedef uint32_t mp_limb;
typedef int32_t mp_slimb;
typedef uint64_t __uint64_t;

#else

typedef u_long mp_limb;
typedef long mp_slimb;

#endif

typedef struct __mp_int {
	mp_limb *p;
	u_long n;
	int s;
} mp_int;

void mp_init(mp_int *);
void mp_init_multi(mp_int *, ...);
void mp_free(mp_int *);
void mp_free_multi(mp_int *, ...);
int mp_grow(mp_int *, u_long);
int mp_shrink(mp_int *, u_long);
int mp_copy(mp_int *, const mp_int *);
void mp_swap(mp_int *, mp_int *);
int mp_safe_cond_assign(mp_int *, const mp_int *, u_char);
int mp_safe_cond_swap(mp_int *, mp_int *, u_char);
int mp_lset(mp_int *, mp_slimb);
int mp_get_bit(const mp_int *, u_long);
int mp_set_bit(mp_int *, u_long, u_char);
u_long mp_lsb(const mp_int *);
u_long mp_bitlen(const mp_int *);
u_long mp_size(const mp_int *);
int mp_read_string(mp_int *, int, const char *);
int mp_write_string(const mp_int *, int, char *, u_long, u_long *);
int mp_read_binary(mp_int *, const void *, u_long, int);
int mp_write_binary(const mp_int *, void *, u_long, int);
int mp_shift_l(mp_int *, u_long);
int mp_shift_r(mp_int *, u_long);
int mp_cmp_abs(const mp_int *, const mp_int *);
int mp_cmp(const mp_int *, const mp_int *);
int mp_cmp_int(const mp_int *, mp_slimb);
int mp_add_abs(mp_int *, const mp_int *, const mp_int *);
int mp_sub_abs(mp_int *, const mp_int *, const mp_int *);
int mp_add(mp_int *, const mp_int *, const mp_int *);
int mp_sub(mp_int *, const mp_int *, const mp_int *);
int mp_add_int(mp_int *, const mp_int *, mp_slimb);
int mp_sub_int(mp_int *, const mp_int *, mp_slimb);
int mp_mul(mp_int *, const mp_int *, const mp_int *);
int mp_mul_int(mp_int *, const mp_int *, mp_limb);
int mp_div(mp_int *, mp_int *, const mp_int *, const mp_int *);
int mp_div_int(mp_int *, mp_int *, const mp_int *, mp_slimb);
int mp_mod(mp_int *, const mp_int *, const mp_int *);
int mp_mod_int(mp_limb *, const mp_int *, mp_slimb);
int mp_exp_mod(mp_int *, const mp_int *, const mp_int *, const mp_int *, mp_int *);
int mp_gcd(mp_int *, const mp_int *, const mp_int *);
int mp_random(mp_int *, u_long, int (*)(void *, u_char *, u_long), void *);
int mp_inv_mod(mp_int *, const mp_int *, const mp_int *);
int mp_inv_int_mod(mp_int *, mp_limb, const mp_int *);
int mp_is_prime(const mp_int *, int (*)(void *, u_char *, u_long), void *);
int mp_gen_prime(mp_int *, u_long, int, int (*)(void *, u_char *, u_long), void *);
int mp_legendre(const mp_int *, const mp_int *, int);
int mp_md_step(struct hbelt_ctx *, const mp_int *, int, int);

#ifdef  __cplusplus
}
#endif

#endif /* __TZI_MPI_H__ */
