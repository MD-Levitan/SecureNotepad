#include "ec.h"

static int ec_pow_int_mod(mp_int *res, const mp_int *base, u_long expo, const ecc *ec)
{
	mp_limb p[1];
	mp_int t;

	*p = expo;
	t.s = 1;
	t.n = 1;
	t.p = p;

	return mp_exp_mod(res, base, &t, &ec->field, 0);
}

static int ec_subm(mp_int *res, const mp_int *a, const mp_int *b, const ecc *ec)
{
	mp_int t;
	int rv;

	mp_init(&t);
	MP_CHK(rv, mp_sub(&t, a, b), out);
	MP_CHK(rv, mp_mod(&t, &t, &ec->field), out);
	mp_swap(res, &t);
out:
	mp_free(&t);
	return rv;
}

static int ec_addm(mp_int *res, const mp_int *a, const mp_int *b, const ecc *ec)
{
	mp_int t;
	int rv;

	mp_init(&t);
	MP_CHK(rv, mp_add(&t, a, b), out);
	MP_CHK(rv, mp_mod(&t, &t, &ec->field), out);
	mp_swap(res, &t);
out:
	mp_free(&t);
	return rv;
}

static int ec_mulm(mp_int *res, const mp_int *a, const mp_int *b, const ecc *ec)
{
	mp_int t;
	int rv;

	mp_init(&t);
	MP_CHK(rv, mp_mul(&t, a, b), out);
	MP_CHK(rv, mp_mod(&t, &t, &ec->field), out);
	mp_swap(res, &t);
out:
	mp_free(&t);
	return rv;
}

static int ec_mul_int_mod(mp_int *res, const mp_int *a, u_long b, const ecc *ec)
{
	mp_limb p[1];
	mp_int t;

	*p = b;
	t.s = 1;
	t.n = 1;
	t.p = p;

	return ec_mulm(res, a, &t, ec);
}

static int ec_invm(mp_int *res, const mp_int *a, const ecc *ec)
{
	return mp_inv_mod(res, a, &ec->field);
}

void ecp_init(ec_point *ecp)
{
	mp_init_multi(&ecp->x, &ecp->y, &ecp->z, 0ul);
}

static int ecp_copy(ec_point *d, const ec_point *s)
{
	int rv;
	MP_CHK(rv, mp_copy(&d->x, &s->x), out);
	MP_CHK(rv, mp_copy(&d->y, &s->y), out);
	MP_CHK(rv, mp_copy(&d->z, &s->z), out);
out:
	return rv;
}

void ecp_swap(ec_point *d, ec_point *s)
{
	mp_swap(&d->x, &s->x);
	mp_swap(&d->y, &s->y);
	mp_swap(&d->z, &s->z);
}

void ecp_free(ec_point *ecp)
{
	if (ecp)
		mp_free_multi(&ecp->z, &ecp->y, &ecp->x, 0ul);
}

static int check_a_is_pminus3(const mp_int *a, const mp_int *p)
{
	mp_int tmp;
	int rv;

	mp_init(&tmp);
	MP_CHK(rv, mp_sub_int(&tmp, p, 3), out);
	rv = mp_cmp(a, &tmp) ? 1 : 0;

out:
	mp_free(&tmp);
	return rv;
}

void ec_init(ecc *ec)
{
	if (ec) {
		tzi_memset(ec, 0, sizeof(*ec));
		mp_init_multi(&ec->field, &ec->a, &ec->b, &ec->order, &ec->two_inv_p, 0ul);
		ecp_init(&ec->group);
	}
}

void ec_swap(ecc *a, ecc *b)
{
	u_char buf[EC_MAX_SEED_LEN];
	u_long len;

	mp_swap(&a->field, &b->field);
	mp_swap(&a->a, &b->a);
	mp_swap(&a->b, &b->b);
	mp_swap(&a->order, &b->order);
	mp_swap(&a->two_inv_p, &b->two_inv_p);
	ecp_swap(&a->group, &b->group);
	tzi_memcpy(buf, a->seed, sizeof(buf));
	tzi_memcpy(a->seed, b->seed, sizeof(buf));
	tzi_memcpy(b->seed, buf, sizeof(buf));

	len = a->seed_len;
	a->seed_len = b->seed_len;
	b->seed_len = len;

	len = a->cofactor;
	a->cofactor = b->cofactor;
	b->cofactor = len;

	tzi_memcln(buf, sizeof(buf));
	tzi_memcln(&len, sizeof(len));
}

