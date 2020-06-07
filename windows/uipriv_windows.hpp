// 21 april 2016
#include "winapi.hpp"
#include "winhresult.hpp"
#define uiprivOSHeader "../ui_windows.h"
#include "../common/uipriv.h"
//TODO#include "compilerver.hpp"

// main.cpp
extern HINSTANCE uipriv_hInstance;
extern int uipriv_nCmdShow;
//TODOextern HFONT hMessageFont;

// utilwin.cpp
enum {
	uiprivUtilWindowMsgQueueMain = WM_USER + 40,
};
extern HWND uiprivUtilWindow;
extern HRESULT uiprivInitUtilWindow(HICON hDefaultIcon, HCURSOR hDefaultCursor);

// window.cpp
extern HRESULT uiprivRegisterWindowClass(HICON hDefaultIcon, HCURSOR hDefaultCursor);

// utf16.cpp
extern WCHAR *uiprivToUTF16(const char *str);
extern char *uiprivToUTF8(const WCHAR *wstr);
