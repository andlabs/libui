// 7 april 2015
#include <string.h>
#include "uipriv_unix.h"

static GPtrArray *allocations;

void initAlloc(void)
{
	allocations = g_ptr_array_new();
}

static void uninitComplain(gpointer ptr, gpointer data)
{
	fprintf(stderr, "[libui] %p\n", ptr);
}

// TODO bring back the type names for this
void uninitAlloc(void)
{
	if (allocations->len == 0) {
		g_ptr_array_free(allocations, TRUE);
		return;
	}
	fprintf(stderr, "[libui] leaked allocations:\n");
	g_ptr_array_foreach(allocations, uninitComplain, NULL);
	complain("either you left something around or there's a bug in libui");
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
	g_ptr_array_add(allocations, out);
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
	if (g_ptr_array_remove(allocations, p) == FALSE)
		complain("%p not found in allocations array in uiRealloc()", p);
	g_ptr_array_add(allocations, out);
	return DATA(out);
}

void uiFree(void *p)
{
	if (p == NULL)
		complain("attempt to uiFree(NULL); there's a bug somewhere");
	g_free(BASE(p));
	if (g_ptr_array_remove(allocations, p) == FALSE)
		complain("%p not found in allocations array in uiFree()", p);
}
