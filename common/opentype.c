// 25 february 2018
#include "uipriv.h"
#include "attrstr.h"

struct feature {
	char a;
	char b;
	char c;
	char d;
	uint32_t value;
};

struct uiOpenTypeFeatures {
	struct feature *data;
	size_t len;
	size_t cap;
};

#define bytecount(n) ((n) * sizeof (struct feature))

uiOpenTypeFeatures *uiNewOpenTypeFeatures(void)
{
	uiOpenTypeFeatures *otf;

	otf = uiprivNew(uiOpenTypeFeatures);
	otf->cap = 16;
	otf->data = (struct feature *) uiprivAlloc(bytecount(otf->cap), "struct feature[]");
	otf->len = 0;
	return otf;
}

void uiFreeOpenTypeFeatures(uiOpenTypeFeatures *otf)
{
	uiprivFree(otf->data);
	uiprivFree(otf);
}

// uiOpenTypeFeaturesClone() makes a copy of otf and returns it.
// Changing one will not affect the other.
_UI_EXTERN uiOpenTypeFeatures *uiOpenTypeFeaturesClone(const uiOpenTypeFeatures *otf)
{
	uiOpenTypeFeatures *ret;

	ret = uiprivNew(uiOpenTypeFeatures);
	ret->len = otf->len;
	ret->cap = otf->cap;
	ret->data = (struct feature *) uiprivAlloc(bytecount(ret->cap), "struct feature[]");
	memset(ret->data, 0, bytecount(ret->cap));
	memmove(ret->data, otf->data, bytecount(ret->len));
	return ret;
}

#define intdiff(a, b) (((int) (a)) - ((int) (b)))

static int featurecmp(const void *a, const void *b)
{
	const struct feature *f = (const struct feature *) a;
	const struct feature *g = (const struct feature *) b;

	if (f->a != g->a)
		return intdiff(f->a, g->a);
	if (f->b != g->b)
		return intdiff(f->b, g->b);
	if (f->c != g->c)
		return intdiff(f->c, g->c);
	return intdiff(f->d, g->d);
}

void uiOpenTypeFeaturesAdd(uiOpenTypeFeatures *otf, char a, char b, char c, char d, uint32_t value)
{
	struct feature *f;

	// replace existing value if any
	f = (struct feature *) bsearch(otf->data, otf->len, sizeof (struct feature), featurecmp);
	if (f != NULL) {
		f->value = value;
		return;
	}

	// if we got here, the tag is new
	if (otf->len == otf->cap) {
		otf->cap *= 2;
		otf->data = (struct feature *) uiprivRealloc(otf->data, bytecount(otf->cap), "struct feature[]");
	}
	f = otf->data + otf->len;
	f->a = a;
	f->b = b;
	f->c = c;
	f->d = d;
	f->value = value;
	// LONGTERM qsort here is overkill
	otf->len++;
	qsort(otf->data, otf->len, sizeof (struct feature), featurecmp);
}

_UI_EXTERN void uiOpenTypeFeaturesRemove(uiOpenTypeFeatures *otf, char a, char b, char c, char d)
{
	struct feature *f;
	ptrdiff_t index;
	size_t count;

	f = (struct feature *) bsearch(otf->data, otf->len, sizeof (struct feature), featurecmp);
	if (f == NULL)
		return;

	index = f - otf->data;
	count = otf->len - index - 1;
	memmove(f + 1, f, bytecount(count));
	otf->len--;
}

int uiOpenTypeFeaturesGet(const uiOpenTypeFeatures *otf, char a, char b, char c, char d, uint32_t *value)
{
	const struct feature *f;

	f = (const struct feature *) bsearch(otf->data, otf->len, sizeof (struct feature), featurecmp);
	if (f == NULL)
		return 0;
	*value = f->value;
	return 1;
}

void uiOpenTypeFeaturesForEach(const uiOpenTypeFeatures *otf, uiOpenTypeFeaturesForEachFunc f, void *data)
{
	size_t n;
	const struct feature *p;
	uiForEach ret;

	p = otf->data;
	for (n = 0; n < otf->len; n++) {
		ret = (*f)(cur->a, cur->b, cur->c, cur->d,
			cur->value, data);
		// TODO for all: require exact match?
		if (ret == uiForEachStop)
			return;
		p++;
	}
}

int uiprivOpenTypeFeaturesEqual(const uiOpenTypeFeatures *a, const uiOpenTypeFeatures *b)
{
	if (a == b)
		return 1;
	if (a->len != b->len)
		return 0;
	return memcmp(a->data, b->data, bytecount(a->len)) == 0;
}
