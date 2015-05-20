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
#include "../out/ui.h"
#include "../ui_windows.h"
#include "../uipriv.h"

// ui internal window messages
enum {
	// redirected WM_COMMAND and WM_NOTIFY
	msgCOMMAND = WM_APP + 0x40,		// start offset just to be safe
	msgNOTIFY,
	msgHSCROLL,
	msgHasTabStops,
	msgConsoleEndSession,
};

// init.c
extern HINSTANCE hInstance;
extern int nCmdShow;
extern HFONT hMessageFont;
extern HBRUSH hollowBrush;

// util.c
extern int windowClassOf(HWND, ...);
extern void mapWindowRect(HWND, HWND, RECT *);
extern DWORD getStyle(HWND);
extern void setStyle(HWND, DWORD);
extern DWORD getExStyle(HWND);
extern void setExStyle(HWND, DWORD);

// text.c
extern WCHAR *toUTF16(const char *);
extern char *toUTF8(const WCHAR *);
extern WCHAR *windowText(HWND);





/////////////////////////////////

// resize.c
extern void initResizes(void);
extern void uninitResizes(void);
extern void queueResize(uiControl *);
extern void doResizes(void);
extern void moveWindow(HWND, intmax_t, intmax_t, intmax_t, intmax_t, uiSizing *);
extern void moveAndReorderWindow(HWND, HWND, intmax_t, intmax_t, intmax_t, intmax_t);

// utilwindow.c
extern HWND utilWindow;
extern const char *initUtilWindow(HICON, HCURSOR);
extern void uninitUtilWindow(void);

// parent.c
extern BOOL handleParentMessages(HWND, UINT, WPARAM, LPARAM, LRESULT *);

// debug.c
extern HRESULT logLastError(const char *);
extern HRESULT logHRESULT(const char *, HRESULT);
extern HRESULT logMemoryExhausted(const char *);

// window.c
extern ATOM registerWindowClass(HICON, HCURSOR);
extern void unregisterWindowClass(void);

// container.c
#define containerClass L"libui_uiContainerClass"
extern ATOM initContainer(HICON, HCURSOR);
extern void uninitContainer(void);

// menu.c
extern HMENU makeMenubar(void);
extern const uiMenuItem *menuIDToItem(UINT_PTR);
extern void runMenuEvent(WORD, uiWindow *);
extern void freeMenubar(HMENU);
extern void uninitMenus(void);

// alloc.c
extern int initAlloc(void);
extern void uninitAlloc(void);

// tab.c
extern void tabEnterTabNavigation(HWND);
extern void tabLeaveTabNavigation(HWND);
