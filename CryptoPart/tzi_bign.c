#include "ec.h"
#include "tzi.h"
#include "tzi_lib.h"
#include "belt.h"
#include "tzi_u128.h"

#define bign_container(ptr) container_of(ptr, struct __bign_key, ext)

struct __bign_key {
	struct bign_key ext;
	ec_point pub_key;
	mp_int priv_key;
	mp_int k;
	const ecc *ec;
};

struct bign_sig {
	mp_int s0, s1;
};

static const char bign_curve256v1_field[] =
	"\x43\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff"
	"\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff";
static const char bign_curve256v1_a[] =
	"\x40\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff"
	"\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff";
static const char bign_curve256v1_b[] =
	"\xf1\x03\x9c\xd6\x6b\x7d\x2e\xb2\x53\x92\x8b\x97\x69\x50\xf5\x4c"
	"\xbe\xfb\xd8\xe4\xab\x3a\xc1\xd2\xed\xa8\xf3\x15\x15\x6c\xce\x77";
static const char bign_curve256v1_order[] =
	"\x07\x66\x3d\x26\x99\xbf\x5a\x7e\xfc\x4d\xfb\x0d\xd6\x8e\x5c\xd9"
	"\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff";
static const char bign_curve256v1_x[] =
	"\x00\x00\x00\x00\x00\x00\x00\x00";
static const char bign_curve256v1_y[] =
	"\x93\x6a\x51\x04\x18\xcf\x29\x1e\x52\xf6\x08\xc4\x66\x39\x91\x78"
	"\x5d\x83\xd6\x51\xa3\xc9\xe4\x5c\x9f\xd6\x16\xfb\x3c\xfc\xf7\x6b";
static const char bign_curve256v1_z[] =
	"\x01\x00\x00\x00\x00\x00\x00\x00";
static const char bign_curve256v1_two_inv_p[] =
	"\xa2\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff"
	"\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\x7f";

static const char bign_curve384v1_field[] =
	"\xc3\xfe\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff"
	"\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff"
	"\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff";
static const char bign_curve384v1_a[] =
	"\xc0\xfe\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff"
	"\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff"
	"\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff";
static const char bign_curve384v1_b[] =
	"\x64\xbf\x73\x68\x23\xfc\xa7\xbc\x7c\xbd\xce\xf3\xf0\xe2\xbd\x14"
	"\x3a\x2e\x71\xe9\xf9\x6a\x21\xa6\x96\xb1\xfb\x0f\xbb\x48\x27\x71"
	"\xd2\x34\x5d\x65\xab\x5a\x07\x33\x20\xef\x9c\x95\xe1\xdf\x75\x3c";
static const char bign_curve384v1_order[] =
	"\xb7\xa7\x0c\xf3\x3f\xdc\xb7\x3d\x0a\xff\xa4\xa6\xe7\xda\x46\x80"
	"\xbb\x7b\xaf\x73\x03\xc4\xcc\x6c\xfe\xff\xff\xff\xff\xff\xff\xff"
	"\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff";
static const char bign_curve384v1_x[] =
	"\x00\x00\x00\x00\x00\x00\x00\x00";
static const char bign_curve384v1_y[] =
	"\x51\xc4\x33\xf7\x31\xcb\x5e\xea\xf9\x42\x2a\x6b\x27\x3e\x40\x84"
	"\x55\xd3\xb1\x66\x9e\xe7\x49\x05\xa0\xff\x86\xdc\x11\x9a\x72\x3a"
	"\x89\xbf\x2d\x43\x7e\x11\x30\x63\x9e\x9e\x2e\xa8\x24\x82\x43\x5d";
static const char bign_curve384v1_z[] =
	"\x01\x00\x00\x00\x00\x00\x00\x00";
static const char bign_curve384v1_two_inv_p[] =
	"\x62\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff"
	"\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff"
	"\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\x7f";

static const char bign_curve512v1_field[] =
	"\xc7\xfd\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff"
	"\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff"
	"\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff"
	"\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff";
static const char bign_curve512v1_a[] =
	"\xc4\xfd\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff"
	"\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff"
	"\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff"
	"\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff";
