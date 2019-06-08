// 8 june 2019
// requires alloc_header.h and any of the strsafe_* files

#include "start.h"

char *sharedbitsPrefixName(Strdup)(const char *s)
{
	char *t;
	size_t n;

	n = strlen(s);
	t = (char *) sharedbitsPrefixName(Alloc)((n + 1) * sizeof (char), "char[]");
	sharedbitsPrefixName(Strncpy)(t, s, n + 1);
	return t;
}

#include "end.h"
