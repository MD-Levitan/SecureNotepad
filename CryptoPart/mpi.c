#include <limits.h>
#include <stdarg.h>
#include "mpi.h"

#if (__SIZEOF_LONG__ == 8)
typedef __uint128_t mp_limb_dbl;
#elif (__SIZEOF_LONG__ == 4)
typedef __uint64_t mp_limb_dbl;
#endif

#define MULADDC_INIT	\
{			\
	mp_limb_dbl r;	\
	mp_limb r0, r1;

#define MULADDC_CORE			\
	r = *(s++) * (mp_limb_dbl) b;	\
	r0 = (mp_limb) r;		\
	r1 = (mp_limb)( r >> biL );	\
	r0 += c;  r1 += (r0 <  c);	\
	r0 += *d; r1 += (r0 < *d);	\
	c = r1; *(d++) = r0;

#define MULADDC_STOP	\
}

#define ciL (sizeof(mp_limb))
#define biL (ciL << 3)
#define biH (ciL << 2)

#define BITS_TO_LIMBS(i)  ((i) / biL + ((i) % biL != 0))
#define CHARS_TO_LIMBS(i) ((i) / ciL + ((i) % ciL != 0))

#define MPI_WINDOW_SIZE	6

#define MPI_MAX_SIZE	1024
#define MPI_MAX_BITS	(8 * MPI_MAX_SIZE)
#define MPI_MAX_LIMBS	10000

static __inline void mpn_zero(mp_limb *p, u_long n)
{
	while (n--)
		*p++ = 0;
}

static __inline mp_limb *mpn_alloc(u_long n)
{
	return tzi_malloc(n * sizeof(mp_limb));
}

static __inline mp_limb *mpn_calloc(u_long n)
{
	mp_limb *rv = mpn_alloc(n);
	if (rv)
		mpn_zero(rv, n);
	return rv;
}

static __inline void mpn_free(mp_limb *p)
{
	tzi_free(p);
}

static __inline void mpn_copy(mp_limb *d, const mp_limb *s, u_long n)
{
	while (n--)
		*d++ = *s++;
}

void mp_init(mp_int *mp)
{
	if (mp) {
		mp->s = 1;
		mp->n = 0;
		mp->p = 0;
	}
}

void mp_init_multi(mp_int *mp, ...)
{
	va_list va;
	for(va_start(va, mp); mp; mp = va_arg(va, mp_int *))
		mp_init(mp);
	va_end(va);
}

void mp_free(mp_int *mp)
{
	if (mp) {
		if (mp->p) {
			mpn_zero(mp->p, mp->n);
			mpn_free(mp->p);
		}
		mp->s = 1;
		mp->n = 0;
		mp->p = 0;
	}
}

void mp_free_multi(mp_int *mp, ...)
{
	va_list va;

	for(va_start(va, mp); mp; mp = va_arg(va, mp_int *))
		mp_free(mp);
	va_end(va);
}

int mp_grow(mp_int *mp, u_long nblimbs)
{
	mp_limb *p;

	if (nblimbs > MPI_MAX_LIMBS)
		return 1;
	if (mp->n < nblimbs) {
		if ((p = mpn_calloc(nblimbs)) == 0)
			return 1;
		if (mp->p) {
			mpn_copy(p, mp->p, mp->n);
			mpn_zero(mp->p, mp->n);
			mpn_free(mp->p);
		}
		mp->n = nblimbs;
		mp->p = p;
	}

	return 0;
}

int mp_shrink(mp_int *mp, u_long nblimbs)
{
	u_long i;
	mp_limb *p;

	if (mp->n <= nblimbs)
		return mp_grow(mp, nblimbs);
	for(i = mp->n - 1; i > 0; --i) {
		if (mp->p[i])
			break;
	}
	++i;

	if (i < nblimbs)
		i = nblimbs;
	if ((p = mpn_calloc(i)) == 0)
		return 1;
	if (mp->p) {
		mpn_copy(p, mp->p, i);
		mpn_zero(mp->p, mp->n);
		mpn_free(mp->p);
	}

	mp->n = i;
	mp->p = p;

	return 0;
}

int mp_copy(mp_int *d, const mp_int *s)
{
	u_long i;
	int rv;

	if (d == s)
		return 0;
	if (!s->p) {
		mp_free(d);
		return 0;
	}

	for(i = s->n - 1; i > 0; --i) {
		if (s->p[i])
			break;
	}
	++i;

	d->s = s->s;
	MP_CHK(rv, mp_grow(d, i), out);
	mpn_zero(d->p, d->n);
	mpn_copy(d->p, s->p, i);
out:
	return rv;
}

void mp_swap(mp_int *d, mp_int *s)
{
	mp_int t = *d;
	*d = *s;
	*s = t;
}

int mp_safe_cond_assign(mp_int *d, const mp_int *s, u_char assign)
{
	u_long i;
	int rv;

	/* make sure assign is 0 or 1 in a time-constant manner */
	assign = (assign | (u_char) - assign) >> 7;

	MP_CHK(rv, mp_grow(d, s->n), out);
	d->s = d->s * (1 - assign) + s->s * assign;

	for(i = 0; i < s->n; ++i)
		d->p[i] = d->p[i] * (1 - assign) + s->p[i] * assign;
	for(; i < d->n; ++i)
		d->p[i] *= (1 - assign);

out:
	return rv;
}

int mp_safe_cond_swap(mp_int *X, mp_int *Y, u_char swap)
{
	u_long i;
	mp_limb tmp;
	int rv, s;

	if (X == Y)
		return 0;

	swap = (swap | (u_char)-swap) >> 7;

	MP_CHK(rv, mp_grow(X, Y->n), out);
	MP_CHK(rv, mp_grow(Y, X->n), out);

	s = X->s;
	X->s = X->s * (1 - swap) + Y->s * swap;
	Y->s = Y->s * (1 - swap) +    s * swap;

	for(i = 0; i < X->n; i++) {
		tmp = X->p[i];
		X->p[i] = X->p[i] * (1 - swap) + Y->p[i] * swap;
		Y->p[i] = Y->p[i] * (1 - swap) +     tmp * swap;
	}
out:
	return rv;
}

