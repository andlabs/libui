// 19 april 2019
#include <errno.h>
#include <inttypes.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ui.h"
#ifdef uiprivOSHeader
#include uiprivOSHeader
#endif
#include "testhooks.h"

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
extern void uiprivSysMain(void);
extern void uiprivSysQuit(void);
extern void uiprivSysQueueMain(void (*f)(void *data), void *data);
extern bool uiprivCheckInitializedAndThreadImpl(const char *func);
#define uiprivCheckInitializedAndThread() uiprivCheckInitializedAndThreadImpl(uiprivFunc)
extern bool uiprivSysCheckThread(void);

// alloc.c
#define sharedbitsPrefix uipriv
// TODO determine if we need the ../ or not, and if not, figure out if we should use it everywhere (including ui.h) or not
#include "../sharedbits/alloc_header.h"
#define uiprivNew(T) ((T *) uiprivAlloc(sizeof (T), #T))
#include "../sharedbits/array_header.h"
#define uiprivArrayStaticInit(T, grow, whatstr) { NULL, 0, 0, sizeof (T), grow, whatstr }
#define uiprivArrayInit(arr, T, nGrow, what) uiprivArrayInitFull(&(arr), sizeof (T), nGrow, what)
#define uiprivArrayFree(arr) uiprivArrayFreeFull(&(arr))
#define uiprivArrayAt(arr, T, n) (((T *) (arr.buf)) + (n))
#include "../sharedbits/strsafe_header.h"
#include "../sharedbits/strdup_header.h"
#undef sharedbitsPrefix

// errors.c
uiprivPrintfFunc(
	extern void uiprivInternalError(const char *fmt, ...),
	1, 2);
uiprivPrintfFunc(
	extern void uiprivProgrammerError(const char *fmt, ...),
	1, 2);
#include "programmererrors.h"
extern void uiprivReportError(const char *prefix, const char *msg, const char *suffix, bool internal);

// controls.c
extern bool uiprivOSVtableValid(const char *name, const uiControlOSVtable *osVtable, const char *func);
extern uiControlOSVtable *uiprivCloneOSVtable(const uiControlOSVtable *osVtable);
extern uiControlOSVtable *uiprivControlOSVtable(uiControl *c);

// utf8.c
extern char *uiprivSanitizeUTF8(const char *str);
extern void uiprivFreeUTF8(char *sanitized);

// window.c
extern uint32_t uiprivSysWindowType(void);
extern uiWindow *uiprivSysNewWindow(void);
extern const char *uiprivSysWindowTitle(uiWindow *w);
extern void uiprivSysWindowSetTitle(uiWindow *w, const char *title);
extern uiControl *uiprivSysWindowChild(uiWindow *w);
extern void uiprivSysWindowSetChild(uiWindow *w, uiControl *child);

#ifdef __cplusplus
}
#endif
