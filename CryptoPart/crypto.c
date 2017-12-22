#include <time.h>
#include "crypto.h"

int randomG(void* _, unsigned char *buf, unsigned long len){
    srand(time(NULL));
    for(unsigned  long i = 0; i < len; ++i)
        buf[i] = (unsigned char) (rand() ^ rand());
    
    return 0;
}

int hash(unsigned char *i, unsigned long ilen, unsigned char *o, unsigned long olen) {
    int rv = olen == 32 ? 0  : 1;
    if(rv)
        return rv;
    
    struct hbelt_ctx ctx;
    hbelt_init(&ctx);
    hbelt_step(&ctx, i, ilen);
    hbelt_fini(&ctx, o);
}

struct bign_key * asym_keys_init(unsigned long type) {
    struct bign_key  *key;
    key = bign_key_new();
    
    switch (type) {
        case 128:
            bign_curve_set(key, BIGN_CURVE256V1);
            break;
        case 256:
            bign_curve_set(key, BIGN_CURVE512V1);
            break;
        case 192:
            bign_curve_set(key, BIGN_CURVE384V1);
            break;
        default:
            bign_curve_set(key, BIGN_CURVE256V1);
    }
    
    return key;
}

int asym_keys_generate(unsigned long len, unsigned char *pr, unsigned long prlen, unsigned char *pb, unsigned long pblen){
    struct bign_key *key  = asym_keys_init(len);
    entropy_f = randomG;
    if(bign_priv_generate(key))
        return 1;
    if(bign_pub_generate(key))
        return 1;
    priv_save(key, pr, prlen);
    public_save(key, pb, pblen);
    return 0;
}

int priv_save(const struct bign_key *key, unsigned char *o, unsigned long olen) {
    return bign_priv_save(key,o,olen);
}

int priv_load(const struct bign_key *key, unsigned char *o, unsigned long olen) {
    return bign_priv_load(key,o,olen);
}

int public_save(const struct bign_key *key, unsigned char *o, unsigned long olen) {
    return bign_pub_save(key,o,olen);
}

int public_load(const struct bign_key *key, unsigned char *o, unsigned long olen) {
    return bign_pub_load(key,o,olen);
}

int asym_decrypt(unsigned long len,
                 unsigned char *pr, unsigned long prlen,
                 unsigned char *i, unsigned long ilen, unsigned char *d, unsigned long dlen,
                 unsigned char * o, unsigned long olen) {
    struct bign_key *key = asym_keys_init(len);
    priv_load(key, pr, prlen);
    entropy_f = randomG;
    return bign_decrypt(key, i, ilen, d, dlen, o, olen);
}

int asym_encrypt(unsigned long len,
                 unsigned char *pb, unsigned long pblen,
                 unsigned char *i, unsigned long ilen, unsigned char *d, unsigned long dlen,
                 unsigned char * o, unsigned long olen) {
    struct bign_key *key = asym_keys_init(len);
    public_load(key, pb, pblen);
    entropy_f = randomG;
    return bign_encrypt(key, i, ilen, d, dlen, o, olen);
}

int create_header(unsigned char *i, unsigned long ilen, unsigned char *o, unsigned long olen){
    entropy_f = randomG;
    if(i == NULL || !ilen)
        memset(o, 0, olen);
    unsigned char hash_data[32];
    hash(i, ilen, hash_data, 32);
    memccpy(i, o, ilen, sizeof(unsigned char));
    memset(hash_data, 0, 32);
    return 0;
}