void ec_free(ecc *ec)
{
	if (ec) {
		ecp_free(&ec->group);
		mp_free_multi(&ec->field, &ec->a, &ec->b, &ec->order, &ec->two_inv_p, 0ul);
		tzi_memcln(ec, sizeof(*ec));
	}
}

int ec_set_curve(ecc *ec, const mp_int *p, const mp_int *a, const mp_int *b)
{
	mp_int tp, ta, tb;
	int rv = 0;

	mp_init_multi(&tp, &ta, &tb, 0ul);
	MP_CHK(rv, check_a_is_pminus3(a, p), out);
	MP_CHK(rv, mp_copy(&tp, p), out);
	MP_CHK(rv, mp_mod(&ta, a, p), out);
	MP_CHK(rv, mp_mod(&tb, b, p), out);
	MP_CHK(rv, mp_inv_int_mod(&ec->two_inv_p, 2, p), out);

	mp_swap(&ec->field, &tp);
	mp_swap(&ec->a, &ta);
	mp_swap(&ec->b, &tb);

out:
	mp_free_multi(&tp, &ta, &tb, 0ul);
	return rv;
}

int ec_set_generator(ecc *ec, const ec_point *generator, const mp_int *order, u_long cofactor)
{
	mp_int torder;
	ec_point tgenerator;
	int rv;

	mp_init(&torder);
	ecp_init(&tgenerator);

	MP_CHK(rv, mp_copy(&torder, order), out);
	MP_CHK(rv, ecp_copy(&tgenerator, generator), out);

	mp_swap(&ec->order, &torder);
	ecp_swap(&ec->group, &tgenerator);
	ec->cofactor = cofactor;

out:
	ecp_free(&tgenerator);
	mp_free(&torder);
	return rv;
}

int ec_set_seed(ecc *ec, const void *seed, u_long seed_len)
{
	if (seed_len > sizeof(ec->seed))
		return 1;
	tzi_memcpy(ec->seed, seed, seed_len);
	ec->seed_len = seed_len;
	return 0;
}

int ec_copy(ecc *d, const ecc *s)
{
	ecc t;
	int rv;

	ec_init(&t);
	MP_CHK(rv, ec_set_curve(&t, &s->field, &s->a, &s->b), out);
	MP_CHK(rv, ec_set_generator(&t, &s->group, &s->order, s->cofactor), out);
	MP_CHK(rv, ec_set_seed(&t, s->seed, s->seed_len), out);
	ec_swap(&t, d);
out:
	ec_free(&t);
	return rv;
}

u_long ec_get_degree(const ecc *ec)
{
	return mp_bitlen(&ec->field);
}

int ec_is_equal(const ecc *lh, const ecc *rh)
{
	if (!lh || !rh ||
	    mp_cmp(&lh->field, &rh->field) ||
	    mp_cmp(&lh->a, &rh->a) ||
	    mp_cmp(&lh->b, &rh->b))
		return 0;
	return 1;
}