int mp_lset(mp_int *mp, mp_slimb z)
{
	int rv;

	MP_CHK(rv, mp_grow(mp, 1), out);
	mpn_zero(mp->p, mp->n);

	mp->p[0] = (z < 0) ? -z : z;
	mp->s    = (z < 0) ? -1 : 1;

out:
	return rv;
}

int mp_get_bit(const mp_int *mp, u_long pos)
{
	if (mp->n * biL <= pos)
		return 0;
	return (mp->p[pos / biL] >> (pos % biL)) & 0x01;
}

int mp_set_bit(mp_int *mp, u_long pos, u_char val)
{
	u_long off = pos / biL;
	u_long idx = pos % biL;
	int rv = 0;

	if (val != 0 && val != 1)
		return 1;

	if ((mp->n * biL <= pos) && (!val || (rv = mp_grow(mp, off + 1))))
		goto out;

	mp->p[off] &= ~((mp_limb) 0x01 << idx);
	mp->p[off] |= (mp_limb) val << idx;

out:
	return rv;
}

u_long mp_lsb(const mp_int *mp)
{
	u_long i, j, count = 0;
	for(i = 0; i < mp->n; ++i) {
		for(j = 0; j < biL; ++j, ++count) {
			if ((mp->p[i] >> j) & 1)
				return count;
		}
	}
	return 0;
}

u_long mp_bitlen(const mp_int *mp)
{
	u_long i, j;

	if (!mp->n)
		return 0;

	for(i = mp->n - 1; i > 0; --i) {
		if (mp->p[i])
			break;
	}
	for(j = biL; j > 0; --j) {
		if ((mp->p[i] >> (j - 1)) & 1)
			break;
	}

	return (i * biL) + j;
}

u_long mp_size(const mp_int *mp)
{
	return (mp_bitlen(mp) + 7) / 8;
}

static int mp_get_digit(mp_limb *d, int radix, char c)
{
	*d = 255;

	if (c >= 0x30 && c <= 0x39) *d = c - 0x30;
	if (c >= 0x41 && c <= 0x46) *d = c - 0x37;
	if (c >= 0x61 && c <= 0x66) *d = c - 0x57;

	if (*d >= (mp_limb) radix)
		return 1;

	return 0;
}

int mp_read_string(mp_int *X, int radix, const char *s)
{
	u_long i, j, slen, n;
	mp_limb d;
	mp_int T;
	int rv;

	if (radix < 2 || radix > 16)
		return 1;

	mp_init(&T);

	slen = tzi_strlen(s);

	if (radix == 16) {
		if (slen > ULONG_MAX >> 2)
			return 1;

		n = BITS_TO_LIMBS(slen << 2);

		MP_CHK(rv, mp_grow(X, n), out);
		MP_CHK(rv, mp_lset(X, 0), out);

		for(i = slen, j = 0; i > 0; i--, j++) {
			if (i == 1 && s[i - 1] == '-') {
				X->s = -1;
				break;
			}

			MP_CHK(rv, mp_get_digit(&d, radix, s[i - 1]), out);
			X->p[j / ( 2 * ciL )] |= d << ( ( j % ( 2 * ciL ) ) << 2 );
		}
	}
	else {
		MP_CHK(rv, mp_lset(X, 0), out);

		for(i = 0; i < slen; i++) {
			if (i == 0 && s[i] == '-') {
				X->s = -1;
				continue;
			}

			MP_CHK(rv, mp_get_digit(&d, radix, s[i]), out);
			MP_CHK(rv, mp_mul_int(&T, X, radix), out);

			if (X->s == 1)
				MP_CHK(rv, mp_add_int(X, &T, d), out);
			else
				MP_CHK(rv, mp_sub_int(X, &T, d), out);
		}
	}

out:
	mp_free(&T);

	return rv;
}

static int mp_write_hlp(mp_int *mp, int radix, char **p)
{
	mp_limb r;
	int rv;

	if (radix < 2 || radix > 16)
		return 1;
	MP_CHK(rv, mp_mod_int(&r, mp, radix), out);
	MP_CHK(rv, mp_div_int(mp, 0, mp, radix), out);

	if (mp_cmp_int(mp, 0))
		MP_CHK(rv, mp_write_hlp(mp, radix, p), out);
	*(*p)++ = (char) (r + (r < 10 ? 0x30 : 0x37));
out:
	return rv;
}

int mp_write_string(const mp_int *X, int radix, char *buf, u_long buflen, u_long *olen)
{
	u_long n;
	mp_int T;
	char *p;
	int rv = 0;

	if (radix < 2 || radix > 16)
		return 1;

	n = mp_bitlen(X);
	if (radix >=  4) n >>= 1;
	if (radix >= 16) n >>= 1;
	n += 3;

	if (buflen < n) {
		*olen = n;
		return 1;
	}

	p = buf;
	mp_init(&T);

	if (X->s == -1)
		*p++ = '-';

	if (radix == 16) {
		u_long i, j, k;
		int c;

		for(i = X->n, k = 0; i > 0; i--) {
			for(j = ciL; j > 0; j--) {
				c = (X->p[i - 1] >> ((j - 1) << 3)) & 0xff;

				if (c == 0 && k == 0 && (i + j) != 2)
					continue;

				*(p++) = "0123456789ABCDEF" [c / 16];
				*(p++) = "0123456789ABCDEF" [c % 16];
				k = 1;
			}
		}
	}
	else {
		MP_CHK(rv, mp_copy(&T, X), out);

		if (T.s == -1)
			T.s = 1;

		MP_CHK(rv, mp_write_hlp(&T, radix, &p), out);
	}

	*p++ = '\0';
	*olen = p - buf;

out:
	mp_free(&T);
	return rv;
}

static int mp_read_binary_be(mp_int *mp, const u_char *buf, u_long buflen)
{
	u_long i, j, n;
	int rv;

	for(n = 0; n < buflen; ++n) {
		if (buf[n])
			break;
	}

	MP_CHK(rv, mp_grow(mp, CHARS_TO_LIMBS(buflen - n)), out);
	MP_CHK(rv, mp_lset(mp, 0), out);

	for(i = buflen, j = 0; i > n; --i, ++j)
		mp->p[j / ciL] |= ((mp_limb) buf[i - 1]) << ((j % ciL) << 3);
out:
	return rv;
}

