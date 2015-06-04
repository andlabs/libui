// 6 january 2015
#include "winapi.h"
#include "../out/ui.h"
#include "../ui_windows.h"
#include "../uipriv.h"
#include "resources.h"

#ifndef __MINGW64_VERSION_MAJOR
#error At present, only MinGW-w64 (>= 4.0.0) is supported. Other toolchains will be supported in the future (but other variants of MinGW will not).
#endif
#if __MINGW64_VERSION_MAJOR < 4
#error MinGW-w64 version 4.0.0 or newer is required.
#endif

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
extern void doResizes(void);
extern void moveWindow(HWND, intmax_t, intmax_t, intmax_t, intmax_t, uiSizing *);
extern void setWindowInsertAfter(HWND, HWND);

// utilwindow.c
extern HWND utilWindow;
extern const char *initUtilWindow(HICON, HCURSOR);
extern void uninitUtilWindow(void);

// parent.c
extern void paintContainerBackground(HWND, HDC, RECT *);
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

// events.c
// TODO split the uiWindows ones to ui_windows.h
extern void uiWindowsRegisterWM_COMMANDHandler(HWND, BOOL (*)(uiControl *, HWND, WORD, LRESULT *), uiControl *);
extern void uiWindowsRegisterWM_NOTIFYHandler(HWND, BOOL (*)(uiControl *, HWND, NMHDR *, LRESULT *), uiControl *);
extern void uiWindowsRegisterWM_HSCROLLHandler(HWND, BOOL (*)(uiControl *, HWND, WORD, LRESULT *), uiControl *);
extern void uiWindowsUnregisterWM_COMMANDHandler(HWND);
extern void uiWindowsUnregisterWM_NOTIFYHandler(HWND);
extern void uiWindowsUnregisterWM_HSCROLLHandler(HWND);
extern BOOL runWM_COMMAND(WPARAM, LPARAM, LRESULT *);
extern BOOL runWM_NOTIFY(WPARAM, LPARAM, LRESULT *);
extern BOOL runWM_HSCROLL(WPARAM, LPARAM, LRESULT *);
extern void uiWindowsRegisterReceiveWM_WININICHANGE(HWND);
extern void uiWindowsUnregisterReceiveWM_WINICHANGE(HWND);
extern void issueWM_WININICHANGE(WPARAM, LPARAM);

// dialoghelper.c
extern void dialogHelperRegisterWindow(HWND);
extern void dialogHelperUnregisterWindow(HWND);
extern ATOM initDialogHelper(HICON, HCURSOR);
extern void uninitDialogHelper(void);
extern HWND beginDialogHelper(void);
extern void endDialogHelper(HWND);

// control.c
extern void setSingleHWNDFuncs(uiControl *);

// tabpage.c
extern uiControl *newTabPage(void);
extern int tabPageMargined(uiControl *);
extern void tabPageSetMargined(uiControl *, int);
extern void tabPageDestroyChild(uiControl *);
extern void tabPagePreserveChild(uiControl *);
extern void tabPageSetChild(uiControl *, uiControl *);
