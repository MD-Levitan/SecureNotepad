#ifndef __EC_H__
#define __EC_H__

#include "mpi.h"

#ifdef  __cplusplus
extern "C" {
#endif

#define EC_MAX_SEED_LEN 8

struct __ec_point {
	mp_int x;
	mp_int y;
	mp_int z;
};
typedef struct __ec_point ec_point;

struct __ec {
	mp_int field;				/* p */
	mp_int a;				/* a */
	mp_int b;				/* b */
	u_char seed[EC_MAX_SEED_LEN];	/* seed */
	u_long seed_len;			/* seed length */
	mp_int order;				/* q */
	u_long cofactor;			/* 1 */
	ec_point group;				/* G */
	mp_int two_inv_p;
};
typedef struct __ec ecc;

void ec_init(ecc *);
void ec_swap(ecc *, ecc *);
void ec_free(ecc *);
int ec_set_curve(ecc *, const mp_int *, const mp_int *, const mp_int *);
int ec_set_generator(ecc *, const ec_point *, const mp_int *, u_long);
int ec_set_seed(ecc *, const void *, u_long);
int ec_copy(ecc *, const ecc *);
u_long ec_get_degree(const ecc *);
int ec_is_equal(const ecc *, const ecc *);

void ecp_init(ec_point *);
void ecp_swap(ec_point *, ec_point *);
void ecp_free(ec_point *);
int ecp_set_affine(const ecc *, ec_point *, const mp_int *, const mp_int *);
int ecp_get_affine(const ecc *, const ec_point *, mp_int *, mp_int *);
int ecp_add(const ecc *, ec_point *, const ec_point *, const ec_point *);
int ecp_mul(const ecc *, ec_point *, const mp_int *, const ec_point *);
int ecp_is_at_infinity(const ec_point *);

#ifdef  __cplusplus
}
#endif

#endif /* __EC_H__ */