static const char bign_curve512v1_b[] =
	"\x90\x9c\x13\xd6\x98\x69\x34\x09\x7a\xa2\x49\x3a\x27\x22\x86\xea"
	"\x43\xa2\xac\x87\x8c\x00\x33\x29\x95\x5e\x24\xc4\xb5\xdc\x11\x27"
	"\x88\xb0\xad\xda\xe3\x13\xce\x17\x51\x25\x5d\xdd\xee\xa9\xc6\x5b"
	"\x89\x58\xfd\x60\x6a\x5d\x8c\xd8\x43\x8c\x3b\x93\x44\x59\xb4\x6c";
static const char bign_curve512v1_order[] =
	"\xf1\x8e\x06\x0d\x49\xad\xff\xdc\x32\xdf\x56\x95\xe5\xca\x1b\x36"
	"\xf4\x13\x21\x2e\xb0\xeb\x6b\xf2\x4e\x00\x98\x01\x2c\x09\xc0\xb2"
	"\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff"
	"\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff";
static const char bign_curve512v1_x[] =
	"\x00\x00\x00\x00\x00\x00\x00\x00";
static const char bign_curve512v1_y[] =
	"\xbd\xed\xef\xce\x6f\xae\x92\xb7\x04\x0d\x4c\xc9\xb9\x83\xaa\x67"
	"\x61\x22\xe8\xee\x95\x73\x77\xff\xd2\x6f\xfa\x0e\xe2\xdd\x73\x69"
	"\xda\xca\xcc\x00\x1b\xf8\xed\xd2\xe2\xbc\x61\xb3\xb3\x41\xab\xb0"
	"\xab\x8f\xd1\xa0\xf7\xe6\x82\xb1\x81\x76\x03\xe4\x7a\xff\x26\xa8";
static const char bign_curve512v1_z[] =
	"\x01\x00\x00\x00\x00\x00\x00\x00";
static const char bign_curve512v1_two_inv_p[] =
	"\xe4\xfe\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff"
	"\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff"
	"\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff"
	"\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\x7f";

#define DEFINE_MP(n) { (void *)n, (sizeof(n) - 1) / sizeof(mp_limb), 1 }
const ecc bign_curves[] = {
{
	DEFINE_MP(bign_curve256v1_field),
	DEFINE_MP(bign_curve256v1_a),
	DEFINE_MP(bign_curve256v1_b),
	{ 0x5e,0x38,0x01,0x00,0x00,0x00,0x00,0x00 }, 8,
	DEFINE_MP(bign_curve256v1_order),
	1ul,
	{
		DEFINE_MP(bign_curve256v1_x),
		DEFINE_MP(bign_curve256v1_y),
		DEFINE_MP(bign_curve256v1_z),
	},
	DEFINE_MP(bign_curve256v1_two_inv_p)
},
{
	DEFINE_MP(bign_curve384v1_field),
	DEFINE_MP(bign_curve384v1_a),
	DEFINE_MP(bign_curve384v1_b),
	{ 0x23,0xaf,0x00,0x00,0x00,0x00,0x00,0x00 }, 8,
	DEFINE_MP(bign_curve384v1_order),
	1ul,
	{
		DEFINE_MP(bign_curve384v1_x),
		DEFINE_MP(bign_curve384v1_y),
		DEFINE_MP(bign_curve384v1_z),
	},
	DEFINE_MP(bign_curve384v1_two_inv_p)
},
{
	DEFINE_MP(bign_curve512v1_field),
	DEFINE_MP(bign_curve512v1_a),
	DEFINE_MP(bign_curve512v1_b),
	{ 0xae,0x17,0x02,0x00,0x00,0x00,0x00,0x00 }, 8,
	DEFINE_MP(bign_curve512v1_order),
	1ul,
	{
		DEFINE_MP(bign_curve512v1_x),
		DEFINE_MP(bign_curve512v1_y),
		DEFINE_MP(bign_curve512v1_z),
	},
	DEFINE_MP(bign_curve512v1_two_inv_p)
}
};

static int bign_point2oct(const ecc *ec, const ec_point *p,
			  u_char *bin, u_long len)
{
	u_long siz;
	int rv = 1;

	siz = (ec_get_degree(ec) + 7) / 8;
	if (bin && len == siz * 2) {
		mp_int x, y;
		mp_init_multi(&x, &y, 0ul);

		MP_CHK(rv, ecp_get_affine(ec, p, &x, &y), out);
		tzi_memset(bin, 0, len);
		mp_write_binary(&x, bin +   0, siz, 1);
		mp_write_binary(&y, bin + siz, siz, 1);
out:
		mp_free_multi(&x, &y, 0ul);
	}

	return rv;
}

