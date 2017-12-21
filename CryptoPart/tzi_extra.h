#ifndef __TZI_EXTRA_H__
#define __TZI_EXTRA_H__

#include <stdint.h>

#ifdef  __cplusplus
extern "C" {
#endif

#define BIGN_CURVE256V1 0ul
#define BIGN_CURVE384V1 1ul
#define BIGN_CURVE512V1 2ul

struct bign_key {
#define BIGN_PRIV_SET (1ul << 0)
#define BIGN_PUB_SET (1ul << 1)
#define BIGN_NO_PUBKEY (1ul << 2)
#define BIGN_NO_PARAMETERS (1ul << 3)
	unsigned long flags;
	unsigned long id;
};

struct bign_sig;

extern unsigned long belt_kwr_len(unsigned long, int);

extern struct bign_key *bign_key_new(void);
extern void bign_key_free(struct bign_key *);
extern int bign_curve_set(struct bign_key *, unsigned long);

extern unsigned long bign_priv_size(const struct bign_key *);
extern int bign_priv_save(const struct bign_key *, unsigned char *, unsigned long);
extern int bign_priv_load(struct bign_key *, const unsigned char *, unsigned long);
extern int bign_priv_generate(struct bign_key *);

extern unsigned long bign_pub_size(const struct bign_key *);
extern int bign_pub_save(const struct bign_key *, unsigned char *, unsigned long);
extern int bign_pub_load(struct bign_key *, const unsigned char *, unsigned long);
extern int bign_pub_generate(struct bign_key *);

extern unsigned long bign_k_size(const struct bign_key *);
extern int bign_k_save(const struct bign_key *, unsigned char *, unsigned long);
extern int bign_k_load(struct bign_key *, const unsigned char *, unsigned long);
extern int bign_k_generate(const unsigned char *, unsigned long,
			   const unsigned char *, unsigned long,
			   struct bign_key *, unsigned long);

extern struct bign_sig *bign_sign_new(void);
extern void bign_sign_free(struct bign_sig *);
extern unsigned long bign_sign_size(const struct bign_key *);
extern int bign_sign_save(const struct bign_key *, const struct bign_sig *, unsigned char *, unsigned long);
extern int bign_sign_load(const struct bign_key *, struct bign_sig *, const unsigned char *, unsigned long);
extern int bign_sign(const unsigned char *, unsigned long, const unsigned char *, unsigned long,
		     struct bign_key *, struct bign_sig *);
extern int bign_verify(const unsigned char *, unsigned long, const unsigned char *, unsigned long,
		       const struct bign_key *, const struct bign_sig *);

extern unsigned long bign_encrypt_size(const struct bign_key *, unsigned long, unsigned long);
extern int bign_encrypt(const struct bign_key *, const unsigned char *, unsigned long,
			const unsigned char *, unsigned long, unsigned char *, unsigned long);
extern unsigned long bign_decrypt_size(const struct bign_key *, unsigned long, unsigned long);
extern int bign_decrypt(const struct bign_key *, const unsigned char *, unsigned long,
			const unsigned char *, unsigned long, unsigned char *, unsigned long);

extern int bign_bdhe(const struct bign_key *, const struct bign_key *,
		     unsigned char *, unsigned long);

extern void belt_block(const void *, const void *, void *);

extern void pbkdf2(unsigned char *, const unsigned char *, unsigned long,
		   const unsigned char *, unsigned long, unsigned long);

#ifdef  __cplusplus
}
#endif

#endif /* __TZI_EXTRA_H__ */
