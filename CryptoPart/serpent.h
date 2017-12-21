//
// Created by levitan on 16.10.17.
//
#ifndef SERPENT_H
#define SERPENT_H

#include <stdint.h>
#include <libnet.h>
#include <x86intrin.h>
//#include "macros.h"

#define GOLDEN_RATIO    0x9e3779b9l

#define SERPENT_ROUNDS      32
#define SERPENT_BLOCK_SIZE  16
#define SERPENT_KEY256      32

#define SERPENT_ENCRYPT  0
#define SERPENT_DECRYPT  1

#define SERPENT_IP       0
#define SERPENT_FP       1

#define U8V(v)  ((uint8_t)(v)  & 0xFFU)
#define U16V(v) ((uint16_t)(v) & 0xFFFFU)
#define U32V(v) ((uint32_t)(v) & 0xFFFFFFFFUL)
#define U64V(v) ((uint64_t)(v) & 0xFFFFFFFFFFFFFFFFULL)

#define ROTL8(v, n) \
  (U8V((v) << (n)) | ((v) >> (8 - (n))))

#define ROTL16(v, n) \
  (U16V((v) << (n)) | ((v) >> (16 - (n))))

#define ROTL32(v, n) \
  (U32V((v) << (n)) | ((v) >> (32 - (n))))

#define ROTL64(v, n) \
  (U64V((v) << (n)) | ((v) >> (64 - (n))))

#define ROTR8(v, n) ROTL8(v, 8 - (n))
#define ROTR16(v, n) ROTL16(v, 16 - (n))
#define ROTR32(v, n) ROTL32(v, 32 - (n))
#define ROTR64(v, n) ROTL64(v, 64 - (n))




typedef union _serpent_blk_t {
    uint8_t b[SERPENT_BLOCK_SIZE ];
    uint32_t w[SERPENT_BLOCK_SIZE /4];
    uint64_t q[SERPENT_BLOCK_SIZE /8];
} serpent_blk;

typedef uint32_t serpent_subkey_t[4];

typedef struct serpent_key_t {
    serpent_subkey_t x[SERPENT_ROUNDS+1];
} serpent_key;

#ifdef __cplusplus
extern "C" {
#endif



// C code
void serpent_set_encrypt_key(serpent_key*,  const unsigned char *);
void serpent_set_decrypt_key(serpent_key*,  const unsigned char *);
void serpent_encrypt_block (void *, serpent_key*);
void serpent_decrypt_block (void *, serpent_key*);


#ifdef __cplusplus
}
#endif

#endif //CRYPTOPART_SERPENT_H
