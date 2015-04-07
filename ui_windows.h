// 6 january 2015

#ifndef __UI_UI_WINDOWS_H__
#define __UI_UI_WINDOWS_H__

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
#include "uipriv.h"

// alloc_windows.c
extern void *uiAlloc(size_t);
// TODO use this in existing files
#define uiNew(T) ((T *) uiAlloc(sizeof (T)))
extern void *uiRealloc(void *, size_t);
extern void uiFree(void *);

// debug_windows.c
extern HRESULT logLastError(const char *);
extern HRESULT logHRESULT(const char *, HRESULT);
extern HRESULT logMemoryExhausted(const char *);

// init_windows.c
extern HINSTANCE hInstance;
extern int nCmdShow;
extern HFONT hMessageFont;

// util_windows.c
extern WCHAR *toUTF16(const char *);

// window_windows.c
extern ATOM registerWindowClass(HICON, HCURSOR);

// singlehandle_windows.c
typedef struct uiSingleHWNDControl uiSingleHWNDControl;
struct uiSingleHWNDControl {
	uiControl control;
	HWND hwnd;
	void (*voidEvent)(uiControl *, void *);
	void *voidEventData;
};
extern uiSingleHWNDControl *newSingleHWNDControl(DWORD, const WCHAR *, DWORD, HWND, HINSTANCE);

#endif
