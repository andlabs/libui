// 6 april 2015
#include <stdlib.h>
#include "ui.h"

#include <stdio.h>
#define LOGALLOC(p, ty) fprintf(stderr, "%p %s ALLOC\n", p, #ty );
#define LOGFREE(p, ty) fprintf(stderr, "%p %s FREE\n", p, #ty );

typedef struct uiSize uiSize;
typedef struct uiSizing uiSizing;

struct uiSize {
	intmax_t width;
	intmax_t height;
};

// TODO handle destruction
struct uiControl {
	uintptr_t (*handle)(uiControl *);
	void (*setParent)(uiControl *, uintptr_t);
	uiSize (*preferredSize)(uiControl *, uiSizing *);
	void (*resize)(uiControl *, intmax_t, intmax_t, intmax_t, intmax_t, uiSizing *);
};

extern void *uiAlloc(size_t);
#define uiNew(T) ((T *) uiAlloc(sizeof (T)))
extern void *uiRealloc(void *, size_t);
extern void uiFree(void *);