static int bign_oct2point(const ecc *ec, ec_point *p,
			  const u_char *bin, u_long len)
{
	u_long siz;
	int rv = 1;

	siz = (ec_get_degree(ec) + 7) / 8;
	if (bin && siz * 2 == len) {
		mp_int x, y;
		mp_init_multi(&x, &y, 0ul);

		MP_CHK(rv, mp_read_binary(&x, bin +   0, siz, 1), out);
		MP_CHK(rv, mp_read_binary(&y, bin + siz, siz, 1), out);
		MP_CHK(rv, ecp_set_affine(ec, p, &x, &y), out);
out:
		mp_free_multi(&x, &y, 0ul);
	}

	return rv;
}

#define align(__n, __b)	((((__n) + (__b) - 1) / (__b)) * (__b))

static void bign_curve_init(struct __bign_key *key)
{
	key->ec = 0ul;
	key->ext.id = -1ul;
}

static void bign_curve_free(struct __bign_key *key)
{
	bign_curve_init(key);
}

static void bign_priv_init(struct __bign_key *key)
{
	mp_init(&key->priv_key);
	key->ext.flags &= ~BIGN_PRIV_SET;
}

static void bign_priv_free(struct __bign_key *key)
{
	mp_free(&key->priv_key);
	key->ext.flags &= ~BIGN_PRIV_SET;
}

static void bign_pub_init(struct __bign_key *key)
{
	ecp_init(&key->pub_key);
	key->ext.flags &= ~BIGN_PUB_SET;
}

static void bign_pub_free(struct __bign_key *key)
{
	ecp_free(&key->pub_key);
	key->ext.flags &= ~BIGN_PUB_SET;
}

struct bign_key *bign_key_new(void)
{
	struct __bign_key *rv = tzi_malloc(sizeof(*rv));
	if (!rv)
		return 0ul;
	bign_curve_init(rv);
	bign_priv_init(rv);
	bign_pub_init(rv);
   	
	mp_init(&rv->k);
	rv->ext.flags = 0ul;
	return &rv->ext;
}

void bign_key_free(struct bign_key *_key)
{
	struct __bign_key *key;
	if (!_key)
		return;
	key = bign_container(_key);
	mp_free(&key->k);
	bign_pub_free(key);
	bign_priv_free(key);
	bign_curve_free(key);
	tzi_memcln(key, sizeof(*key));
	tzi_free(key);
}

int bign_curve_set(struct bign_key *_key, unsigned long curve)
{
	struct __bign_key *key = bign_container(_key);
	int rv = 0;

	if (curve == BIGN_CURVE256V1 ||
	    curve == BIGN_CURVE384V1 ||
	    curve == BIGN_CURVE512V1) {
		key->ec = &bign_curves[curve];
		key->ext.id = curve;
	}
	else {
		rv = 1;
	}

	return rv;
}

/* priv_key support functions */
u_long bign_priv_size(const struct bign_key *_key)
{
	const struct __bign_key *key = bign_container(_key);
	return align(mp_size(&key->ec->order), 16);
}

int bign_priv_save(const struct bign_key *_key, u_char *bin, u_long len)
{
	const struct __bign_key *key = bign_container(_key);
	return mp_write_binary(&key->priv_key, bin, len, 1);
}

int bign_priv_load(struct bign_key *_key, const u_char *bin, u_long len)
{
	struct __bign_key *key = bign_container(_key);
	int rv = mp_read_binary(&key->priv_key, bin, len, 1);
	if (!rv)
		_key->flags |= BIGN_PRIV_SET;
	return rv;
}

int bign_priv_generate(struct bign_key *_key)
{
	struct __bign_key *key = bign_container(_key);
	ec_point q;
	mp_int d;
	int rv = 1;

	mp_init(&d);
	ecp_init(&q);

	if (!key || !entropy_f)
		goto out;

	do {
		do {
			MP_CHK(rv, mp_random(&d, mp_size(&key->ec->order),
					     entropy_f, entropy_p), out);
			MP_CHK(rv, mp_mod(&d, &d, &key->ec->order), out);
		} while (mp_cmp_int(&d, 0) == 0);
		MP_CHK(rv, ecp_mul(key->ec, &q, &d, &key->ec->group), out);
	} while (ecp_is_at_infinity(&q));

	mp_swap(&key->priv_key, &d);
	_key->flags |= BIGN_PRIV_SET;
out:
	ecp_free(&q);
	mp_free(&d);
	return rv;
}