/*  RESULT = 2 * POINT  */
static int ecp_double(ec_point *ecr, const ec_point *ecp, const ecc *ec)
{
	mp_int t1, t2, t3, l1, l2, l3, tx, ty, tz;
	int rv = 0;

	mp_init_multi(&t1, &t2, &t3, &l1, &l2, &l3, &tx, &ty, &tz, 0ul);

	if (!mp_cmp_int(&ecp->y, 0) || !mp_cmp_int(&ecp->z, 0)) {
		/* P_y == 0 || P_z == 0 => [1:1:0] */
		mp_lset(&ecr->x, 1);
		mp_lset(&ecr->y, 1);
		mp_lset(&ecr->z, 0);
	}
	else {
		MP_CHK(rv, mp_copy(&tx, &ecr->x), out);
		MP_CHK(rv, mp_copy(&ty, &ecr->y), out);
		MP_CHK(rv, mp_copy(&tz, &ecr->z), out);
		/* L1 = 3X^2 + aZ^4 */
		/* T1: used for aZ^4. */
		MP_CHK(rv, ec_pow_int_mod(&l1, &ecp->x, 2, ec), out);
		MP_CHK(rv, ec_mul_int_mod(&l1, &l1, 3, ec), out);
		MP_CHK(rv, ec_pow_int_mod(&t1, &ecp->z, 4, ec), out);
		MP_CHK(rv, ec_mulm(&t1, &t1, &ec->a, ec), out);
		MP_CHK(rv, ec_addm(&l1, &l1, &t1, ec), out);
		/* Z3 = 2YZ */
		MP_CHK(rv, ec_mulm(&tz, &ecp->y, &ecp->z, ec), out);
		MP_CHK(rv, ec_mul_int_mod(&tz, &tz, 2, ec), out);
		/* L2 = 4XY^2 */
		/* T2: used for Y2; required later. */
		MP_CHK(rv, ec_pow_int_mod(&t2, &ecp->y, 2, ec), out);
		MP_CHK(rv, ec_mulm(&l2, &t2, &ecp->x, ec), out);
		MP_CHK(rv, ec_mul_int_mod(&l2, &l2, 4, ec), out);
		/* X3 = L1^2 - 2L2 */
		/* T1: used for L2^2. */
		MP_CHK(rv, ec_pow_int_mod(&tx, &l1, 2, ec), out);
		MP_CHK(rv, ec_mul_int_mod(&t1, &l2, 2, ec), out);
		MP_CHK(rv, ec_subm(&tx, &tx, &t1, ec), out);
		/* L3 = 8Y^4 */
		/* T2: taken from above. */
		MP_CHK(rv, ec_pow_int_mod(&t2, &t2, 2, ec), out);
		MP_CHK(rv, ec_mul_int_mod(&l3, &t2, 8, ec), out);
		/* Y3 = L1(L2 - X3) - L3 */
		MP_CHK(rv, ec_subm(&ty, &l2, &tx, ec), out);
		MP_CHK(rv, ec_mulm(&ty, &ty, &l1, ec), out);
		MP_CHK(rv, ec_subm(&ty, &ty, &l3, ec), out);
		mp_swap(&ecr->x, &tx);
		mp_swap(&ecr->y, &ty);
		mp_swap(&ecr->z, &tz);
	}
out:
	mp_free_multi(&t1, &t2, &t3, &l1, &l2, &l3, &tx, &ty, &tz, 0ul);
	return rv;
}


