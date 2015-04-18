// 6 january 2015
#define UNICODE
#define _UNICODE
#define STRICT
#define STRICT_TYPED_ITEMIDS
#define CINTERFACE
#define COBJMACROS
// see https://github.com/golang/go/issues/9916#issuecomment-74812211
#define INITGUID
// get Windows version right; right now Windows XP
#define WINVER 0x0501
#define _WIN32_WINNT 0x0501
#define _WIN32_WINDOWS 0x0501		/* according to Microsoft's winperf.h */
#define _WIN32_IE 0x0600			/* according to Microsoft's sdkddkver.h */
#define NTDDI_VERSION 0x05010000	/* according to Microsoft's sdkddkver.h */
#include <windows.h>
#include <commctrl.h>
#include <stdint.h>
#include <uxtheme.h>
#include <string.h>
#include <wchar.h>
#include <windowsx.h>
#include <vsstyle.h>
#include <vssym32.h>
#include <stdarg.h>
#include <oleacc.h>
#include <stdio.h>
#include "../uipriv.h"
#include "../ui_windows.h"

// ui internal window messages
enum {
	// redirected WM_COMMAND and WM_NOTIFY
	msgCOMMAND = WM_APP + 0x40,		// start offset just to be safe
	msgNOTIFY,
	msgUpdateChild,		// fake because Windows seems to SWP_NOSIZE MoveWindow()s and SetWindowPos()s that don't change the window size (even if SWP_NOSIZE isn't specified)
	msgCanDestroyNow,
};

#define HWND(c) ((HWND) uiControlHandle(uiControl(c)))
#define uiParentHWND(p) ((HWND) uiParentHandle(p))

// debug_windows.c
extern HRESULT logLastError(const char *);
extern HRESULT logHRESULT(const char *, HRESULT);
extern HRESULT logMemoryExhausted(const char *);

// init_windows.c
extern HINSTANCE hInstance;
extern int nCmdShow;
extern HFONT hMessageFont;
extern HBRUSH hollowBrush;

// util_windows.c
extern int windowClassOf(HWND, ...);

// text_windows.c
extern WCHAR *toUTF16(const char *);
extern char *toUTF8(const WCHAR *);
extern WCHAR *windowText(HWND);

// comctl32_windows.c
extern BOOL (*WINAPI fv_SetWindowSubclass)(HWND, SUBCLASSPROC, UINT_PTR, DWORD_PTR);
extern BOOL (*WINAPI fv_RemoveWindowSubclass)(HWND, SUBCLASSPROC, UINT_PTR);
extern LRESULT (*WINAPI fv_DefSubclassProc)(HWND, UINT, WPARAM, LPARAM);
extern const char *initCommonControls(void);

// window_windows.c
extern ATOM registerWindowClass(HICON, HCURSOR);

// parent_windows.c
extern HWND initialParent;
extern const char *initParent(HICON, HCURSOR);
