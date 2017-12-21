/*
 * Class with simple
 */
#ifndef __CRYPTO_H
#define __CRYPTO_H

#include "tzi_extra.h"
#include "belt.h"
#include "serpent.h"

/*
 * BIGN STB-34.101.45
 * Support next functions: 1. Generate private, public and one-off(k) keys.
 *                         2. Save and Load keys.
 *                         3. Transport keys(It means support decrypt and encrypt functions
 *                            symmetric key(in general any data) using elliptic curves).
 */



int priv_save(const struct bign_key*, unsigned char *, unsigned long);
int priv_load(const struct bign_key*, unsigned char *, unsigned long);

int public_save(const struct bign_key*, unsigned char *, unsigned long);
int public_load(const struct bign_key*, unsigned char *, unsigned long);

struct bign_key*  asym_keys_init(unsigned long type);
int asym_keys_generate(unsigned long, unsigned char *, unsigned long, unsigned char *, unsigned long);

int asym_decrypt(unsigned long,
                 unsigned char *, unsigned long,
                 unsigned char *, unsigned long, unsigned char *, unsigned long,
                 unsigned char *, unsigned long);
int asym_encrypt(unsigned long,
                 unsigned char *, unsigned long,
                 unsigned char *, unsigned long, unsigned char *, unsigned long,
                 unsigned char *, unsigned long);

int create_header(unsigned char *, unsigned long, unsigned char *, unsigned long);


///////////////////////////// /*BIGN*/


/*
 * BELT STB-34.101.31
 * Support next functions: 1. Range of functions to work with hash stuff(Init, step-hash, finish and clear context).
 *                         2. Symmetric cipher BELT, without any modes. It is used in BIGN, HBELT, KWR.
 *                         3. Stuff for protection keys(KWR - keywrap). It is used in BIGN.
 */

int hash(unsigned char *, unsigned long, unsigned char *, unsigned long);


///////////////////////////// /*BELT*/


/*
 * SERPENT -CBC
 * Support next functions: 1. Range of functions to work with hash stuff(Init, step-hash, finish and clear context).
 *                         2. Symmetric cipher BELT, without any modes. It is used in BIGN, HBELT, KWR.
 *                         3. Stuff for protection keys(KWR - keywrap). It is used in BIGN.
 */

void serpent_encrypt(unsigned char *, unsigned long, unsigned char *, unsigned long,
                     unsigned char *, unsigned long, unsigned char *, unsigned long);
void serpent_decrypt(unsigned char *, unsigned long, unsigned char *, unsigned long,
                     unsigned char *, unsigned long, unsigned char *, unsigned long);


void genKey(unsigned char *,long );

///////////////////////////// /*SERPENT*/


#endif //__CRYPTO_H //
