// 6 april 2015
#include <stdlib.h>

extern uiInitOptions options;

extern void *uiAlloc(size_t, const char *);
#define uiNew(T) ((T *) uiAlloc(sizeof (T), #T))
extern void *uiRealloc(void *, size_t, const char *);
extern void uiFree(void *);

extern void complain(const char *, ...);

// ptrarray.c
struct ptrArray {
	void **ptrs;
	uintmax_t len;
	uintmax_t cap;
};
struct ptrArray *newPtrArray(void);
void ptrArrayDestroy(struct ptrArray *);
void ptrArrayAppend(struct ptrArray *, void *);
void ptrArrayInsertAt(struct ptrArray *, uintmax_t, void *);
void ptrArrayDelete(struct ptrArray *, uintmax_t);
#define ptrArrayIndex(p, T, i) ((T) ((p)->ptrs[(i)]))

// shouldquit.c
int shouldQuit(void);

// control.c
extern void osSingleDestroy(void *);
extern uintptr_t osSingleHandle(void *);
extern void osSingleSetParent(void *, uiControl *, uiControl *);
extern void osSingleResize(void *, intmax_t, intmax_t, intmax_t, intmax_t, uiSizing *);
extern uiSizing *osSingleSizing(void *, uiControl *);
extern void osSingleShow(void *);
extern void osSingleHide(void *);
extern void osSingleEnable(void *);
extern void osSingleDisable(void *);
extern void makeControl(uiControl *, void *);
