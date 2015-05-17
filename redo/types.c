// 17 may 2015
#include "out/ui.h"
#include "uipriv.h"

struct typeinfo {
	const char *name;
	uintmax_t parent;
};

static struct ptrArray *types = NULL;

uintmax_t uiRegisterType(const char *name, uintmax_t parent)
{
	struct typeinfo *ti;

	if (types == NULL) {
		types = newPtrArray();
		// reserve ID 0
		ptrArrayAppend(types, NULL);
	}
	ti = uiNew(struct typeinfo);
	ti->name = name;
	ti->parent = parent;
	ptrArrayAppend(types, ti);
	return types->len - 1;
}

void *uiIsA(void *p, uintmax_t id, int fail)
{
	uiTyped *t;
	struct typeinfo *ti;
	uintmax_t compareTo;

	if (id == 0 || id >= types->len)
		complain("invalid type ID given to uiIsA()");
	t = (uiTyped *) p;
	compareTo = t->Type;
	for (;;) {
		if (compareTo == 0 || compareTo >= types->len)
			complain("invalid type ID in uiIsA()", t);
		if (compareTo == id)
			return t;
		ti = ptrArrayIndex(types, struct typeinfo *, t->Type);
		if (ti->parent == 0)
			break;
		compareTo = ti->parent;
	}
	if (fail) {
		ti = ptrArrayIndex(types, struct typeInfo *, t->Type);
		complain("object %p not a %s in uiIsA()", t, ti->name);
	}
	return NULL;
}

// TODO free type info
