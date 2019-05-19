// 19 april 2019

#ifdef __cplusplus
extern "C" {
#endif

// init.c
extern const char **uiprivSysInitErrors(void);
extern int uiprivSysInit(void *options, uiInitError *err);
extern int uiprivInitReturnError(uiInitError *err, const char *msg);
extern int uiprivInitReturnErrorf(uiInitError *err, const char *msg, ...);

// alloc.c
extern void *uiprivAlloc(size_t n, const char *what);
extern void *uiprivRealloc(void *p, size_t old, size_t new, const char *what);
extern void uiprivFree(void *p);
typedef struct uiprivArray uiprivArray;
struct uiprivArray {
	void *buf;
	size_t len;
	size_t cap;
	size_t elemsize;
	size_t nGrow;
	const char *what;
};
#define uiprivArrayStaticInit(T, grow, whatstr) { NULL, 0, 0, sizeof (T), grow, whatstr }
#define uiprivArrayInit(arr, T, grow, whatstr) \
	memset(&(arr), 0, sizeof (uiprivArray)); \
	arr.elemsize = sizeof (T); \
	arr.nGrow = grow; \
	arr.what = whatstr;
#define uiprivArrayFree(arr) \
	uiprivFree(arr.buf); \
	memset(&arr, 0, sizeof (uiprivArray);
#define uiprivArrayAt(arr, T, n) (((T *) (arr.buf)) + (n))
extern void *uiprivArrayAppend(uiprivArray *arr, size_t n);
extern void *uiprivArrayInsertAt(uiprivArray *arr, size_t pos, size_t n);
extern void uiprivArrayDelete(uiprivArray *arr, size_t pos, size_t n);
extern void uiprivArrayDeleteItem(uiprivArray *arr, void *p, size_t n);
extern void *uiprivArrayBsearch(const uiprivArray *arr, const void *key, int (*compare)(const void *, const void *));
extern void uiprivArrayQsort(uiprivArray *arr, int (*compare)(const void *, const void *));

// errors.c
extern void uiprivInternalError(const char *fmt, ...);
enum {
	uiprivProgrammerErrorWrongStructSize,		// arguments: size_t badSize, const char *structName
	uiprivProgrammerErrorIndexOutOfRange,	// arguments: int badIndex, __func__
	uiprivProgrammerErrorNullPointer,			// arguments: const char *paramDesc, __func__
	uiprivProgrammerErrorIntIDNotFound,		// arguments: const char *idDesc, int badID, __func__
	// TODO type mismatch
	uiprivProgrammerErrorBadSenderForEvent,	// arguments: const char *senderDesc, const char *eventDesc, __func__
	uiprivProgrammerErrorChangingEventDuringFire,		// arguments: __func__
	uiprivProgrammerErrorRecursiveEventFire,	// no arguments
	uiprivNumProgrammerErrors,
};
extern void uiprivProgrammerError(unsigned int which, ...);
extern void uiprivReportError(const char *prefix, const char *msg, const char *suffix, bool internal);

#ifdef __cplusplus
}
#endif