static int mp_write_binary_be(const mp_int *mp, u_char *buf, u_long buflen)
{
	u_long i, j, n;

	n = mp_size(mp);
	if (buflen < n)
		return 1;

	tzi_memset(buf, 0, buflen);
	for(i = buflen - 1, j = 0; n > 0; --i, ++j, --n)
		buf[i] = (u_char) (mp->p[j / ciL] >> ((j % ciL) << 3));

	return 0;
}

static int mp_read_binary_le(mp_int *mp, const u_char *buf, u_long buflen)
{
	u_long i;
	int rv;

	for(; buflen; --buflen) {
		if (buf[buflen - 1])
			break;
	}

	MP_CHK(rv, mp_grow(mp, CHARS_TO_LIMBS(buflen)), out);
	MP_CHK(rv, mp_lset(mp, 0), out);

	for(i = 0; i < buflen; ++i)
		mp->p[i / ciL] |= ((mp_limb) buf[i]) << ((i % ciL) << 3);
out:
	return rv;
}

static int mp_write_binary_le(const mp_int *mp, u_char *buf, u_long buflen)
{
	u_long i, n;

	n = mp_size(mp);
	if (buflen < n)
		return 1;

	tzi_memset(buf, 0, buflen);
	for(i = 0; i < n; ++i)
		buf[i] = (u_char) (mp->p[i / ciL] >> ((i % ciL) << 3));

	return 0;
}

int mp_read_binary(mp_int *mp, const void *buf, u_long buflen, int order)
{
	int rv = 0;
	if (order)
		rv = mp_read_binary_le(mp, buf, buflen);
	else
		rv = mp_read_binary_be(mp, buf, buflen);
	return rv;
}

int mp_write_binary(const mp_int *mp, void *buf, u_long buflen, int order)
{
	int rv = 0;
	if (order)
		rv = mp_write_binary_le(mp, buf, buflen);
	else
		rv = mp_write_binary_be(mp, buf, buflen);
	return rv;
}

int mp_shift_l(mp_int *mp, u_long count)
{
	u_long i, v0, t1;
	mp_limb r0 = 0, r1;
	int rv = 0;

	v0 = count / (biL    );
	t1 = count & (biL - 1);

	i = mp_bitlen(mp) + count;

	if (mp->n * biL < i)
		MP_CHK(rv, mp_grow(mp, BITS_TO_LIMBS(i)), out);

	if (v0 > 0) {
		for(i = mp->n; i > v0; --i)
			mp->p[i - 1] = mp->p[i - v0 - 1];
		for(; i > 0; --i)
			mp->p[i - 1] = 0;
	}

	if (t1 > 0) {
		for(i = v0; i < mp->n; ++i) {
			r1 = mp->p[i] >> (biL - t1);
			mp->p[i] <<= t1;
			mp->p[i] |= r0;
			r0 = r1;
		}
	}
out:
	return rv;
}

int mp_shift_r(mp_int *mp, u_long count)
{
	u_long i, v0, v1;
	mp_limb r0 = 0, r1;

	v0 = count / (biL    );
	v1 = count & (biL - 1);

	if (v0 > mp->n || (v0 == mp->n && v1 > 0))
		return mp_lset(mp, 0);

	if (v0 > 0) {
		for(i = 0; i < mp->n - v0; ++i)
			mp->p[i] = mp->p[i + v0];
		for(; i < mp->n; ++i)
			mp->p[i] = 0;
	}

	if (v1 > 0) {
		for(i = mp->n; i > 0; --i) {
			r1 = mp->p[i - 1] << (biL - v1);
			mp->p[i - 1] >>= v1;
			mp->p[i - 1] |= r0;
			r0 = r1;
		}
	}

	return 0;
}

int mp_cmp_abs(const mp_int *X, const mp_int *Y)
{
	u_long i, j;

	for(i = X->n; i > 0; i--) {
		if (X->p[i - 1])
			break;
	}
	for(j = Y->n; j > 0; j--) {
		if (Y->p[j - 1])
			break;
	}

	if (i == 0 && j == 0)
		return 0;

	if (i > j)
		return 1;
	if (j > i)
		return -1;

	for(; i > 0; i--) {
		if (X->p[i - 1] > Y->p[i - 1])
			return 1;
		if (X->p[i - 1] < Y->p[i - 1])
			return -1;
	}

	return 0;
}

int mp_cmp(const mp_int *x, const mp_int *y)
{
	unsigned i, j;

	for(i = x->n; i > 0; --i) {
		if (x->p[i - 1])
			break;
	}
	for(j = y->n; j > 0; --j) {
		if (y->p[j - 1])
			break;
	}

	if (!i && !j)
		return 0;

	if (i > j) return x->s;
	if (j > i) return -y->s;

	if (x->s > 0 && y->s < 0)
		return 1;
	if (y->s > 0 && x->s < 0)
		return -1;

	for(; i > 0; --i) {
		if (x->p[i - 1] > y->p[i - 1])
			return x->s;
		if (x->p[i - 1] < y->p[i - 1])
			return -x->s;
	}

	return 0;
}

int mp_cmp_int(const mp_int *mp, mp_slimb z)
{
	mp_limb p[1];
	mp_int t;

	*p  = (z < 0) ? -z : z;
	t.s = (z < 0) ? -1 : 1;
	t.n = 1;
	t.p = p;

	return mp_cmp(mp, &t);
}

int mp_add_abs(mp_int *x, const mp_int *a, const mp_int *b)
{
	u_long i, j;
	mp_limb *o, *p, c;
	int rv;

	if (x == b) {
		const mp_int *t = a;
		a = x;
		b = t;
	}
	if (x != a)
		MP_CHK(rv, mp_copy(x, a), out);
	x->s = 1;

	for(j = b->n; j > 0; --j) {
		if (b->p[j - 1])
			break;
	}

	MP_CHK(rv, mp_grow(x, j), out);

	o = b->p; p = x->p; c = 0;
	for(i = 0; i < j; ++i, ++o, ++p) {
		*p +=  c; c  = (*p <  c);
		*p += *o; c += (*p < *o);
	}

	while (c) {
		if (i >= x->n) {
			MP_CHK(rv, mp_grow(x, i + 1), out);
			p = x->p + i;
		}
		*p += c; c = (*p < c); ++i; ++p;
	}
out:
	return rv;
}

