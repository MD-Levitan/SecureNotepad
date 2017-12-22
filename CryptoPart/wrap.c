//
// Created by levitan on 12/20/17.
//
#include "crypto.h"

int wrap_hash(unsigned char *i, unsigned long ilen, unsigned char *o, unsigned long olen) {
    hash(i, ilen, o, olen);
}

int wrap_serpent_encrypt(unsigned char *in, unsigned long inlen, unsigned char *iv, unsigned long ivlen,
                         unsigned char *key, unsigned long keylen, unsigned char *out, unsigned long outlen){
 
    serpent_encrypt(in, inlen, iv, ivlen, key, keylen, out, outlen);
}

int wrap_serpent_decrypt(unsigned char *in, unsigned long inlen, unsigned char *iv, unsigned long ivlen,
                         unsigned char *key, unsigned long keylen, unsigned char *out, unsigned long outlen){
    serpent_decrypt(in, inlen, iv, ivlen, key, keylen, out, outlen);

}

int wrap_genKey(unsigned char *in, unsigned long inlen){
	genKey(in, inlen);
}

int wrap_generate_keys(unsigned long level, unsigned char *pr, unsigned long prlen,
						 unsigned char *pub, unsigned long publen){
	tzi_init(malloc, free);					 
	asym_keys_generate(level, pr, prlen, pub, publen);
}

int wrap_asym_encrypt(	unsigned long level, 
						unsigned char *pub, unsigned long publen,
						unsigned char *in, unsigned long inlen,
						unsigned char *head, unsigned long headlen,
						unsigned char *out, unsigned long outlen){
	tzi_init(malloc, free);					 
	asym_encrypt(level, pub, publen, in, inlen, head, 16, out, outlen);
}

int wrap_asym_decrypt(	unsigned long level, 
						unsigned char *pr, unsigned long prlen,
						unsigned char *in, unsigned long inlen,
						unsigned char *head, unsigned long headlen,
						unsigned char *out, unsigned long outlen){
	tzi_init(malloc, free);					 
	asym_decrypt(level, pr, prlen, in, inlen, head, 16, out, outlen);
}