/* pub_key support functions */
static int bign_valpubkey(const ecc *ec, const ec_point *q)
{
	int rv;
	mp_int t1, t2;

	mp_init_multi(&t1, &t2, 0ul);
	MP_CHK(rv, mp_cmp_int(&q->x, 0) < 0 || mp_cmp(&q->x, &ec->field) >= 0, out);
	MP_CHK(rv, mp_cmp_int(&q->y, 0) < 0 || mp_cmp(&q->y, &ec->field) >= 0, out);
	/* t1 <- x^3 */
	MP_CHK(rv, mp_mul(&t1, &q->x, &q->x), out);
	MP_CHK(rv, mp_mul(&t1, &t1, &q->x), out);
	/* t2 <- a*x */
	MP_CHK(rv, mp_mul(&t2, &q->x, &ec->a), out);
	/* t1 <- x^3 + a*x */
	MP_CHK(rv, mp_add(&t1, &t1, &t2), out);
	/* t1 <- x^3 + a*x + b */
	MP_CHK(rv, mp_add(&t1, &t1, &ec->b), out);
	/* t1 <- (x^3 + a*x + b) mod p */
	MP_CHK(rv, mp_mod(&t1, &t1, &ec->field), out);
	/* t2 <- y^2 mod p */
	MP_CHK(rv, mp_mul(&t2, &q->y, &q->y), out);
	MP_CHK(rv, mp_mod(&t2, &t2, &ec->field), out);
	MP_CHK(rv, mp_cmp(&t1, &t2) != 0, out);
out:
	mp_free_multi(&t1, &t2, 0ul);
	return rv;
}

u_long bign_pub_size(const struct bign_key *_key)
{
	const struct __bign_key *key = bign_container(_key);
	return align(mp_size(&key->ec->field), 16) * 2;
}

int bign_pub_save(const struct bign_key *_key, u_char *bin, u_long len)
{
	const struct __bign_key *key = bign_container(_key);
	return bign_point2oct(key->ec, &key->pub_key, bin, len);
}

int bign_pub_load(struct bign_key *_key, const u_char *bin, u_long len)
{
	struct __bign_key *key = bign_container(_key);
	ec_point q;
	int rv;

	ecp_init(&q);
	MP_CHK(rv, bign_oct2point(key->ec, &q, bin, len), out);
	MP_CHK(rv, bign_valpubkey(key->ec, &q), out);
	ecp_swap(&q, &key->pub_key);
	_key->flags |= BIGN_PUB_SET;
out:
	ecp_free(&q);
	return rv;
}

int bign_pub_generate(struct bign_key *_key)
{
	struct __bign_key *key = bign_container(_key);
	ec_point q;
	int rv = 1;

	ecp_init(&q);

	if (!key || !mp_cmp_int(&key->priv_key, 0) ||
	    mp_cmp(&key->priv_key, &key->ec->order) >= 0)
		goto out;

	MP_CHK(rv, ecp_mul(key->ec, &q, &key->priv_key, &key->ec->group), out);
	MP_CHK(rv, ecp_is_at_infinity(&q), out);

	ecp_swap(&key->pub_key, &q);
	_key->flags |= BIGN_PUB_SET;
out:
	ecp_free(&q);
	return rv;
}

/* genk support functions */
int bign_k_generate(const u_char *h, u_long hlen, const u_char *o, u_long olen,
		    struct bign_key *_key, u_long tlen)
{
	struct __bign_key *key = bign_container(_key);
	const ecc *ec = key->ec;
	struct hbelt_ctx ctx;
	u_char *t = 0ul, r[64], T[32], s[16];
	u_long i, n;
	mp_int mr;
	int rv;

