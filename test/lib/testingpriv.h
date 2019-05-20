// 19 may 2019

extern void testingprivInternalError(const char *fmt, ...);

extern void *testingprivAlloc(size_t n, const char *what);
#define testingprivNew(T) ((T *) testingprivAlloc(sizeof (T), #T))
#define testingprivNewArray(T, n) ((T *) testingprivAlloc(n * sizeof (T), #T "[]"))
extern void *testingprivRealloc(void *p, size_t old, size_t new, const char *what);
#define testingprivResizeArray(x, T, old, new) ((T *) testingprivRealloc(x, old * sizeof (T), new * sizeof (T), #T "[]"))
extern void testingprivFree(void *p);

typedef struct testingprivArray testingprivArray;
struct testingprivArray {
	void *buf;
	size_t len;
	size_t cap;
	size_t elemsize;
	size_t nGrow;
	const char *what;
};
#define testingprivArrayStaticInit(T, grow, whatstr) { NULL, 0, 0, sizeof (T), grow, whatstr }
#define testingprivArrayInit(arr, T, grow, whatstr) \
	memset(&(arr), 0, sizeof (testingprivArray)); \
	arr.elemsize = sizeof (T); \
	arr.nGrow = grow; \
	arr.what = whatstr;
#define testingprivArrayFree(arr) \
	testingprivFree(arr.buf); \
	memset(&arr, 0, sizeof (testingprivArray));
#define testingprivArrayAt(arr, T, n) (((T *) (arr.buf)) + (n))
extern void *testingprivArrayAppend(testingprivArray *arr, size_t n);
extern void *testingprivArrayInsertAt(testingprivArray *arr, size_t pos, size_t n);
extern void testingprivArrayDelete(testingprivArray *arr, size_t pos, size_t n);
extern void testingprivArrayDeleteItem(testingprivArray *arr, void *p, size_t n);
extern void *testingprivArrayBsearch(const testingprivArray *arr, const void *key, int (*compare)(const void *, const void *));
extern void testingprivArrayQsort(testingprivArray *arr, int (*compare)(const void *, const void *));

extern int testingprivVsnprintf(char *s, size_t n, const char *format, va_list ap);
extern int testingprivSnprintf(char *s, size_t n, const char *format, ...);
extern char *testingprivStrdup(const char *s);
extern char *testingprivVsmprintf(const char *fmt, va_list ap);
extern char *testingprivSmprintf(const char *fmt, ...);

// a testingprivOutbuf of NULL writes directly to stdout
typedef struct testingprivOutbuf testingprivOutbuf;
extern testingprivOutbuf *testingprivNewOutbuf(void);
extern void testingprivOutbufFree(testingprivOutbuf *o);
extern void testingprivOutbufVprintf(testingprivOutbuf *o, const char *fmt, va_list ap);
extern void testingprivOutbufPrintf(testingprivOutbuf *o, const char *fmt, ...);
extern void testingprivOutbufAppendOutbuf(testingprivOutbuf *o, testingprivOutbuf *src, int indent);
