// 19 april 2019
#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ui.h"
#ifdef uiprivOSHeader
#include uiprivOSHeader
#endif

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
// TODO AND WHY IS MINGW AFFECTED?!?!?!?!
#ifdef _WIN32
#define uiprivSizetPrintf "Iu"
#else
#define uiprivSizetPrintf "zu"
#endif

// main.c
extern bool uiprivSysInit(void *options, uiInitError *err);
extern bool uiprivInitReturnErrorf(uiInitError *err, const char *msg, ...);
extern void uiprivSysQueueMain(void (*f)(void *data), void *data);
extern bool uiprivCheckInitializedAndThreadImpl(const char *func);
#define uiprivCheckInitializedAndThread() uiprivCheckInitializedAndThreadImpl(uiprivFunc)
extern bool uiprivSysCheckThread(void);

// alloc.c
#define sharedbitsPrefix uipriv
// TODO determine if we need the ../ or not, and if not, figure out if we should use it everywhere (including ui.h) or not
#include "../sharedbits/alloc_header.h"
#include "../sharedbits/array_header.h"
#define uiprivArrayStaticInit(T, grow, whatstr) { NULL, 0, 0, sizeof (T), grow, whatstr }
#define uiprivArrayInit(arr, T, nGrow, what) uiprivArrayInitFull(&(arr), sizeof (T), nGrow, what)
#define uiprivArrayFree(arr) uiprivArrayFreeFull(&(arr))
#define uiprivArrayAt(arr, T, n) (((T *) (arr.buf)) + (n))
#include "../sharedbits/strsafe_header.h"
#undef sharedbitsPrefix

// errors.c
extern void uiprivInternalError(const char *fmt, ...);
enum {
	uiprivProgrammerErrorNotInitialized,		// arguments: uiprivFunc
	uiprivProgrammerErrorWrongThread,		// arguments: uiprivFunc
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
