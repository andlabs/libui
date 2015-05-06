// 5 may 2015
#include <string.h>
#include "ui.h"
#include "uipriv.h"

// TODO see if we can use memmove()

struct ptrArray newPtrArray(void)
{
	return uiNew(struct ptrArray);
}

void ptrArrayDestroy(struct ptrArray *p)
{
	if (p->len != 0)
		complain("attempt to destroy ptrarray %p while it still has pointers inside", p);
	uiFree(p);
}

#define grow 32

void ptrArrayAppend(struct ptrArray *p, void *d)
{
	p->len++;
	if (p->len >= p->cap) {
		p->cap += grow;
		p->ptrs = (void **) uiRealloc(p->ptrs, p->cap * sizeof (void *));
	}
	p->ptrs[p->len - 1] = d;
}

void ptrArrayInsertBefore(struct ptrArray *p, uintmax_t i, void *d)
{
	uintmax_t j;

	if (i >= p->len)
		complain("index out of range in ptrArrayInsertBefore()");
	// TODO does this need to be here
	p->len++;
	if (p->len >= p->cap) {
		p->cap += grow;
		p->ptrs = (void **) uiRealloc(p->ptrs, p->cap * sizeof (void *));
	}
	for (j = p->len - 1; j >= i; j--)
		p->ptrs[j + 1] = p->ptrs[j];
	p->ptrs[i] = d;
}

void ptrArrayDelete(struct ptrArray *p, uintmax_t i)
{
	uintmax_t j;

	if (i >= p->len)
		complain("index out of range in ptrArrayRemove()");
	for (j = i; j < p->len - 1; j++)
		p->ptrs[j] = p->ptrs[j + 1];
	p->ptrs[j] = NULL;
	p->len--;
}