void serpent_encrypt(unsigned char *in, unsigned long inlen, unsigned char *iv, unsigned long ivlen,
                     unsigned char *key, unsigned long keylen, unsigned char *out, unsigned long outlen){
                     
	
	outlen =  inlen;
	
    serpent_key *key_st = (serpent_key*) malloc(sizeof(serpent_key));
    
    if(keylen != 32 || ivlen != 16 || outlen != inlen || inlen < 16)
        	return;
    serpent_set_encrypt_key(key_st, key);
    int64_t blocks = (inlen + 15) / 16, iter = 0;
    int16_t last_block = inlen % 16;
    void *block;
    block = malloc(16);
    ///Y_0 = IV
    memcpy(block, iv, 16);
    while(iter < blocks - 2 || (!last_block && (iter < blocks))){
        for(int32_t index = 0; index < 16; ++index)
            ((uint8_t*)block)[index] ^= in[index + 16 * iter];
        serpent_encrypt_block(block, key_st);
        memcpy(out + 16 * iter, block, 16);
        iter++;
    }
    ///Padding
    if(last_block){
        ////1. Y_n || r = F(X_(n-1) ^ Y_(n-2))
        for(int32_t index = 0; index < 16; ++index)
            ((uint8_t*)block)[index] ^= in[index + 16 * iter];
        serpent_encrypt_block(block, key_st);
        memcpy(out + 16 + 16 * iter, block, last_block);
        iter++;
        
        ////2. Y_(n-1) = F((X_n ^ Y_n) || r)
        for(int32_t index = 0; index < last_block; ++index)
            ((uint8_t*)block)[index] ^= in[index + 16 * iter];
        serpent_encrypt_block(block, key_st);
        memcpy(out + 16 * (iter-1), block, 16);
    }
    if(block != NULL)
        free(block);
    if(key_st != NULL)
        free(key_st);
}

void serpent_decrypt(unsigned char *in, unsigned long inlen, unsigned char *iv, unsigned long ivlen,
                     unsigned char *key, unsigned long keylen, unsigned char *out, unsigned long outlen){
    outlen =  inlen;
    
    serpent_key *key_st = (serpent_key*) malloc(sizeof(serpent_key));
    if(keylen != 32 || ivlen != 16 || outlen != inlen || inlen < 16)
        return;
    serpent_set_decrypt_key(key_st,key);
    
    int64_t blocks = (inlen + 15) / 16, iter = 0;
    int16_t last_block = inlen % 16;
    
    void *block;
    block = malloc(16);
    memcpy(block, in, 16);
    
    while(iter < blocks - 2 || (!last_block && (iter < blocks))){
        if(iter)
            memcpy(block, in + iter * 16, 16);
        serpent_decrypt_block(block, key_st);
        if(iter)
            for(int32_t index = 0; index < 16; ++index)
                ((uint8_t*)block)[index] ^= in[index + 16 * (iter-1)];
        else
            for(int32_t index = 0; index < 16; ++index)
                ((uint8_t*)block)[index] ^= iv[index];
        memcpy(out + 16 * iter, block, 16);
        iter++;
    }
    ///Padding
    if(last_block){
        ////1. Y_n || r = D(X_(n-1)) ^ (X_(n) || O^(16 - |X_n|))
        if(iter)
            memcpy(block, in + iter * 16, 16);
        serpent_decrypt_block(block, key_st);
        for(int32_t index = 0; index < last_block; ++index)
            ((uint8_t*)block)[index] ^= in[index + 16 * (iter + 1)];
        
        memcpy(out + 16 + 16 * iter, block, last_block);
        iter++;
        
        ////2. Y_(n-1) = D((X_n || r)^ X_(n_2)
        memcpy(block,in + 16 * iter, last_block);
        serpent_decrypt_block(block, key_st);
        for(int32_t index = 0; index < 16; ++index)
            if(blocks == 2)
                ((uint8_t*)block)[index] ^= iv[index];
            else
                ((uint8_t*)block)[index] ^= in[index + (iter - 2) * 16];
        memcpy(out + 16 * (iter-1), block, 16);
    }
    if(block != NULL)
        free(block);
    if(key_st != NULL)
        free(key_st);
    
}

void genKey(unsigned char *key, long len){
    srand(time(NULL));
    for(long index=0; index < len; ++ index){
        key[index] = (unsigned char) (rand() ^ rand());
    }
}
