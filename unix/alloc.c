// 7 april 2015
#include <string.h>
#include "uipriv_unix.h"

static GHashTable *allocsizes;			// map[*void]*size_t

void initAlloc(void)
{
	allocsizes = g_hash_table_new(g_direct_hash, g_direct_equal);
}

void *uiAlloc(size_t size)
{
	void *out;
	size_t *s;

	out = g_malloc0(size);
	if (g_hash_table_lookup(allocsizes, out) != NULL)
		complain("already have a size for %p in uiAlloc()", out);
	s = g_new0(size_t, 1);
	*s = size;
	g_hash_table_insert(allocsizes, out, s);
	return out;
}

void *uiRealloc(void *p, size_t new)
{
	void *out;
	size_t *s;

	if (p == NULL)
		return uiAlloc(new);
	out = g_realloc(p, new);
	s = (size_t *) g_hash_table_lookup(allocsizes, p);
	if (s == NULL)
		complain("no size found for %p in uiRealloc()", p);
	if (new <= *s)
		memset(((uint8_t *) out) + *s, 0, new - *s);
	*s = new;
	g_hash_table_remove(allocsizes, p);
	g_hash_table_insert(allocsizes, out, s);
	return out;
}

void uiFree(void *p)
{
	size_t *s;

	g_free(p);
	s = (size_t *) g_hash_table_lookup(allocsizes, p);
	if (s == NULL)
		complain("no size found for %p in uiFree()", p);
	g_hash_table_remove(allocsizes, p);
	g_free(s);
}
