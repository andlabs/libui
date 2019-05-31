// 30 may 2019
// requires: alloc_header.h

#ifndef sharedbitsPrefix
#error you must define sharedbitsPrefix before including this
#endif
#define sharedbitsPrefixMakeName(x, y) x ## y
#define sharedbitsPrefixName(Name) sharedbitsPrefixMakeName(sharedbitsPrefix, Name)

extern void sharedbitsPrefixName(InternalError)(const char *fmt, ...);

void *sharedbitsPrefixName(Alloc)(size_t n, const char *what)
{
	void *p;

	p = malloc(n);
	if (p == NULL)
		sharedbitsPrefixName(InternalError)("memory exhausted allocating %s", what);
	memset(p, 0, n);
	return p;
}

void *sharedbitsPrefixName(Realloc)(void *p, size_t nOld, size_t nNew, const char *what)
{
	p = realloc(p, nNew);
	if (p == NULL)
		sharedbitsPrefixName(InternalError)("memory exhausted reallocating %s", what);
	if (nNew > nOld)
		memset(((uint8_t *) p) + nOld, 0, nNew - nOld);
	return p;
}

void sharedbitsPrefixName(Free)(void *p)
{
	free(p);
}

#undef sharedbitsPrefixName
#undef sharedbitsPrefixMakeName