/* RESULT = P1 + P2 */
int ecp_add(const ecc *ec, ec_point *result, const ec_point *p1, const ec_point *p2)
{
	mp_int l1, l2, l3, l4, l5, l6, l7, l8, l9, t1, t2;
	int rv = 0;

#define	x1	(p1->x    )
#define	y1	(p1->y    )
#define	z1	(p1->z    )
#define	x2	(p2->x    )
#define	y2	(p2->y    )
#define	z2	(p2->z    )
#define	x3	(result->x)
#define	y3	(result->y)
#define	z3	(result->z)
	mp_init_multi(&l1, &l2, &l3, &l4, &l5, &l6, &l7, &l8, &l9, &t1, &t2, 0ul);

	if ((!mp_cmp(&x1, &x2)) && (!mp_cmp(&y1, &y2)) && (!mp_cmp(&z1, &z2))) {
		/* Same point; need to call the duplicate function.  */
		MP_CHK(rv, ecp_double(result, p1, ec), out);
	}
	else if (!mp_cmp_int(&z1, 0)) {
		/* P1 is at infinity.  */
		MP_CHK(rv, mp_copy(&x3, &x2), out);
		MP_CHK(rv, mp_copy(&y3, &y2), out);
		MP_CHK(rv, mp_copy(&z3, &z2), out);
	}
	else if (!mp_cmp_int(&z2, 0)) {
		/* P2 is at infinity.  */
		MP_CHK(rv, mp_copy(&x3, &x1), out);
		MP_CHK(rv, mp_copy(&y3, &y1), out);
		MP_CHK(rv, mp_copy(&z3, &z1), out);
	}
	else {
		int z1_is_one = !mp_cmp_int(&z1, 1);
		int z2_is_one = !mp_cmp_int(&z2, 1);

		/* l1 = x1 z2^2  */
		/* l2 = x2 z1^2  */
		if (z2_is_one) {
			MP_CHK(rv, mp_copy(&l1, &x1), out);
		}
		else {
			MP_CHK(rv, ec_pow_int_mod(&l1, &z2, 2, ec), out);
			MP_CHK(rv, ec_mulm(&l1, &l1, &x1, ec), out);
		}
		if (z1_is_one) {
			MP_CHK(rv, mp_copy(&l2, &x2), out);
		}
		else {
			MP_CHK(rv, ec_pow_int_mod(&l2, &z1, 2, ec), out);
			MP_CHK(rv, ec_mulm(&l2, &l2, &x2, ec), out);
		}
		/* l3 = l1 - l2 */
		MP_CHK(rv, ec_subm(&l3, &l1, &l2, ec), out);
		/* l4 = y1 z2^3  */
		MP_CHK(rv, ec_pow_int_mod(&l4, &z2, 3, ec), out);
		MP_CHK(rv, ec_mulm(&l4, &l4, &y1, ec), out);
		/* l5 = y2 z1^3  */
		MP_CHK(rv, ec_pow_int_mod(&l5, &z1, 3, ec), out);
		MP_CHK(rv, ec_mulm(&l5, &l5, &y2, ec), out);
		/* l6 = l4 - l5  */
		MP_CHK(rv, ec_subm(&l6, &l4, &l5, ec), out);

		if (!mp_cmp_int(&l3, 0)) {
			if (!mp_cmp_int(&l6, 0)) {
				/* P1 and P2 are the same - use duplicate function.  */
				MP_CHK(rv, ecp_double(result, p1, ec), out);
			}
			else {
				/* P1 is the inverse of P2.  */
				MP_CHK(rv, mp_lset(&x3, 1), out);
				MP_CHK(rv, mp_lset(&y3, 1), out);
				MP_CHK(rv, mp_lset(&z3, 0), out);
			}
		}
		else {
			/* l7 = l1 + l2  */
			MP_CHK(rv, ec_addm(&l7, &l1, &l2, ec), out);
			/* l8 = l4 + l5  */
			MP_CHK(rv, ec_addm(&l8, &l4, &l5, ec), out);
			/* z3 = z1 z2 l3  */
			MP_CHK(rv, ec_mulm(&z3, &z1, &z2, ec), out);
			MP_CHK(rv, ec_mulm(&z3, &z3, &l3, ec), out);
			/* x3 = l6^2 - l7 l3^2  */
			MP_CHK(rv, ec_pow_int_mod(&t1, &l6, 2, ec), out);
			MP_CHK(rv, ec_pow_int_mod(&t2, &l3, 2, ec), out);
			MP_CHK(rv, ec_mulm(&t2, &t2, &l7, ec), out);
			MP_CHK(rv, ec_subm(&x3, &t1, &t2, ec), out);
			/* l9 = l7 l3^2 - 2 x3  */
			MP_CHK(rv, ec_mul_int_mod(&t1, &x3, 2, ec), out);
			MP_CHK(rv, ec_subm(&l9, &t2, &t1, ec), out);
			/* y3 = (l9 l6 - l8 l3^3)/2  */
			MP_CHK(rv, ec_mulm(&l9, &l9, &l6, ec), out);
			/* fixme: Use saved value*/
			MP_CHK(rv, ec_pow_int_mod(&t1, &l3, 3, ec), out);
			MP_CHK(rv, ec_mulm(&t1, &t1, &l8, ec), out);
			MP_CHK(rv, ec_subm(&y3, &l9, &t1, ec), out);
			MP_CHK(rv, ec_mulm(&y3, &y3, &ec->two_inv_p, ec), out);
		}
	}

#undef x1
#undef y1
#undef z1
#undef x2
#undef y2
#undef z2
#undef x3
#undef y3
#undef z3
out:
	mp_free_multi(&l1, &l2, &l3, &l4, &l5, &l6, &l7, &l8, &l9, &t1, &t2, 0ul);
	return rv;
}

/* Scalar point multiplication - the main function for ECC.  If takes
 * an integer SCALAR and a POINT as well as the usual context CTX.
 * RESULT will be set to the resulting point. */