static void mp_sub_hlp(u_long n, mp_limb *s, mp_limb *d)
{
	u_long i;
	mp_limb c, z;

	for(i = c = 0; i < n; ++i, ++s, ++d) {
		z = (*d <  c);     *d -=  c;
		c = (*d < *s) + z; *d -= *s;
	}

	while (c) {
		z = (*d < c); *d -= c;
		c = z; ++i; ++d;
	}
}

int mp_sub_abs(mp_int *x, const mp_int *a, const mp_int *b)
{
	u_long n;
	mp_int tb;
	int rv = 0;

	if (mp_cmp_abs(a, b) < 0)
		return 1;

	mp_init(&tb);
	if (x == b) {
		MP_CHK(rv, mp_copy(&tb, b), out);
		b = &tb;
	}
	if (x != a)
		MP_CHK(rv, mp_copy(x, a), out);
	x->s = 1;

	for(n = b->n; n > 0; --n) {
		if (b->p[n - 1])
			break;
	}
	mp_sub_hlp(n, b->p, x->p);
out:
	mp_free(&tb);
	return rv;
}

int mp_add(mp_int *x, const mp_int *a, const mp_int *b)
{
	int rv = 0, s = a->s;

	if (a->s * b->s < 0) {
		if (mp_cmp_abs(a, b) >= 0) {
			MP_CHK(rv, mp_sub_abs(x, a, b), out);
			x->s = s;
		}
		else {
			MP_CHK(rv, mp_sub_abs(x, b, a), out);
			x->s = -s;
		}
	}
	else {
		MP_CHK(rv, mp_add_abs(x, a, b), out);
		x->s = s;
	}
out:
	return rv;
}

int mp_sub(mp_int *X, const mp_int *A, const mp_int *B)
{
	int rv, s = A->s;

	if (A->s * B->s > 0) {
		if (mp_cmp_abs(A, B) >= 0) {
			MP_CHK(rv, mp_sub_abs(X, A, B), out);
			X->s =  s;
		}
		else {
			MP_CHK(rv, mp_sub_abs(X, B, A), out);
			X->s = -s;
		}
	}
	else {
		MP_CHK(rv, mp_add_abs(X, A, B), out);
		X->s = s;
	}

out:
	return rv;
}

int mp_add_int(mp_int *x, const mp_int *a, mp_slimb b)
{
	mp_limb p[1];
	mp_int t;

	p[0] = (b < 0) ? -b : b;
	t.s  = (b < 0) ? -1 : 1;
	t.n  = 1;
	t.p  = p;

	return mp_add(x, a, &t);
}

int mp_sub_int(mp_int *x, const mp_int *a, mp_slimb b)
{
	mp_limb p[1];
	mp_int t;

	p[0] = (b < 0) ? -b : b;
	t.s  = (b < 0) ? -1 : 1;
	t.n  = 1;
	t.p  = p;

	return mp_sub(x, a, &t);
}

static void mp_mul_hlp(u_long i, mp_limb *s, mp_limb *d, mp_limb b)
{
	mp_limb c = 0, t = 0;

	for(; i >= 16; i -= 16) {
		MULADDC_INIT
		MULADDC_CORE MULADDC_CORE
		MULADDC_CORE MULADDC_CORE
		MULADDC_CORE MULADDC_CORE
		MULADDC_CORE MULADDC_CORE

		MULADDC_CORE MULADDC_CORE
		MULADDC_CORE MULADDC_CORE
		MULADDC_CORE MULADDC_CORE
		MULADDC_CORE MULADDC_CORE
		MULADDC_STOP
	}
	for(; i >= 8; i -= 8) {
		MULADDC_INIT
		MULADDC_CORE MULADDC_CORE
		MULADDC_CORE MULADDC_CORE

		MULADDC_CORE MULADDC_CORE
		MULADDC_CORE MULADDC_CORE
		MULADDC_STOP
	}
	for(; i > 0; --i) {
		MULADDC_INIT
		MULADDC_CORE
		MULADDC_STOP
	}

	t++;

	do {
		*d += c; c = (*d < c) ; ++d;
	} while (c);
}

int mp_mul(mp_int *x, const mp_int *a, const mp_int *b)
{
	u_long i, j;
	mp_int ta, tb;
	int rv;

	mp_init_multi(&ta, &tb, 0ul);

	if (x == a) {
		MP_CHK(rv, mp_copy(&ta, a), out);
		a = &ta;
	}
	if (x == b) {
		MP_CHK(rv, mp_copy(&tb, b), out);
		b = &tb;
	}

	for(i = a->n; i > 0; --i) {
		if (a->p[i - 1])
			break;
	}
	for(j = b->n; j > 0; --j) {
		if (b->p[j - 1])
			break;
	}

	MP_CHK(rv, mp_grow(x, i + j), out);
	MP_CHK(rv, mp_lset(x, 0), out);

	for(++i; j > 0; --j)
		mp_mul_hlp(i - 1, a->p, x->p + j - 1, b->p[j - 1]);
	x->s = a->s * b->s;
out:
	mp_free_multi(&tb, &ta, 0ul);
	return rv;
}

int mp_mul_int(mp_int *x, const mp_int *a, mp_limb b)
{
	mp_limb p[1];
	mp_int t;

	p[0] = b;
	t.s  = 1;
	t.n  = 1;
	t.p  = p;

	return mp_mul(x, a, &t);
}

