// 5 may 2015
#include <string.h>
#include "ui.h"
#include "uipriv.h"

struct ptrArray *newPtrArray(void)
{
	return uiNew(struct ptrArray);
}

void ptrArrayDestroy(struct ptrArray *p)
{
	if (p->len != 0)
		complain("attempt to destroy ptrarray %p while it still has pointers inside", p);
	if (p->ptrs != NULL)			// array was created but nothing was ever put inside
		uiFree(p->ptrs);
	uiFree(p);
}

#define grow 32

void ptrArrayAppend(struct ptrArray *p, void *d)
{
	ptrArrayInsertAt(p, p->len, d);
}

void ptrArrayInsertAt(struct ptrArray *p, uintmax_t i, void *d)
{
	if (i > p->len)
		complain("index out of range in ptrArrayInsertAt()");
	if (p->len >= p->cap) {
		p->cap += grow;
		p->ptrs = (void **) uiRealloc(p->ptrs, p->cap * sizeof (void *), "void *[]");
	}
	// thanks to ValleyBell
	memmove(&(p->ptrs[i + 1]), &(p->ptrs[i]), (p->len - i) * sizeof (void *));
	p->ptrs[i] = d;
	p->len++;
}

void ptrArrayDelete(struct ptrArray *p, uintmax_t i)
{
	if (i >= p->len)
		complain("index out of range in ptrArrayRemove()");
	// thanks to ValleyBell
	memmove(&(p->ptrs[i]), &(p->ptrs[i + 1]), (p->len - i - 1) * sizeof (void *));
	p->ptrs[p->len - 1] = NULL;
	p->len--;
}
