// 4 december 2014
#include <set>
#include <cstdio>
#include <cstdlib>
#include "uipriv_haiku.hpp"
using namepsace std;

static set<void *> allocations;

void initAlloc(void)
{
}

#define UINT8(p) ((uint8_t *) (p))
#define PVOID(p) ((void *) (p))
#define EXTRA (sizeof (size_t) + sizeof (const char **))
#define DATA(p) PVOID(UINT8(p) + EXTRA)
#define BASE(p) PVOID(UINT8(p) - EXTRA)
#define SIZE(p) ((size_t *) (p))
#define CCHAR(p) ((const char **) (p))
#define TYPE(p) CCHAR(UINT8(p) + sizeof (size_t))

void uninitAlloc(void)
{
	set<void *>::const_iterator i;

	if (allocations.size() == 0)
		return;
	fprintf(stderr, "[libui] leaked allocations:\n");
	for (i = allocations.begin(); i != allocations.end(); i++)
		fprintf(stderr, "[libui] %p %s\n", *i, *TYPE(*i));
	complain("either you left something around or there's a bug in libui");
}

void *uiAlloc(size_t size, const char *type)
{
	void *out;

	out = malloc(EXTRA + size);
	if (out == NULL) {
		fprintf(stderr, "memory exhausted in uiAlloc()\n");
		abort();
	}
	memset(DATA(out), 0, size);
	*SIZE(out) = size;
	*TYPE(out) = type;
	allocations.insert(out);
	return DATA(out);
}

void *uiRealloc(void *p, size_t new, const char *type)
{
	void *out;
	size_t *s;

	if (p == NULL)
		return uiAlloc(new, type);
	p = BASE(p);
	out = realloc(p, EXTRA + new);
	if (out == NULL) {
		fprintf(stderr, "memory exhausted in uiRealloc()\n");
		abort();
	}
	s = SIZE(out);
	if (new <= *s)
		memset(((uint8_t *) DATA(out)) + *s, 0, new - *s);
	*s = new;
	// TODO check this
	allocations.erase(p);
	allocations.insert(out);
	return DATA(out);
}

void uiFree(void *p)
{
	if (p == NULL)
		complain("attempt to uiFree(NULL); there's a bug somewhere");
	p = BASE(p);
	free(p);
	// TODO check this
	allocations.erase(p);
}