int mp_div(mp_int *Q, mp_int *R, const mp_int *A, const mp_int *B )
{
	u_long i, n, t, k;
	mp_int X, Y, Z, T1, T2;
	int rv;

	if (mp_cmp_int(B, 0) == 0)
		return 1;

	mp_init_multi(&X, &Y, &Z, &T1, &T2, 0ul);

	if (mp_cmp_abs(A, B) < 0) {
		if (Q) MP_CHK(rv, mp_lset(Q, 0), out);
		if (R) MP_CHK(rv, mp_copy(R, A), out);
		return 0;
	}

	MP_CHK(rv, mp_copy(&X, A), out);
	MP_CHK(rv, mp_copy(&Y, B), out);
	X.s = Y.s = 1;

	MP_CHK(rv, mp_grow(&Z, A->n + 2), out);
	MP_CHK(rv, mp_lset(&Z, 0), out);
	MP_CHK(rv, mp_grow(&T1, 2), out);
	MP_CHK(rv, mp_grow(&T2, 3), out);

	k = mp_bitlen( &Y ) % biL;
	if (k < biL - 1) {
		k = biL - 1 - k;
		MP_CHK(rv, mp_shift_l(&X, k), out);
		MP_CHK(rv, mp_shift_l(&Y, k), out);
	}
	else {
		k = 0;
	}

	n = X.n - 1;
	t = Y.n - 1;
	MP_CHK(rv, mp_shift_l(&Y, biL * ( n - t )), out);

	while (mp_cmp(&X, &Y) >= 0) {
		Z.p[n - t]++;
		MP_CHK(rv, mp_sub(&X, &X, &Y), out);
	}
	MP_CHK(rv, mp_shift_r(&Y, biL * (n - t)), out);

	for(i = n; i > t ; i--) {
		if (X.p[i] >= Y.p[t]) {
			Z.p[i - t - 1] = ~0ul;
		}
		else {
			mp_limb_dbl r;

			r  = (mp_limb_dbl) X.p[i] << biL;
			r |= (mp_limb_dbl) X.p[i - 1];
			r /= Y.p[t];
			if (r > ((mp_limb_dbl) 1 << biL) - 1)
				r = ((mp_limb_dbl) 1 << biL) - 1;

			Z.p[i - t - 1] = (mp_limb) r;
		}

		Z.p[i - t - 1]++;
		do {
			Z.p[i - t - 1]--;

			MP_CHK(rv, mp_lset(&T1, 0), out);
			T1.p[0] = (t < 1) ? 0 : Y.p[t - 1];
			T1.p[1] = Y.p[t];
			MP_CHK(rv, mp_mul_int(&T1, &T1, Z.p[i - t - 1]), out);

			MP_CHK(rv, mp_lset(&T2, 0), out);
			T2.p[0] = ( i < 2 ) ? 0 : X.p[i - 2];
			T2.p[1] = ( i < 1 ) ? 0 : X.p[i - 1];
			T2.p[2] = X.p[i];
		} while(mp_cmp(&T1, &T2) > 0);

		MP_CHK(rv, mp_mul_int(&T1, &Y, Z.p[i - t - 1]), out);
		MP_CHK(rv, mp_shift_l(&T1,  biL * (i - t - 1)), out);
		MP_CHK(rv, mp_sub(&X, &X, &T1), out);

		if (mp_cmp_int(&X, 0) < 0) {
			MP_CHK(rv, mp_copy(&T1, &Y), out);
			MP_CHK(rv, mp_shift_l(&T1, biL * (i - t - 1)), out);
			MP_CHK(rv, mp_add(&X, &X, &T1), out);
			Z.p[i - t - 1]--;
		}
	}

	if (Q) {
		MP_CHK(rv, mp_copy(Q, &Z), out);
		Q->s = A->s * B->s;
	}

	if (R) {
		MP_CHK(rv, mp_shift_r(&X, k), out);
		X.s = A->s;
		MP_CHK(rv, mp_copy(R, &X), out);

		if (!mp_cmp_int(R, 0))
			R->s = 1;
	}

out:
	mp_free_multi(&X, &Y, &Z, &T1, &T2, 0ul);

	return rv;
}

int mp_div_int(mp_int *q, mp_int *r, const mp_int *a, mp_slimb b)
{
	mp_limb p[1];
	mp_int t;

	p[0] = (b < 0) ? -b : b;
	t.s  = (b < 0) ? -1 : 1;
	t.n  = 1;
	t.p  = p;

	return mp_div(q, r, a, &t);
}

int mp_mod(mp_int *r, const mp_int *a, const mp_int *b)
{
	int rv;

	if (mp_cmp_int(b, 0) < 0)
		return 1;
	MP_CHK(rv, mp_div(0, r, a, b), out);
	while (mp_cmp_int(r, 0) < 0)
		MP_CHK(rv, mp_add(r, r, b), out);
	while (mp_cmp(r, b) >= 0)
		MP_CHK(rv, mp_sub(r, r, b), out);
out:
	return rv;
}

int mp_mod_int(mp_limb *r, const mp_int *a, mp_slimb b)
{
	u_long i;
	mp_limb x, y, z;

	if (!b)
		return 1;
	if (b < 0)
		return 1;

	if (b == 1) {
		*r = 0;
		return 0;
	}
	if (b == 2) {
		*r = a->p[0] & 1;
		return 0;
	}

	for(i = a->n, y = 0; i > 0; --i) {
		x  = a->p[i - 1];
		y  = (y << biH) | (x >> biH);
		z  = y / b;
		y -= z * b;

		x <<= biH;
		y  = (y << biH) | (x >> biH);
		z  = y / b;
		y -= z * b;
	}

	if (a->s < 0 && y)
		y = b - y;
	*r = y;

	return 0;
}

static void mp_mont_init(mp_limb *mm, const mp_int *N)
{
	mp_limb x, m0 = N->p[0];
	u_long i;

	x  = m0;
	x += ((m0 + 2) & 4) << 1;

	for(i = biL; i >= 8; i /= 2)
		x *= (2 - (m0 * x));

	*mm = ~x + 1;
}

static void mp_montmul(mp_int *A, const mp_int *B, const mp_int *N, mp_limb mm, const mp_int *T)
{
	u_long i, n, m;
	mp_limb u0, u1, *d;

	tzi_memset(T->p, 0, T->n * ciL);

	d = T->p;
	n = N->n;
	m = (B->n < n) ? B->n : n;

	for(i = 0; i < n; i++) {
		u0 = A->p[i];
		u1 = (d[0] + u0 * B->p[0]) * mm;

		mp_mul_hlp(m, B->p, d, u0);
		mp_mul_hlp(n, N->p, d, u1);

		*d++ = u0; d[n + 1] = 0;
	}

	tzi_memcpy(A->p, d, (n + 1) * ciL);

	if (mp_cmp_abs(A, N) >= 0)
		mp_sub_hlp(n, N->p, A->p);
	else
		mp_sub_hlp(n, A->p, T->p);
}

static void mp_montred(mp_int *A, const mp_int *N, mp_limb mm, const mp_int *T )
{
	mp_int U;
	mp_limb z = 1;

	U.n = U.s = (int) z;
	U.p = &z;

	mp_montmul(A, &U, N, mm, T);
}

