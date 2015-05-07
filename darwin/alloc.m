// 4 december 2014
#import <stdlib.h>
#import "uipriv_darwin.h"

void initAlloc(void)
{
}

#define UINT8(p) ((uint8_t *) (p))
#define PVOID(p) ((void *) (p))
#define DATA(p) PVOID(UINT8(p) + sizeof (size_t))
#define BASE(p) PVOID(UINT8(p) - sizeof (size_t))
#define SIZE(p) ((size_t *) (p))

void *uiAlloc(size_t size)
{
	void *out;

	out = malloc(sizeof (size_t) + size);
	if (out == NULL) {
		fprintf(stderr, "memory exhausted in uiAlloc()\n");
		abort();
	}
	memset(DATA(out), 0, size);
	*SIZE(out) = size;
	return DATA(out);
}

void *uiRealloc(void *p, size_t new)
{
	void *out;
	size_t *s;

	if (p == NULL)
		return uiAlloc(new);
	p = BASE(p);
	out = realloc(p, sizeof (size_t) + new);
	if (out == NULL) {
		fprintf(stderr, "memory exhausted in uiRealloc()\n");
		abort();
	}
	s = SIZE(out);
	if (new <= *s)
		memset(((uint8_t *) DATA(out)) + *s, 0, new - *s);
	*s = new;
	return DATA(out);
}

void uiFree(void *p)
{
	if (p == NULL)
		complain("attempt to uiFree(NULL); there's a bug somewhere");
	free(BASE(p));
}
