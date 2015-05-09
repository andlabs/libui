// 6 april 2015
#include <stdlib.h>

extern uiInitOptions options;

extern void *uiAlloc(size_t, const char *);
#define uiNew(T) ((T *) uiAlloc(sizeof (T), #T))
extern void *uiRealloc(void *, size_t, const char *);
extern void uiFree(void *);

extern void complain(const char *, ...);

extern uiContainer *newBin(void);
extern void binSetMainControl(uiContainer *, uiControl *);
extern void binSetMargins(uiContainer *, intmax_t, intmax_t, intmax_t, intmax_t);
extern void binSetParent(uiContainer *, uintptr_t);

// array.c
struct ptrArray {
	void **ptrs;
	uintmax_t len;
	uintmax_t cap;
};
struct ptrArray *newPtrArray(void);
void ptrArrayDestroy(struct ptrArray *);
void ptrArrayAppend(struct ptrArray *, void *);
void ptrArrayInsertBefore(struct ptrArray *, uintmax_t, void *);
void ptrArrayDelete(struct ptrArray *, uintmax_t);
#define ptrArrayIndex(p, T, i) ((T) ((p)->ptrs[(i)]))

// shouldquit.c
int shouldQuit(void);