	mp_init(&mr);
	MP_CHK(rv, !ec || !entropy_f || sizeof(r) < hlen, out);
	MP_CHK(rv, !(n = mp_size(&ec->order)) || (n != 32 && n != 48 && n != 64), out);
	/* 1. t <- rand(tlen) */
	MP_CHK(rv, !(t = tzi_malloc(tlen)), out);
	MP_CHK(rv, entropy_f(entropy_p, t, tlen), out);
	/* 2. T <- belt-hash(OID(h) || <d>_2l || t) */
	hbelt_init(&ctx);
	hbelt_step(&ctx, o, olen);
	MP_CHK(rv, mp_md_step(&ctx, &key->priv_key, n, 1), out);
	hbelt_step(&ctx, t, tlen);
	hbelt_fini(&ctx, T);
	/* 3. r <- H */
	tzi_memcpy(r, h, hlen);
	tzi_memset(r + hlen, 0, sizeof(r) - hlen);
	for(i = 1;; ++i) {
		switch (n) {
			/* 1) n == 2 */
			case 32:
				/* a) s <- r1 */
				u128_mov(s, r);
				break;
			/* 2) n == 3 */
			case 48:
				/* a) s <- r1 ^ r2 */
				u128_xor(s, r, r + 16);
				/* b) r1 <- r2 */
				u128_mov(r, r + 16);
				break;
			/* 3) n == 4 */
			case 64:
				/* a) s <- r1 ^ r2 ^ r3 */
				u128_xor(s, r, r + 16);
				u128_xor(s, s, r + 24);
				/* b) r1 <- r2 */
				u128_mov(r, r + 16);
				/* c) r2 <- r3 */
				u128_mov(r + 16, r + 24);
				break;
		}
		/* 4) r_(n-1) <- belt-block(s, T) ^ r_n ^ <i>_128 */
		belt_block(T, s, r + n - 32);
		u128_xor(r + n - 32, r + n - 32, r + n - 16);
		r[n - 32] ^= i;
		if (i > 0xff) {
			r[n - 32 + 1] ^= (i >>  8);
			r[n - 32 + 2] ^= (i >> 16);
			r[n - 32 + 3] ^= (i >> 24);
		}
		/* 5) r_n <- s */
		u128_mov(r + n - 16, s);
		if ((i % ((n / 16) * 2)) == 0) {
			MP_CHK(rv, mp_read_binary(&mr, r, n, 1), out);
			if (mp_cmp_int(&mr, 0) > 0 && mp_cmp(&mr, &ec->order) < 0)
				break;
		}
	}
	mp_swap(&mr, &key->k);
out:
	if (t) {
		tzi_memcln(t, tlen);
		tzi_free(t);
	}
	tzi_memcln(T, sizeof(T));
	tzi_memcln(r, sizeof(r));
	tzi_memcln(s, sizeof(s));
	mp_free(&mr);
	return rv;
}

unsigned long bign_k_size(const struct bign_key *_key)
{
	const struct __bign_key *key = bign_container(_key);
	return mp_size(&key->k);
}

int bign_k_save(const struct bign_key *_key, unsigned char *k, unsigned long klen)
{
	const struct __bign_key *key = bign_container(_key);
	u_long tlen;
	int rv;

	MP_CHK(rv, (!(tlen = bign_k_size(_key)) || !k || klen < tlen), out);
	MP_CHK(rv, mp_write_binary(&key->k, k, klen, 1), out);
out:
	return rv;
}

int bign_k_load(struct bign_key *_key, const unsigned char *k, unsigned long klen)
{
	struct __bign_key *key = bign_container(_key);
	int rv;

	MP_CHK(rv, !k, out);
	MP_CHK(rv, mp_read_binary(&key->k, k, klen, 1), out);
out:
	return rv;
}

/* sign support functions */
struct bign_sig *bign_sign_new(void)
{
	struct bign_sig *rv = tzi_malloc(sizeof(*rv));
	if (rv)
		mp_init_multi(&rv->s0, &rv->s1, 0ul);
	return rv;
}

void bign_sign_free(struct bign_sig *sig)
{
	if (sig) {
		mp_free_multi(&sig->s0, &sig->s1, 0ul);
		tzi_free(sig);
	}
}

u_long bign_sign_size(const struct bign_key *_key)
{
	struct __bign_key *key = bign_container(_key);
	return key ? (align(mp_size(&key->ec->order), 16) / 2) * 3 : 0;
}

int bign_sign_save(const struct bign_key *key, const struct bign_sig *sig, u_char *bin, u_long len)
{
	u_long s0len, s1len, siz;
	int rv = 1;

	s0len = mp_size(&sig->s0);
	s1len = mp_size(&sig->s1);
	siz = bign_sign_size(key);
	if (bin && siz <= len && (s0len + s1len) <= siz) {
		siz /= 3;
		MP_CHK(rv, mp_write_binary(&sig->s0, bin, siz, 1), out);
		rv = mp_write_binary(&sig->s1, bin + siz, siz * 2, 1);
		if (rv)
			tzi_memset(bin, 0, siz);
	}
out:
	return rv;
}