int ecp_mul(const ecc *ec, ec_point *result, const mp_int *scalar, const ec_point *point)
{
	mp_int k, h, yy, z2, z3;
	ec_point p1, p2, p1inv;
	int i, loops, rv = 0;

	ecp_init(&p1); ecp_init(&p2); ecp_init(&p1inv);
	mp_init_multi(&k, &h, &yy, &z2, &z3, 0ul);

	MP_CHK(rv, mp_copy(&p1.x, &ec->field), out);
	MP_CHK(rv, mp_copy(&p1.y, &ec->field), out);
	MP_CHK(rv, mp_lset(&p1.z, 1), out);
	MP_CHK(rv, mp_copy(&h, &ec->field), out);
	MP_CHK(rv, mp_copy(&k, scalar), out);
	MP_CHK(rv, mp_copy(&yy, &point->y), out);

	if (k.s == -1) {
		k.s = -k.s;
		MP_CHK(rv, ec_invm(&yy, &yy, ec), out);
	}

	if (!mp_cmp_int(&point->z, 1)) {
		MP_CHK(rv, mp_copy(&p1.x, &point->x), out);
		MP_CHK(rv, mp_copy(&p1.y, &yy), out);
	}
	else {
		MP_CHK(rv, mp_copy(&z2, &ec->field), out);
		MP_CHK(rv, mp_copy(&z3, &ec->field), out);
		MP_CHK(rv, ec_mulm(&z2, &point->z, &point->z, ec), out);
		MP_CHK(rv, ec_mulm(&z3, &point->z, &z2, ec), out);
		MP_CHK(rv, ec_invm(&z2, &z2, ec), out);
		MP_CHK(rv, ec_mulm(&p1.x, &point->x, &z2, ec), out);
		MP_CHK(rv, ec_invm(&z3, &z3, ec), out);
		MP_CHK(rv, ec_mulm(&p1.y, &yy, &z3, ec), out);
	}

	/* h = 3k */
	MP_CHK(rv, mp_mul_int(&h, &k, 3), out);
	loops = mp_bitlen(&h);
	if (loops < 2) {
		/* If SCALAR is zero, the above mpi_mul sets H to zero and thus
		 * LOOPs will be zero.  To avoid an underflow of I in the main
		 * loop we set LOOP to 2 and the result to (0,0,0).  */
		loops = 2;
		MP_CHK(rv, mp_lset(&result->x, 0), out);
		MP_CHK(rv, mp_lset(&result->y, 0), out);
		MP_CHK(rv, mp_lset(&result->z, 0), out);
	}
	else {
		MP_CHK(rv, mp_copy(&result->x, &point->x), out);
		MP_CHK(rv, mp_copy(&result->y, &yy), out);
		MP_CHK(rv, mp_copy(&result->z, &point->z), out);
	}

	for(i = loops - 2; i > 0; i--) {
		MP_CHK(rv, ecp_double(result, result, ec), out);
		if (mp_get_bit(&h, i) == 1 && mp_get_bit(&k, i) == 0) {
			MP_CHK(rv, ecp_copy(&p2, result), out);
			MP_CHK(rv, ecp_add(ec, result, &p2, &p1), out);
		}
		if (mp_get_bit(&h, i) == 0 && mp_get_bit(&k, i) == 1) {
			MP_CHK(rv, ecp_copy(&p2, result), out);
			/* Invert point: y = p - y mod p  */
			MP_CHK(rv, ecp_copy(&p1inv, &p1), out);
			MP_CHK(rv, ec_subm(&p1inv.y, &ec->field, &p1inv.y, ec), out);
			MP_CHK(rv, ecp_add(ec, result, &p2, &p1inv), out);
		}
	}

out:
	mp_free_multi(&k, &h, &yy, &z2, &z3, 0ul);
	ecp_free(&p1); ecp_free(&p2); ecp_free(&p1inv);
	return rv;
}

/* Compute the affine coordinates from the projective coordinates in
 * POINT.  Set them into X and Y.  If one coordinate is not required,
 * X or Y may be passed as 0.  CTX is the usual context. Returns: 0
 * on success or ECANCELED if POINT is at infinity.  */
int ecp_get_affine(const ecc *ec, const ec_point *point, mp_int *x, mp_int *y)
{
	mp_int z1, z2, z3;
	int rv = 0;

	if (!point || !ec)
		return 1;

	if (!mp_cmp_int(&point->z, 0))
		return 1;

	mp_init_multi(&z1, &z2, &z3, 0ul);

	MP_CHK(rv, ec_invm(&z1, &point->z, ec), out);	/* z1 = z^(-1) mod p  */
	MP_CHK(rv, ec_mulm(&z2, &z1, &z1, ec), out);	/* z2 = z^(-2) mod p  */

	if (x) MP_CHK(rv, ec_mulm(x, &point->x, &z2, ec), out);
	if (y) {
		/* z3 = z^(-3) mod p  */
		MP_CHK(rv, ec_mulm(&z3, &z2, &z1, ec), out);
		MP_CHK(rv, ec_mulm(y, &point->y, &z3, ec), out);
	}

out:
	mp_free_multi(&z1, &z2, &z3, 0ul);
	return rv;
}

int ecp_set_affine(const ecc *ec, ec_point *point, const mp_int *x, const mp_int *y)
{
	mp_int tx, ty;
	int rv;

	mp_init_multi(&tx, &ty, 0ul);

	MP_CHK(rv, mp_mod(&tx, x, &ec->field), out);
	MP_CHK(rv, mp_mod(&ty, y, &ec->field), out);
	MP_CHK(rv, mp_lset(&point->z, 1), out);

	mp_swap(&point->x, &tx);
	mp_swap(&point->y, &ty);

out:
	mp_free_multi(&tx, &ty, 0ul);
	return rv;
}

int ecp_is_at_infinity(const ec_point *point)
{
	return (mp_cmp_int(&point->z, 0ul) == 0);
}
