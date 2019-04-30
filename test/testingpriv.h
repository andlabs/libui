// 29 april 2019

extern void testingprivInternalError(const char *fmt, ...);
extern void *testingprivMalloc(size_t n, const char *what);
#define testingprivNew(T) ((T *) testingprivMalloc(sizeof (T), #T))
#define testingprivNewArray(T, n) ((T *) testingprivMalloc(n * sizeof (T), #T "[" #n "]"))
extern void *testingprivRealloc(void *x, size_t n, const char *what);
#define testingprivResizeArray(x, T, n) ((T *) testingprivRealloc(x, n * sizeof (T), #T "[" #n "]"))
extern void testingprivFree(void *x);
