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

// TODO {
// WHY IS THIS NEEDED?!?!?!?!!?!??!Q https://stackoverflow.com/questions/15610053/correct-printf-format-specifier-for-size-t-zu-or-iu SAYS THAT VS2013 DOES SUPPORT %zu
// AND WHY IS MINGW AFFECTED?!?!?!?!
// Oh and even better: the -Wno-pedantic-ms-printf stuff doesn't result in a warning about this either...
// }
#ifdef _WIN32
#define uiprivSizetPrintf "Iu"
#else
#define uiprivSizetPrintf "zu"
#endif

#include "../sharedbits/printfwarn_header.h"
#define uiprivPrintfFunc(decl, fmtpos, appos) sharedbitsPrintfFunc(decl, fmtpos, appos)

// main.c
extern bool uiprivSysInit(void *options, uiInitError *err);
uiprivPrintfFunc(
	extern bool uiprivInitReturnErrorf(uiInitError *err, const char *fmt, ...),
	2, 3);
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
uiprivPrintfFunc(
	extern void uiprivInternalError(const char *fmt, ...),
	1, 2);
// TODO turn each of these into function-like macros
#define uiprivProgrammerErrorNotInitialized "attempt to call %s() before uiInit()"		// arguments: uiprivFunc
#define uiprivProgrammerErrorWrongThread "attempt to call %s() on a thread other than the GUI thread"		// arguments: uiprivFunc
#define uiprivProgrammerErrorWrongStructSize "wrong size %" uiprivSizetPrintf " for %s"		// arguments: size_t badSize, const char *structName
#define uiprivProgrammerErrorIndexOutOfRange "index %d out of range in %s()"		// arguments: int badIndex, uiprivFunc
#define uiprivProgrammerErrorNullPointer "invalid null pointer for %s passed into %s()"			// arguments: const char *paramDesc, uiprivFunc
#define uiprivProgrammerErrorIntIDNotFound "%s identifier %d not found in %s()"		// arguments: const char *idDesc, int badID, uiprivFunc
// TODO type mismatch
#define uiprivProgrammerErrorBadSenderForEvent "attempt to use a %s sender with a %s event in %s()"			// arguments: const char *senderDesc, const char *eventDesc, uiprivFunc
#define uiprivProgrammerErrorChangingEventDuringFire "attempt to change a uiEvent with %s() while it is firing"				// arguments: uiprivFunc
#define uiprivProgrammerErrorRecursiveEventFire "attempt to fire a uiEvent while it is already being fired"		// no arguments
uiprivPrintfFunc(
	extern void uiprivProgrammerError(const char *fmt, ...),
	1, 2);
extern void uiprivReportError(const char *prefix, const char *msg, const char *suffix, bool internal);

#ifdef __cplusplus
}
#endif