int bign_sign_load(const struct bign_key *key, struct bign_sig *sig, const u_char *bin, u_long len)
{
	mp_int s0, s1;
	u_long siz;
	int rv = 1;

	mp_init_multi(&s0, &s1, 0ul);
	siz = bign_sign_size(key);
	if (bin && len == siz) {
		len /= 3;
		MP_CHK(rv, mp_read_binary(&s0, bin, len, 1), out);
		MP_CHK(rv, mp_read_binary(&s1, bin + len, len * 2, 1), out);
		mp_swap(&sig->s0, &s0);
		mp_swap(&sig->s1, &s1);
	}
out:
	mp_free_multi(&s0, &s1, 0ul);
	return rv;
}

int bign_sign(const u_char *dgst, u_long dlen, const u_char *oid, u_long olen,
	      struct bign_key *_key, struct bign_sig *sig)
{
	struct __bign_key *key = bign_container(_key);
	u_char buf[TZI_MD_MAX_BLOCK_SIZE];
	struct hbelt_ctx ctx;
	mp_int s0, s1, H, rx;
	ec_point R;
	u_long siz = bign_sign_size(_key) / 3, n;
	int rv = 1;

	if (!entropy_f || !sig || (n = mp_size(&key->ec->order)) == 0ul)
		return 1;

	mp_init_multi(&s0, &s1, &H, &rx, 0ul);
	ecp_init(&R);

	if (mp_cmp_int(&key->k, 0) > 0 && mp_cmp(&key->k, &key->ec->order) < 0) {
		MP_CHK(rv, ecp_mul(key->ec, &R, &key->k, &key->ec->group), out);
		goto skip_k_gen;
	}
	do {
		u_long tlen = 0ul;
		MP_CHK(rv, entropy_f(entropy_p, (void *) &tlen, sizeof(tlen)), out);
		/* generate at least 32 bytes */
		tlen = (tlen % 257ul) + 32ul;
		MP_CHK(rv, bign_k_generate(dgst, dlen, oid, olen, _key, tlen), out);
		MP_CHK(rv, ecp_mul(key->ec, &R, &key->k, &key->ec->group), out);
	} while (ecp_is_at_infinity(&R));
skip_k_gen:
	MP_CHK(rv, ecp_get_affine(key->ec, &R, &rx, 0ul), out);
	MP_CHK(rv, mp_read_binary(&H, dgst, dlen, 1), out);

	hbelt_init(&ctx);
	hbelt_step(&ctx, oid, olen);
	MP_CHK(rv, mp_md_step(&ctx, &rx, n, 1), out);
	hbelt_step(&ctx, dgst, dlen);
	hbelt_fini(&ctx, buf);
	MP_CHK(rv, mp_read_binary(&s0, buf, siz, 1), out);
	MP_CHK(rv, mp_copy(&s1, &s0), out);
	MP_CHK(rv, mp_set_bit(&s1, mp_bitlen(&key->ec->order) / 2, 1), out);
	MP_CHK(rv, mp_mul(&s1, &key->priv_key, &s1), out);
	MP_CHK(rv, mp_add(&s1, &H, &s1), out);
	MP_CHK(rv, mp_sub(&s1, &key->k, &s1), out);
	MP_CHK(rv, mp_mod(&s1, &s1, &key->ec->order), out);

	mp_swap(&sig->s0, &s0);
	mp_swap(&sig->s1, &s1);

out:
	tzi_memcln(buf, sizeof(buf));
	ecp_free(&R);
	mp_free_multi(&s0, &s1, &H, &rx, &key->k, 0ul);
	return rv;
}

