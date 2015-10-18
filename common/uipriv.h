// 6 april 2015
#include <stdlib.h>

#define uthash_fatal(msg) complain("uthash failed: %s", (msg))
#define uthash_malloc(sz) uiAlloc((sz), "(uthash internal)")
#define uthash_free(ptr,sz) uiFree((ptr))
#include "uthash/uthash.h"
#include "uthash/utarray.h"

extern uiInitOptions options;

extern void *uiAlloc(size_t, const char *);
#define uiNew(T) ((T *) uiAlloc(sizeof (T), #T))
extern void *uiRealloc(void *, size_t, const char *);
extern void uiFree(void *);

extern void complain(const char *, ...);

extern int isToplevel(uiControl *);
extern uiControl *toplevelOwning(uiControl *);
extern int controlSelfVisible(uiControl *);
extern void controlUpdateState(uiControl *);

extern void osCommitEnable(uiControl *);
extern void osCommitDisable(uiControl *);

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
extern int shouldQuit(void);

// types.c
extern void uninitTypes(void);
extern uiTyped *newTyped(uintmax_t type);

// areaevents.c
typedef struct clickCounter clickCounter;
// you should call Reset() to zero-initialize a new instance
// it doesn't matter that all the non-count fields are zero: the first click will fail the curButton test straightaway, so it'll return 1 and set the rest of the structure accordingly
struct clickCounter {
	uintmax_t curButton;
	intmax_t rectX0;
	intmax_t rectY0;
	intmax_t rectX1;
	intmax_t rectY1;
	uintptr_t prevTime;
	uintmax_t count;
};
extern uintmax_t clickCounterClick(clickCounter *, uintmax_t, intmax_t, intmax_t, uintptr_t, uintptr_t, intmax_t, intmax_t);
extern void clickCounterReset(clickCounter *);
extern int fromScancode(uintptr_t, uiAreaKeyEvent *);

// matrix.c
extern void setIdentity(uiDrawMatrix *);
extern void fallbackSkew(uiDrawMatrix *, double, double, double, double);
extern void fallbackTranslate(uiDrawMatrix *, double, double);
extern void scaleCenter(double, double, double *, double *);
extern void fallbackScale(uiDrawMatrix *, double, double, double, double);
extern void fallbackMultiply(uiDrawMatrix *, uiDrawMatrix *);
extern void fallbackTransformPoint(uiDrawMatrix *, double *, double *);
extern void fallbackTransformSize(uiDrawMatrix *, double *, double *);
