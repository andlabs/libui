// 6 april 2015

#ifdef __cplusplus
extern "C" {
#endif

#include <stdarg.h>
#include <string.h>
#include "controlsigs.h"
#include "utf.h"

extern uiInitOptions options;

extern void *uiAlloc(size_t, const char *);
#define uiNew(T) ((T *) uiAlloc(sizeof (T), #T))
extern void *uiRealloc(void *, size_t, const char *);
extern void uiFree(void *);

// ugh, this was only introduced in MSVC 2015...
#ifdef _MSC_VER
#define __func__ __FUNCTION__
#endif
extern void realbug(const char *file, const char *line, const char *func, const char *prefix, const char *format, va_list ap);
#define _ns2(s) #s
#define _ns(s) _ns2(s)
extern void _implbug(const char *file, const char *line, const char *func, const char *format, ...);
#define implbug(...) _implbug(__FILE__, _ns(__LINE__), __func__, __VA_ARGS__)
extern void _userbug(const char *file, const char *line, const char *func, const char *format, ...);
#define userbug(...) _userbug(__FILE__, _ns(__LINE__), __func__, __VA_ARGS__)

// control.c
extern uiControl *newControl(size_t size, uint32_t OSsig, uint32_t typesig, const char *typenamestr);

// shouldquit.c
extern int shouldQuit(void);

// areaevents.c
typedef struct clickCounter clickCounter;
// you should call Reset() to zero-initialize a new instance
// it doesn't matter that all the non-count fields are zero: the first click will fail the curButton test straightaway, so it'll return 1 and set the rest of the structure accordingly
struct clickCounter {
	int curButton;
	int rectX0;
	int rectY0;
	int rectX1;
	int rectY1;
	uintptr_t prevTime;
	int count;
};
int clickCounterClick(clickCounter *c, int button, int x, int y, uintptr_t time, uintptr_t maxTime, int32_t xdist, int32_t ydist);
extern void clickCounterReset(clickCounter *);
extern int fromScancode(uintptr_t, uiAreaKeyEvent *);

// matrix.c
extern void fallbackSkew(uiDrawMatrix *, double, double, double, double);
extern void scaleCenter(double, double, double *, double *);
extern void fallbackTransformSize(uiDrawMatrix *, double *, double *);

// for attrstr.c
struct graphemes {
	size_t len;
	size_t *pointsToGraphemes;
	size_t *graphemesToPoints;
};
extern int graphemesTakesUTF16(void);
extern struct graphemes *graphemes(void *s, size_t len);

// TODO split these into a separate header file?

// attrstr.c
extern const uint16_t *attrstrUTF16(uiAttributedString *s);
extern size_t attrstrUTF16Len(uiAttributedString *s);
extern size_t attrstrUTF8ToUTF16(uiAttributedString *s, size_t n);
extern size_t *attrstrCopyUTF16ToUTF8(uiAttributedString *s, size_t *n);

// attrlist.c
struct attrlist;
extern void attrlistInsertAttribute(struct attrlist *alist, uiAttribute type, uintptr_t val, size_t start, size_t end);
extern void attrlistInsertCharactersUnattributed(struct attrlist *alist, size_t start, size_t count);
extern void attrlistInsertCharactersExtendingAttributes(struct attrlist *alist, size_t start, size_t count);
extern void attrlistRemoveAttribute(struct attrlist *alist, uiAttribute type, size_t start, size_t end);
extern void attrlistRemoveAttributes(struct attrlist *alist, size_t start, size_t end);
extern void attrlistRemoveCharacters(struct attrlist *alist, size_t start, size_t end);
extern void attrlistForEach(struct attrlist *alist, uiAttributedString *s, uiAttributedStringForEachAttributeFunc f, void *data);
// TODO move these to the top like everythng else
extern struct attrlist *attrlistNew(void);
extern void attrlistFree(struct attrlist *alist);

#ifdef __cplusplus
}
#endif