int mp_exp_mod(mp_int *X, const mp_int *A, const mp_int *E, const mp_int *N, mp_int *_RR)
{
	u_long wbits, wsize, one = 1;
	u_long i, j, nblimbs;
	u_long bufsize, nbits;
	mp_limb ei, mm, state;
	mp_int RR, T, W[2 << MPI_WINDOW_SIZE], Apos;
	int rv, neg;

	if (mp_cmp_int(N, 0) < 0 || (N->p[0] & 1) == 0)
		return 1;

	if (mp_cmp_int(E, 0) < 0)
		return 1;

	mp_mont_init(&mm, N);
	mp_init_multi(&RR, &T, &Apos, 0ul);
	tzi_memset(W, 0, sizeof(W));

	i = mp_bitlen(E);

	wsize = ( i > 671 ) ? 6 : ( i > 239 ) ? 5 :
		( i >  79 ) ? 4 : ( i >  23 ) ? 3 : 1;

	if (wsize > MPI_WINDOW_SIZE)
		wsize = MPI_WINDOW_SIZE;

	j = N->n + 1;
	MP_CHK(rv, mp_grow(X, j), out);
	MP_CHK(rv, mp_grow(&W[1], j), out);
	MP_CHK(rv, mp_grow(&T, j * 2), out);

	neg = (A->s == -1);
	if (neg) {
		MP_CHK(rv, mp_copy(&Apos, A), out);
		Apos.s = 1;
		A = &Apos;
	}

	if (_RR == 0ul || _RR->p == 0ul) {
		MP_CHK(rv, mp_lset(&RR, 1), out);
		MP_CHK(rv, mp_shift_l(&RR, N->n * 2 * biL), out);
		MP_CHK(rv, mp_mod(&RR, &RR, N), out);

		if (_RR != 0ul)
			tzi_memcpy(_RR, &RR, sizeof(mp_int));
	}
	else {
		tzi_memcpy(&RR, _RR, sizeof(mp_int));
	}

	if (mp_cmp(A, N) >= 0)
		MP_CHK(rv, mp_mod(&W[1], A, N), out);
	else
		MP_CHK(rv, mp_copy(&W[1], A), out);

	mp_montmul(&W[1], &RR, N, mm, &T);

	MP_CHK(rv, mp_copy(X, &RR), out);
	mp_montred(X, N, mm, &T);

	if (wsize > 1) {
		j =  one << (wsize - 1);

		MP_CHK(rv, mp_grow(&W[j], N->n + 1), out);
		MP_CHK(rv, mp_copy(&W[j], &W[1]), out);

		for(i = 0; i < wsize - 1; i++)
			mp_montmul(&W[j], &W[j], N, mm, &T);

		for(i = j + 1; i < (one << wsize); i++) {
			MP_CHK(rv, mp_grow(&W[i], N->n + 1), out);
			MP_CHK(rv, mp_copy(&W[i], &W[i - 1]), out);

			mp_montmul(&W[i], &W[1], N, mm, &T);
		}
	}

	nblimbs = E->n;
	bufsize = 0;
	nbits   = 0;
	wbits   = 0;
	state   = 0;

	while(1) {
		if (bufsize == 0) {
			if (nblimbs == 0)
				break;

			nblimbs--;
			bufsize = sizeof(mp_limb) << 3;
		}

		bufsize--;

		ei = (E->p[nblimbs] >> bufsize) & 1;

		if (ei == 0 && state == 0)
			continue;

		if (ei == 0 && state == 1) {
			mp_montmul(X, X, N, mm, &T);
			continue;
		}

		state = 2;

		nbits++;
		wbits |= (ei << (wsize - nbits));

		if (nbits == wsize) {
			for(i = 0; i < wsize; i++)
				mp_montmul(X, X, N, mm, &T);

			mp_montmul(X, &W[wbits], N, mm, &T);

			state--;
			nbits = 0;
			wbits = 0;
		}
	}

	for(i = 0; i < nbits; i++) {
		mp_montmul(X, X, N, mm, &T);

		wbits <<= 1;

		if ((wbits & (one << wsize)) != 0)
			mp_montmul(X, &W[1], N, mm, &T);
	}

	mp_montred(X, N, mm, &T);

	if (neg) {
		X->s = -1;
		MP_CHK(rv, mp_add(X, N, X), out);
	}

out:
	for(i = (one << (wsize - 1)); i < (one << wsize); i++)
		mp_free(&W[i]);

	mp_free_multi(&W[1], &T, &Apos, 0ul);

	if (_RR == 0ul || _RR->p == 0ul)
		mp_free(&RR);

	return rv;
}

int mp_gcd(mp_int *G, const mp_int *A, const mp_int *B)
{
	u_long lz, lzt;
	mp_int TG, TA, TB;
	int rv;

	mp_init_multi(&TG, &TA, &TB, 0ul);

	MP_CHK(rv, mp_copy(&TA, A), out);
	MP_CHK(rv, mp_copy(&TB, B), out);

	lz  = mp_lsb(&TA);
	lzt = mp_lsb(&TB);

	if (lzt < lz)
		lz = lzt;

	MP_CHK(rv, mp_shift_r(&TA, lz), out);
	MP_CHK(rv, mp_shift_r(&TB, lz), out);

	TA.s = TB.s = 1;

	while(mp_cmp_int(&TA, 0) != 0) {
		MP_CHK(rv, mp_shift_r(&TA, mp_lsb(&TA)), out);
		MP_CHK(rv, mp_shift_r(&TB, mp_lsb(&TB)), out);

		if (mp_cmp(&TA, &TB) >= 0) {
			MP_CHK(rv, mp_sub_abs(&TA, &TA, &TB), out);
			MP_CHK(rv, mp_shift_r(&TA, 1), out);
		}
		else {
			MP_CHK(rv, mp_sub_abs(&TB, &TB, &TA), out);
			MP_CHK(rv, mp_shift_r(&TB, 1), out);
		}
	}

	MP_CHK(rv, mp_shift_l(&TB, lz), out);
	MP_CHK(rv, mp_copy(G, &TB), out);

out:
	mp_free_multi(&TG, &TA, &TB, 0ul);

	return rv;
}

int mp_random(mp_int *X, u_long size, int (*f_rng)(void *, u_char *, u_long), void *p_rng)
{
	u_char buf[MPI_MAX_SIZE];
	int rv;

	if (size > MPI_MAX_SIZE)
		return 1;

	MP_CHK(rv, f_rng(p_rng, buf, size), out);
	MP_CHK(rv, mp_read_binary(X, buf, size, 0), out);

out:
	return rv;
}

