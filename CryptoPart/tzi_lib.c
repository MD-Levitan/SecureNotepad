#if defined(MEMORY_DEBUG)
#define _BSD_SOURCE
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#endif
#include "tzi_lib.h"

int (*entropy_f)(void *, u_char *, u_long);
void *entropy_p;

static void *(*malloc_fnc)(u_long) = 0ul;
static void (*free_fnc)(void *) = 0ul;

#if defined(MEMORY_DEBUG)
static void tzi_on_exit(int, void *);
#endif

void tzi_init(void *(*a)(u_long), void (*f)(void *))
{
#if defined(MEMORY_DEBUG)
	on_exit(tzi_on_exit, 0ul);
#endif
	malloc_fnc = a;
	free_fnc = f;
}

#if defined(MEMORY_DEBUG)
struct list {
	struct list *n;
	void *ptr;
	u_long siz;
	const char *fnc;
	int lin;
};

static struct list *head = 0ul;

static void tzi_on_exit(int status, void *arg)
{
	struct list *p;
	u_long c;
	for(p = head, c = 0ul; p; p = p->n) {
		fprintf(stderr, "memory leak detected: %p (%lu bytes) allocated at %s, %d\n",
			p->ptr, p->siz, p->fnc, p->lin);
		c += p->siz;
	}
	if (c)
		fprintf(stderr, "%lu bytes leaked :(\n", c);
	else
		fprintf(stderr, "all memory freed\n");
}

void *__tzi_malloc(u_long siz, const char *fnc, int lin)
{
	void *rv = malloc_fnc(siz);
	if (rv) {
		struct list *n = malloc_fnc(sizeof(*n));
		assert(n != 0ul);
		n->n = head;
		n->ptr = rv;
		n->siz = siz;
		n->fnc = fnc;
		n->lin = lin;
		head = n;
	}
	return rv;
}
#else
void *tzi_malloc(u_long siz)
{
	return malloc_fnc(siz);
}
#endif

void tzi_free(void *ptr)
{
#if defined(MEMORY_DEBUG)
	struct list *p = 0ul, *c = head;
	assert(head != 0ul);
	while (c) {
		if (c->ptr == ptr) {
			if (p)
				p->n = c->n;
			else
				head = c->n;
			break;
		}
		p = c;
		c = c->n;
	}
	assert(c != 0ul);
	free_fnc(c);
#endif
	free_fnc(ptr);
}

void tzi_memcln(void *ptr, u_long len)
{
	tzi_memset(ptr, 0, len);
}

void tzi_memcpy(void *d, const void *s, u_long l)
{
	const u_char *src = s;
	u_char *dst = d;
	while (l--)
		*dst++ = *src++;
}

void tzi_memset(void *d, int n, u_long l)
{
	u_char *dst = d;
	while (l--)
		*dst++ = n;
}

int tzi_memcmp(const void *a, const void *b, u_long l)
{
	const u_char *lh = a, *rh = b;
	u_long i;
	int rv = 0;

	for(i = 0; i < l; ++i) {
		rv = (int)lh[i] - (int)rh[i];
		if (rv)
			break;
	}

	return rv;
}

u_long tzi_strlen(const char *s)
{
	u_long rv = 0;
	while (s[rv])
		rv++;
	return rv;
}

int tzi_strcmp(const char *s1, const char *s2)
{
	for(; *s1 && *s2; s1++, s2++) {
		if (*s1 != *s2)
			break;
	}
	return (int)*s1 - (int)*s2;
}
