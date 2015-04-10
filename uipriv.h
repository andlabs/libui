// 6 april 2015
#include <stdlib.h>
#include "ui.h"

extern uiInitOptions options;

extern void *uiAlloc(size_t, const char *);
#define uiNew(T) ((T *) uiAlloc(sizeof (T), #T ))
extern void *uiRealloc(void *, size_t, const char *);
extern void uiFree(void *);

extern void updateParent(uintptr_t);