int mp_inv_mod(mp_int *X, const mp_int *A, const mp_int *N)
{
	mp_int G, TA, TU, U1, U2, TB, TV, V1, V2;
	int rv;

	if (mp_cmp_int(N, 0) <= 0)
		return 1;

	mp_init_multi(&TA, &TU, &U1, &U2, &G, &TB, &TV, &V1, &V2, 0ul);

	MP_CHK(rv, mp_gcd(&G, A, N), out);

	if (mp_cmp_int(&G, 1) != 0) {
		rv = 1;
		goto out;
	}

	MP_CHK(rv, mp_mod(&TA, A, N), out);
	MP_CHK(rv, mp_copy(&TU, &TA), out);
	MP_CHK(rv, mp_copy(&TB, N), out);
	MP_CHK(rv, mp_copy(&TV, N), out);

	MP_CHK(rv, mp_lset(&U1, 1), out);
	MP_CHK(rv, mp_lset(&U2, 0), out);
	MP_CHK(rv, mp_lset(&V1, 0), out);
	MP_CHK(rv, mp_lset(&V2, 1), out);

	do {
		while((TU.p[0] & 1) == 0) {
			MP_CHK(rv, mp_shift_r(&TU, 1), out);

			if ((U1.p[0] & 1) != 0 || (U2.p[0] & 1) != 0) {
				MP_CHK(rv, mp_add(&U1, &U1, &TB), out);
				MP_CHK(rv, mp_sub(&U2, &U2, &TA), out);
			}

			MP_CHK(rv, mp_shift_r(&U1, 1), out);
			MP_CHK(rv, mp_shift_r(&U2, 1), out);
		}

		while((TV.p[0] & 1) == 0) {
			MP_CHK(rv, mp_shift_r(&TV, 1), out);

			if ((V1.p[0] & 1) != 0 || (V2.p[0] & 1) != 0) {
				MP_CHK(rv, mp_add(&V1, &V1, &TB), out);
				MP_CHK(rv, mp_sub(&V2, &V2, &TA), out);
			}

			MP_CHK(rv, mp_shift_r(&V1, 1), out);
			MP_CHK(rv, mp_shift_r(&V2, 1), out);
		}

		if (mp_cmp(&TU, &TV) >= 0) {
			MP_CHK(rv, mp_sub(&TU, &TU, &TV), out);
			MP_CHK(rv, mp_sub(&U1, &U1, &V1), out);
			MP_CHK(rv, mp_sub(&U2, &U2, &V2), out);
		}
		else {
			MP_CHK(rv, mp_sub(&TV, &TV, &TU), out);
			MP_CHK(rv, mp_sub(&V1, &V1, &U1), out);
			MP_CHK(rv, mp_sub(&V2, &V2, &U2), out);
		}
	} while(mp_cmp_int(&TU, 0) != 0 );

	while(mp_cmp_int(&V1, 0) < 0)
		MP_CHK(rv, mp_add(&V1, &V1, N), out);

	while(mp_cmp(&V1, N) >= 0)
		MP_CHK(rv, mp_sub(&V1, &V1, N), out);

	MP_CHK(rv, mp_copy(X, &V1), out);

out:
	mp_free_multi(&TA, &TU, &U1, &U2, &G, &TB, &TV, &V1, &V2, 0ul);

	return rv;
}

int mp_inv_int_mod(mp_int *res, mp_limb d, const mp_int *mod)
{
	mp_limb p[1];
	mp_int t;

	*p = d;
	t.s = 1;
	t.n = 1;
	t.p = p;

	return mp_inv_mod(res, &t, mod);
}

static const int small_prime[] = {
      3,   5,   7,  11,  13,  17,  19,  23,
     29,  31,  37,  41,  43,  47,  53,  59,
     61,  67,  71,  73,  79,  83,  89,  97,
    101, 103, 107, 109, 113, 127, 131, 137,
    139, 149, 151, 157, 163, 167, 173, 179,
    181, 191, 193, 197, 199, 211, 223, 227,
    229, 233, 239, 241, 251, 257, 263, 269,
    271, 277, 281, 283, 293, 307, 311, 313,
    317, 331, 337, 347, 349, 353, 359, 367,
    373, 379, 383, 389, 397, 401, 409, 419,
    421, 431, 433, 439, 443, 449, 457, 461,
    463, 467, 479, 487, 491, 499, 503, 509,
    521, 523, 541, 547, 557, 563, 569, 571,
    577, 587, 593, 599, 601, 607, 613, 617,
    619, 631, 641, 643, 647, 653, 659, 661,
    673, 677, 683, 691, 701, 709, 719, 727,
    733, 739, 743, 751, 757, 761, 769, 773,
    787, 797, 809, 811, 821, 823, 827, 829,
    839, 853, 857, 859, 863, 877, 881, 883,
    887, 907, 911, 919, 929, 937, 941, 947,
    953, 967, 971, 977, 983, 991, 997,-103
};

static int mp_check_small_factors(const mp_int *X)
{
	u_long i;
	mp_limb r;
	int rv = 0;

	if ((X->p[0] & 1) == 0)
		return 1;

	for(i = 0; small_prime[i] > 0; i++) {
		if (mp_cmp_int(X, small_prime[i]) <= 0)
			return 1;

		MP_CHK(rv, mp_mod_int(&r, X, small_prime[i]), out);

		if (r == 0)
			return 1;
	}

out:
	return rv;
}