int bign_verify(const u_char *dgst, u_long dlen, const u_char *oid, u_long olen,
		const struct bign_key *_key, const struct bign_sig *sig)
{
	const struct __bign_key *key = bign_container(_key);
	u_char buf[TZI_MD_MAX_BLOCK_SIZE];
	u_long s0len, s1len;
	struct hbelt_ctx ctx;
	ec_point R, G;
	mp_int H;
	u_long siz = bign_sign_size(_key) / 3, n;
	int rv = 1;

	if (!sig || (n = mp_size(&key->ec->order)) == 0ul)
		return 1;

	ecp_init(&R);
	ecp_init(&G);
	mp_init(&H);

	s0len = mp_size(&sig->s0);
	s1len = mp_size(&sig->s1);
	if (bign_sign_size(_key) < s0len + s1len)
		goto out;
	if (mp_cmp(&sig->s1, &key->ec->order) >= 0)
		goto out;

	MP_CHK(rv, mp_read_binary(&H, dgst, dlen, 1), out);
	MP_CHK(rv, mp_add(&H, &H, &sig->s1), out);
	MP_CHK(rv, mp_mod(&H, &H, &key->ec->order), out);
	MP_CHK(rv, ecp_mul(key->ec, &G, &H, &key->ec->group), out);

	MP_CHK(rv, mp_copy(&H, &sig->s0), out);
	MP_CHK(rv, mp_set_bit(&H, mp_bitlen(&key->ec->order) / 2, 1), out);
	MP_CHK(rv, ecp_mul(key->ec, &R, &H, &key->pub_key), out);

	MP_CHK(rv, ecp_add(key->ec, &R, &R, &G), out);
	MP_CHK(rv, ecp_is_at_infinity(&R), out);

	MP_CHK(rv, ecp_get_affine(key->ec, &R, &H, 0ul), out);
	hbelt_init(&ctx);
	hbelt_step(&ctx, oid, olen);
	MP_CHK(rv, mp_md_step(&ctx, &H, n, 1), out);
	hbelt_step(&ctx, dgst, dlen);
	hbelt_fini(&ctx, buf);

	MP_CHK(rv, mp_read_binary(&H, buf, siz, 1), out);
	if (mp_cmp(&sig->s0, &H))
		rv = 1;

out:
	tzi_memcln(buf, sizeof(buf));
	mp_free(&H);
	ecp_free(&G);
	ecp_free(&R);
	return rv;
}

u_long bign_encrypt_size(const struct bign_key *_key, u_long klen, u_long hlen)
{
	const struct __bign_key *key = bign_container(_key);
	u_long rv = hlen == 16 ? belt_kwr_len(klen, 1) : 0;
	if (rv)
		rv += mp_size(&key->ec->field);
	return rv;
}

int bign_encrypt(const struct bign_key *_key, const u_char *ikey, u_long ilen,
		 const u_char *hdr, u_long hlen, u_char *out, u_long len)
{
	const struct __bign_key *key = bign_container(_key);
	struct kwr_ctx ctx;
	mp_int k;
	ec_point R, Q;
	int rv = 1;
	u_char *buf = 0ul; /* auth || skey */
	u_long blen = 0, degree = 0;

	mp_init(&k);
	ecp_init(&R);
	ecp_init(&Q);
	belt_kwr_init(&ctx);

	if (!entropy_f || bign_encrypt_size(_key, ilen, hlen) != len)
		goto out;

	do {
		do {
			do {
				MP_CHK(rv, mp_random(&k, mp_size(&key->ec->order),
						     entropy_f, entropy_p), out);
				MP_CHK(rv, mp_mod(&k, &k, &key->ec->order), out);
			} while (mp_cmp_int(&k, 0) == 0);
			MP_CHK(rv, ecp_mul(key->ec, &R, &k, &key->ec->group), out);
		} while (ecp_is_at_infinity(&R));
		MP_CHK(rv, ecp_mul(key->ec, &Q, &k, &key->pub_key), out);
	} while (ecp_is_at_infinity(&Q));

	blen = (degree = (ec_get_degree(key->ec) + 7) / 8) * 4;
	MP_CHK(rv, (!(buf = tzi_malloc(blen))), out);
	tzi_memset(buf, 0, blen);
	MP_CHK(rv, bign_point2oct(key->ec, &R, buf, degree * 2), out);
	MP_CHK(rv, bign_point2oct(key->ec, &Q, buf + degree * 2, degree * 2), out);
	MP_CHK(rv, !belt_kwr_ctrl(&ctx, KWR_CTRL_SET_KEY, buf + degree * 2, 32), out);
	MP_CHK(rv, !belt_kwr_ctrl(&ctx, KWR_CTRL_SET_IV, (void *)hdr, hlen), out);
	if (belt_kwr_estep(&ctx, out + degree, ikey, ilen) != len - degree) {
		rv = 1;
		goto out;
	}
	tzi_memcpy(out, buf, degree);

out:
	if (buf) {
		tzi_memcln(buf, blen);
		tzi_free(buf);
	}
	belt_kwr_clear(&ctx);
	ecp_free(&Q);
	ecp_free(&R);
	mp_free(&k);
	return rv;
}

