// 30 may 2019
// requires: array_header.h

#include "start.h"

void sharedbitsPrefixName(ArrayInitFull)(sharedbitsPrefixName(Array) *arr, size_t elemsize, size_t nGrow, const char *what)
{
	memset(arr, 0, sizeof (sharedbitsPrefixName(Array)));
	arr->elemsize = elemsize;
	arr->nGrow = nGrow;
	arr->what = what;
}

void sharedbitsPrefixName(ArrayFreeFull)(sharedbitsPrefixName(Array) *arr)
{
	sharedbitsPrefixName(Free)(arr->buf);
	memset(arr, 0, sizeof (sharedbitsPrefixName(Array)));
}

#include "end.h"
