// 7 april 2015
#include <string.h>
#include "uipriv_unix.h"

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

	out = g_malloc0(sizeof (size_t) + size);
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
	out = g_realloc(p, sizeof (size_t) + new);
	s = SIZE(out);
	if (new <= *s)
		memset(((uint8_t *) DATA(out)) + *s, 0, new - *s);
	*s = new;
	return DATA(out);
}

void uiFree(void *p)
{
	g_free(BASE(p));
}
