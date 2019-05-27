// 19 april 2019

#ifdef __cplusplus
extern "C" {
#endif

// TODO figure out why this is needed despite what https://docs.microsoft.com/en-us/previous-versions/visualstudio/visual-studio-2013/b0084kay(v=vs.120) says
#ifdef _MSC_VER
#define uiprivFunc __FUNCTION__
#else
#define uiprivFunc __func__
#endif

// TODO WHY IS THIS NEEDED?!?!?!?!!?!??!Q https://stackoverflow.com/questions/15610053/correct-printf-format-specifier-for-size-t-zu-or-iu SAYS THAT VS2013 DOES SUPPORT %zu
#ifdef _MSC_VER
#define uiprivSizetPrintf "Iu"
#else
#define uiprivSizetPrintf "zu"
#endif

// init.c
extern const char **uiprivSysInitErrors(void);
extern int uiprivSysInit(void *options, uiInitError *err);
extern int uiprivInitReturnError(uiInitError *err, const char *msg);
extern int uiprivInitReturnErrorf(uiInitError *err, const char *msg, ...);

// alloc.c
extern void *uiprivAlloc(size_t n, const char *what);
extern void *uiprivRealloc(void *p, size_t nOld, size_t nNew, const char *what);
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
	memset(&arr, 0, sizeof (uiprivArray));
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
	uiprivProgrammerErrorIndexOutOfRange,	// arguments: int badIndex, uiprivFunc
	uiprivProgrammerErrorNullPointer,			// arguments: const char *paramDesc, uiprivFunc
	uiprivProgrammerErrorIntIDNotFound,		// arguments: const char *idDesc, int badID, uiprivFunc
	// TODO type mismatch
	uiprivProgrammerErrorBadSenderForEvent,	// arguments: const char *senderDesc, const char *eventDesc, uiprivFunc
	uiprivProgrammerErrorChangingEventDuringFire,		// arguments: uiprivFunc
	uiprivProgrammerErrorRecursiveEventFire,	// no arguments
	uiprivNumProgrammerErrors,
};
extern void uiprivProgrammerError(unsigned int which, ...);
extern void uiprivReportError(const char *prefix, const char *msg, const char *suffix, bool internal);

#ifdef __cplusplus
}
#endif