static int mp_miller_rabin(const mp_int *X, int (*f_rng)(void *, u_char *, u_long), void *p_rng)
{
	u_long i, j, k, n, s;
	mp_int W, R, T, A, RR;
	int rv, count;

	mp_init_multi(&W, &R, &T, &A, &RR, 0ul);

	MP_CHK(rv, mp_sub_int(&W, X, 1), out);
	s = mp_lsb(&W);
	MP_CHK(rv, mp_copy(&R, &W), out);
	MP_CHK(rv, mp_shift_r(&R, s), out);

	i = mp_bitlen(X);
	n = ((i >= 1300) ?  2 : (i >=  850) ?  3 :
	     (i >=  650) ?  4 : (i >=  350) ?  8 :
	     (i >=  250) ? 12 : (i >=  150) ? 18 : 27);

	for(i = 0; i < n; i++) {
		MP_CHK(rv, mp_random(&A, X->n * ciL, f_rng, p_rng), out);

		if (mp_cmp(&A, &W) >= 0) {
			j = mp_bitlen(&A) - mp_bitlen(&W);
			MP_CHK(rv, mp_shift_r(&A, j + 1 ), out);
		}
		A.p[0] |= 3;

		count = 0;
		do {
			MP_CHK(rv, mp_random(&A, X->n * ciL, f_rng, p_rng), out);

			j = mp_bitlen(&A);
			k = mp_bitlen(&W);
			if (j > k)
				MP_CHK(rv, mp_shift_r(&A, j - k), out);

			if (count++ > 30)
				return 1;
		} while(mp_cmp(&A, &W) >= 0 || mp_cmp_int(&A, 1) <= 0);

		MP_CHK(rv, mp_exp_mod(&A, &A, &R, X, &RR), out);

		if (!mp_cmp(&A, &W) || !mp_cmp_int(&A,  1))
			continue;

		for(j = 1; j < s && mp_cmp(&A, &W); ++j) {
			MP_CHK(rv, mp_mul(&T, &A, &A), out);
			MP_CHK(rv, mp_mod(&A, &T, X ), out);
			if (!mp_cmp_int(&A, 1))
				break;
		}

		if (mp_cmp(&A, &W)|| !mp_cmp_int(&A, 1)) {
			rv = 1;
			break;
		}
	}

out:
	mp_free_multi(&W, &R, &T, &A, &RR, 0ul);

	return rv;
}

int mp_is_prime(const mp_int *X, int (*f_rng)(void *, u_char *, u_long), void *p_rng)
{
	mp_int XX;
	int rv;

	XX.s = 1;
	XX.n = X->n;
	XX.p = X->p;

	if (!mp_cmp_int(&XX, 0) || !mp_cmp_int(&XX, 1))
		return 1;

	if (!mp_cmp_int(&XX, 2))
		return 0;

	if ((rv = mp_check_small_factors(&XX)))
		return rv == 1 ? 0 : rv;

	return mp_miller_rabin(&XX, f_rng, p_rng);
}

int mp_gen_prime(mp_int *X, u_long nbits, int dh_flag,
		 int (*f_rng)(void *, u_char *, u_long), void *p_rng)
{
	u_long k, n;
	mp_limb r;
	mp_int Y;
	int rv;

	if (nbits < 3 || nbits > MPI_MAX_BITS)
		return 1;

	mp_init(&Y);

	n = BITS_TO_LIMBS(nbits);

	MP_CHK(rv, mp_random(X, n * ciL, f_rng, p_rng), out);

	k = mp_bitlen(X);
	if (k > nbits)
		MP_CHK(rv, mp_shift_r(X, k - nbits + 1), out);

	mp_set_bit(X, nbits-1, 1);

	X->p[0] |= 1;
	if (!dh_flag) {
		while((rv = mp_is_prime(X, f_rng, p_rng))) {
			if (rv != 1)
				goto out;
			MP_CHK(rv, mp_add_int(X, X, 2), out);
		}
	}
	else {
		X->p[0] |= 2;

		MP_CHK(rv, mp_mod_int(&r, X, 3), out);
		if (!r)
			MP_CHK(rv, mp_add_int(X, X, 8), out);
		else if (r == 1)
			MP_CHK(rv, mp_add_int(X, X, 4), out);

		MP_CHK(rv, mp_copy(&Y, X), out);
		MP_CHK(rv, mp_shift_r(&Y, 1), out);

		while(1) {
			if (!(rv = mp_check_small_factors(X)) &&
			    !(rv = mp_check_small_factors(&Y)) &&
			    !(rv = mp_miller_rabin(X, f_rng, p_rng)) &&
			    !(rv = mp_miller_rabin(&Y, f_rng, p_rng)))
				break;

			if (rv != 1)
				goto out;

			MP_CHK(rv, mp_add_int( X,  X, 12), out);
			MP_CHK(rv, mp_add_int(&Y, &Y,  6), out);
		}
	}

out:
	mp_free(&Y);
	return rv;
}

int mp_legendre(const mp_int *U, const mp_int *N, int e)
{
	mp_int u, n, r;
	int rv = 1, t = 1;

	mp_init_multi(&u, &n, &r, 0ul);

	MP_CHK(rv, mp_copy(&u, U), out);
	MP_CHK(rv, mp_copy(&n, N), out);

	while (mp_cmp_int(&n, 1) > 0) {
		u_long bit = mp_bitlen(&u), tr;

		if (bit == 0) {
			t = 0;
			break;
		}
		if (bit == 1)
			break;

		MP_CHK(rv, mp_copy(&r, &u), out);
		if ((bit = mp_lsb(&r)))
			MP_CHK(rv, mp_shift_r(&r, bit), out);

		MP_CHK(rv, mp_mod_int(&tr, &n, 8ul), out);
		if ((bit & 1ul) && (tr == 3 || tr == 5))
			t = -t;
		if ((tr % 4) == 3) {
			MP_CHK(rv, mp_mod_int(&tr, &r, 4ul), out);
			if (tr == 3)
				t = -t;
		}

		MP_CHK(rv, mp_mod(&u, &n, &r), out);
		MP_CHK(rv, mp_copy(&n, &r), out);
	}

out:
	if (!rv)
		rv = (t != e);
	mp_free_multi(&u, &n, &r, 0ul);
	return rv;
}

int mp_md_step(struct hbelt_ctx *ctx, const mp_int *mp, int pad, int order)
{
	u_long len = mp_size(mp);
	u_char *buf = 0ul;
	int rv = 1;

	pad = pad == -1 ? len : pad;
	if (pad < len)
		goto out;

	if ((rv = !!len)) {
		if (!(buf = tzi_malloc(pad)))
			goto out;
		if (order == 1) {
			if ((rv = mp_write_binary(mp, buf, len, order)))
				goto out;
			tzi_memset(buf + len, 0, pad - len);
		}
		else {
			tzi_memset(buf, 0, pad - len);
			if ((rv = mp_write_binary(mp, buf + pad - len, len, order)))
				goto out;
		}
		hbelt_step(ctx, buf, pad);
	}
out:
	if (buf) {
		tzi_memcln(buf, pad);
		tzi_free(buf);
	}
	return rv;
}