u_long bign_decrypt_size(const struct bign_key *_key, u_long klen, u_long hlen)
{
	const struct __bign_key *key = bign_container(_key);
	u_long rv = hlen == 16 ? belt_kwr_len(klen, 0) : 0;
	if (rv)
		rv -= mp_size(&key->ec->field);
	return rv;
}

int bign_decrypt(const struct bign_key *_key, const u_char *ikey, u_long ilen,
		 const u_char *hdr, u_long hlen, u_char *out, u_long len)
{
	const struct __bign_key *key = bign_container(_key);
	struct kwr_ctx ctx;
	mp_int xr, yr, t1, t2;
	ec_point R;
	u_char *buf = 0ul;
	u_long blen, degree;
	int rv = 1;

	mp_init_multi(&xr, &yr, &t1, &t2, 0ul);
	ecp_init(&R);
	belt_kwr_init(&ctx);

	if (bign_decrypt_size(_key, ilen, hlen) != len)
		goto out;

	degree = (ec_get_degree(key->ec) + 7) / 8;
	blen = len + degree * 2 + hlen;
	buf = tzi_malloc(blen);
	if (!buf)
		goto out;

	MP_CHK(rv, mp_read_binary(&xr, ikey, degree, 1), out);
	if (mp_cmp(&xr, &key->ec->field) >= 0) {
		rv = 1;
		goto out;
	}

	/* t1 <- xr^3 + a*xr + b */
	MP_CHK(rv, mp_mul(&t1, &xr, &xr), out);
	MP_CHK(rv, mp_mul(&t1, &t1, &xr), out);
	MP_CHK(rv, mp_mul(&t2, &key->ec->a, &xr), out);
	MP_CHK(rv, mp_add(&t1, &t1, &t2), out);
	MP_CHK(rv, mp_add(&t1, &t1, &key->ec->b), out);

	/* t2 <- (p + 1) / 4 */
	MP_CHK(rv, mp_add_int(&t2, &key->ec->field, 1), out);
	MP_CHK(rv, mp_div_int(&t2, 0ul, &t2, 4), out);

	/* yr <- (xr^3 + a*xr + b) ^((p + 1) / 4) mod p */
	MP_CHK(rv, mp_exp_mod(&yr, &t1, &t2, &key->ec->field, 0ul), out);
	/* t2 <- yr^2 */
	MP_CHK(rv, mp_mul(&t2, &yr, &yr), out);
	MP_CHK(rv, mp_mod(&t2, &t2, &key->ec->field), out);
	/* t1 <- (xr^3 + a*xr + b) mod p */
	MP_CHK(rv, mp_mod(&t1, &t1, &key->ec->field), out);
	if (mp_cmp(&t2, &t1) != 0) {
		rv = 1;
		goto out;
	}

	MP_CHK(rv, ecp_set_affine(key->ec, &R, &xr, &yr), out);
	MP_CHK(rv, ecp_mul(key->ec, &R, &key->priv_key, &R), out);

	MP_CHK(rv, bign_point2oct(key->ec, &R, buf, degree * 2), out);

	MP_CHK(rv, !belt_kwr_ctrl(&ctx, KWR_CTRL_SET_KEY, buf, 32), out);
	MP_CHK(rv, !belt_kwr_ctrl(&ctx, KWR_CTRL_SET_IV, (void *)hdr, hlen), out);
	if (belt_kwr_dstep(&ctx, buf + degree * 2, ikey + degree, ilen - degree) != len) {
		rv = 1;
		goto out;
	}
	tzi_memcpy(out, buf + degree * 2, len);

out:
	if (buf) {
		tzi_memcln(buf, blen);
		tzi_free(buf);
	}
	belt_kwr_clear(&ctx);
	ecp_free(&R);
	mp_free_multi(&xr, &yr, &t1, &t2, 0ul);
	return rv;
}

int bign_bdhe(const struct bign_key *_kd, const struct bign_key *_kq,
	      unsigned char *buf, unsigned long len)
{
	const struct __bign_key *kd = bign_container(_kd);
	const struct __bign_key *kq = bign_container(_kq);
	ec_point R;
	int rv = 1;

	ecp_init(&R);

	if (_kd->id != _kq->id)
		goto out;

	MP_CHK(rv, ecp_mul(kd->ec, &R, &kd->priv_key, &kq->pub_key), out);
	rv = bign_point2oct(kd->ec, &R, buf, len);

out:
	ecp_free(&R);
	return rv;
}
