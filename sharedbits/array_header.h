// 30 may 2019
// requires: alloc_header.h

#include "start.h"

typedef struct sharedbitsPrefixName(Array) sharedbitsPrefixName(Array);

struct sharedbitsPrefixName(Array) {
	void *buf;
	size_t len;
	size_t cap;
	size_t elemsize;
	size_t nGrow;
	const char *what;
};

extern void sharedbitsPrefixName(ArrayInitFull)(sharedbitsPrefixName(Array) *arr, size_t elemsize, size_t nGrow, const char *what);
extern void sharedbitsPrefixName(ArrayFreeFull)(sharedbitsPrefixName(Array) *arr);
extern void *sharedbitsPrefixName(ArrayAppend)(sharedbitsPrefixName(Array) *arr, size_t n);
extern void *sharedbitsPrefixName(ArrayInsertAt)(sharedbitsPrefixName(Array) *arr, size_t pos, size_t n);
extern void sharedbitsPrefixName(ArrayDelete)(sharedbitsPrefixName(Array) *arr, size_t pos, size_t n);
extern void sharedbitsPrefixName(ArrayDeleteItem)(sharedbitsPrefixName(Array) *arr, void *p, size_t n);
extern void *sharedbitsPrefixName(ArrayBsearch)(const sharedbitsPrefixName(Array) *arr, const void *key, int (*compare)(const void *, const void *));
extern void sharedbitsPrefixName(ArrayQsort)(sharedbitsPrefixName(Array) *arr, int (*compare)(const void *, const void *));

/*
you may also want to define the following:

#define sharedbitsPrefixName(StaticInit)(T, nGrow, what) { NULL, 0, 0, sizeof (T), nGrow, what }
#define sharedbitsPrefixName(Init)(arr, T, nGrow, what) sharedbitsPrefixName(InitFull)(&(arr), sizeof (T), nGrow, what)
#define sharedbitsPrefixName(ArrayFree)(arr) sharedbitsPrefixName(ArrayFreeFull)(&(arr))
#define sharedbitsPrefixName(ArrayAt)(arr, T, n) (((T *) (arr.buf)) + (n))
*/

#include "end.h"
