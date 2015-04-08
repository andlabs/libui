// 6 april 2015
#include <stdlib.h>
#include "ui.h"

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
// TODO use this in existing files
#define uiNew(T) ((T *) uiAlloc(sizeof (T)))
extern void *uiRealloc(void *, size_t);
extern void uiFree(void *);
