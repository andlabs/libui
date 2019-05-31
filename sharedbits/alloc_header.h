// 30 may 2019

#include "start.h"

extern void *sharedbitsPrefixName(Alloc)(size_t n, const char *what);
extern void *sharedbitsPrefixName(Realloc)(void *p, size_t nOld, size_t nNew, const char *what);
extern void sharedbitsPrefixName(Free)(void *p);

/*
you may also want to define the following:

#define sharedbitsPrefixName(New)(T) ((T *) sharedbitsPrefix ## Alloc(sizeof (T), #T))
#define sharedbitsPrefixName(NewArray)(T, n) ((T *) sharedbitsPrefix ## Alloc(n * sizeof (T), #T "[]"))
#define sharedbitsPrefixName(ResizeArray)(x, T, old, new) ((T *) sharedbitsPrefix ## Realloc(x, old * sizeof (T), new * sizeof (T), #T "[]"))
*/

#include "end.h"
