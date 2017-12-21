#ifndef __TZI_LIB_H__
#define __TZI_LIB_H__

#ifdef  __cplusplus
extern "C" {
#endif

#if defined(_WIN32)
#define __inline
#else
#define __inline __inline__
#endif

typedef unsigned char u_char;
typedef unsigned short u_short;
typedef unsigned int u_int;
typedef unsigned long u_long;

#if defined(MEMORY_DEBUG)
extern void *__tzi_malloc(u_long, const char *, int);
#define tzi_malloc(s) __tzi_malloc(s, __func__, __LINE__)
#else
extern void *tzi_malloc(u_long);
#endif
extern void tzi_free(void *);

extern void tzi_memcln(void *, u_long);
extern void tzi_memcpy(void *, const void *, u_long);
extern void tzi_memset(void *, int, u_long);
extern int tzi_memcmp(const void *, const void *, u_long);

extern unsigned long tzi_strlen(const char *);
extern int tzi_strcmp(const char *, const char *);

#define offsetof(type, member) ((u_long)&((type *)0)->member)
#define container_of(ptr, type, member) \
	((type *) (((u_char *) ptr) - offsetof(type, member)))
#define static_assert(e, n) enum { assert_line_ ## n = 1 / (!!(e)) }

#ifdef  __cplusplus
}
#endif

#endif /* __TZI_LIB_H__ */
