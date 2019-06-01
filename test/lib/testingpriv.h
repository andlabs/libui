// 19 may 2019

extern void testingprivInternalError(const char *fmt, ...);

#define sharedbitsPrefix testingpriv

#include "../../sharedbits/alloc_header.h"
#define testingprivNew(T) ((T *) testingprivAlloc(sizeof (T), #T))
#define testingprivNewArray(T, n) ((T *) testingprivAlloc(n * sizeof (T), #T "[]"))
#define testingprivResizeArray(x, T, old, new) ((T *) testingprivRealloc(x, old * sizeof (T), new * sizeof (T), #T "[]"))

#include "../../sharedbits/array_header.h"
#define testingprivArrayStaticInit(T, nGrow, what) { NULL, 0, 0, sizeof (T), nGrow, what }
#define testingprivArrayInit(arr, T, nGrow, what) testingprivArrayInitFull(&(arr), sizeof (T), nGrow, what)
#define testingprivArrayFree(arr) testingprivArrayFreeFull(&(arr))
#define testingprivArrayAt(arr, T, n) (((T *) (arr.buf)) + (n))

#undef sharedbitsPrefix

extern int testingprivVsnprintf(char *s, size_t n, const char *fmt, va_list ap);
extern int testingprivSnprintf(char *s, size_t n, const char *fmt, ...);
extern char *testingprivStrdup(const char *s);
extern char *testingprivVsmprintf(const char *fmt, va_list ap);
extern char *testingprivSmprintf(const char *fmt, ...);

// a testingprivOutbuf of NULL writes directly to stdout
typedef struct testingprivOutbuf testingprivOutbuf;
extern testingprivOutbuf *testingprivNewOutbuf(void);
extern void testingprivOutbufFree(testingprivOutbuf *o);
extern void testingprivOutbufVprintf(testingprivOutbuf *o, const char *fmt, va_list ap);
extern void testingprivOutbufVprintfIndented(testingprivOutbuf *o, const char *fmt, va_list ap);
extern void testingprivOutbufPrintf(testingprivOutbuf *o, const char *fmt, ...);
extern void testingprivOutbufAppendOutbuf(testingprivOutbuf *o, testingprivOutbuf *src);
extern const char *testingprivOutbufString(testingprivOutbuf *o);
