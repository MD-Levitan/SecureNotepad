
#include "crypto.h"

int main() {
    unsigned char *o, in[32]={49, 50, 51, 49, 50, 51, 49, 50, 51, 49, 50, 51, 49, 50, 51, 49, 50, 51, 49, 50, 51, 49, 50, 51, 49, 50, 51, 49, 50, 51, 49, 50};
    o = (unsigned char*) malloc(32);
    for(int i=0; i<32; ++i)
        printf("%d ", in[i]);
    //printf("\n");
    hash(in, 32, o, 32);
    for(int i=0; i<32; ++i)
            printf("%d ", o[i]);
    
    
    
    
}
//
//    tzi_init(malloc, free);
//    char k[] =	"\xb1\x94\xba\xc8\x0a\x08\xf5\x3b\x36\x6d\x00\x8e\x58\x4a\x5d\xe4"
//            "\x85\x04";
//    char h[] =	"\x5b\xe3\xd6\x12\x17\xb9\x61\x81\xfe\x67\x86\xad\x71\x6b\x89\x0b";
//    char e[] =	"\x9b\x4e\xa6\x69\xda\xbd\xf1\x00\xa7\xd4\xb6\xe6\xeb\x76\xee\x52"
//            "\x51\x91\x25\x31\xf4\x26\x75\x0a\xac\x8a\x9d\xbb\x51\xc5\x4d\x8d"
//            "\xeb\x92\x89\xb5\x0a\x46\x95\x2d\x05\x31\x86\x1e\x45\xa8\x81\x4b"
//            "\x00\x8f\xdc\x65\xde\x9f\xf1\xfa\x2a\x1f\x16\xb6\xa2\x80\xe9\x57"
//            "\xa8\x14";
//    unsigned char res[1024];
//    unsigned long len = 32;
//    unsigned char *pr =  (unsigned char*) malloc(len);
//    unsigned char *pub =  (unsigned char*) malloc(2*len);
//    for(int i=0; i<len; ++i)
//        printf("%d", pr[i]);
//
//    printf("\n");
//
//    for(int i=0; i<2*len; ++i)
//        printf("%d", pub[i]);
//
//    printf("\n");
//
//    asym_keys_generate(len, pr, len, pub, 2*len);
//
//    for(int i=0; i<len; ++i)
//        printf("%d", pr[i]);
//
//    printf("\n");
//
//    for(int i=0; i<2*len; ++i)
//        printf("%d", pub[i]);
//
//    printf("\n");
//
//
//    free(pr);
//    free(pub);
//
//
//
//
////    int l = 63;
////    unsigned char *data = (unsigned char *)malloc(l);
////    memset(data, 5, l);
////
////    unsigned  char *iv = (unsigned char *)malloc(16);
////    memset(iv, 2, 16);
////
////    unsigned char *key =(unsigned char *) malloc(32);
////    memset(key, 6, 32);
////
////
////    unsigned char *res = (unsigned char *)malloc(32);
////    hash(data, l, res, 32);
////    unsigned char *res2 = (unsigned char *)malloc(l);
////
////    serpent_encrypt(data, l, iv, 16, key, 32, res, l);
////    serpent_decrypt(res, l, iv, 16, key, 32, res2, l);
////
////
////     free(data);
////    free(iv);
////    free(key);
////        free(res);
////    free(res2);
//
//
//
//    return 0;
//}