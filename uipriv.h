// 6 april 2015
#include <stdlib.h>

extern uiInitOptions options;

extern void *uiAlloc(size_t, const char *);
#define uiNew(T) ((T *) uiAlloc(sizeof (T), #T))
extern void *uiRealloc(void *, size_t, const char *);
extern void uiFree(void *);

extern void complain(const char *, ...);

extern uiBin *newBin(void);
extern int binHasOSParent(uiBin *);
extern void binSetOSParent(uiBin *, uintptr_t);
extern void binRemoveOSParent(uiBin *);
extern void binResizeRootAndUpdate(uiBin *, intmax_t, intmax_t, intmax_t, intmax_t);
extern void binTranslateMargins(uiBin *, intmax_t *, intmax_t *, intmax_t *, intmax_t *, uiSizing *);

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
