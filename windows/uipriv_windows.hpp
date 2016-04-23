// 21 april 2016
#include "winapi.hpp"
#include "../ui.h"
#include "../ui_windows.h"
#include "../common/uipriv.h"
#include "resources.hpp"
#include "compilerver.hpp"

// ui internal window messages
enum {
	// redirected WM_COMMAND and WM_NOTIFY
	msgCOMMAND = WM_APP + 0x40,		// start offset just to be safe
	msgNOTIFY,
	msgHSCROLL,
	msgGetuiWindow,
	msgQueued,
	msgD2DScratchPaint,
};

// alloc.cpp
extern void initAlloc(void);
extern void uninitAlloc(void);

// events.cpp
extern BOOL runWM_COMMAND(WPARAM wParam, LPARAM lParam, LRESULT *lResult);
extern BOOL runWM_NOTIFY(WPARAM wParam, LPARAM lParam, LRESULT *lResult);
extern BOOL runWM_HSCROLL(WPARAM wParam, LPARAM lParam, LRESULT *lResult);
extern void issueWM_WININICHANGE(WPARAM wParam, LPARAM lParam);

// utf16.cpp
#define emptyUTF16() ((WCHAR *) uiAlloc(1 * sizeof (WCHAR), "WCHAR[]"))
#define emptyUTF8() ((char *) uiAlloc(1 * sizeof (char), "char[]"))
extern WCHAR *toUTF16(const char *str);
extern char *toUTF8(const WCHAR *wstr);
extern WCHAR *utf16dup(const WCHAR *orig);
extern WCHAR *strf(const WCHAR *format, ...);
extern WCHAR *vstrf(const WCHAR *format, va_list ap);
extern WCHAR *debugstrf(const WCHAR *format, ...);
extern WCHAR *debugvstrf(const WCHAR *format, va_list ap);
extern char *LFtoCRLF(const char *lfonly);
extern void CRLFtoLF(const char *s);

// debug.cpp
// see http://stackoverflow.com/questions/14421656/is-there-widely-available-wide-character-variant-of-file
// TODO turn line into a const WCHAR* this way
#define _ws(m) L ## m
#define debugargs const WCHAR *file, uintmax_t line, const WCHAR *func
extern HRESULT _logLastError(debugargs, const WCHAR *s);
#define logLastError(s) _logLastError(_ws(__FILE__), __LINE__, _ws(__func__), s)
extern HRESULT _logHRESULT(debugargs, const WCHAR *s, HRESULT hr);
#define logHRESULT(s, hr) _logHRESULT(_ws(__FILE__), __LINE__, _ws(__func__), s, hr)
extern void _implbug(debugargs, const WCHAR *format, ...);
#define implbug(...) _implbug(_ws(__FILE__), __LINE__, _ws(__func__), __VA_LIST__)

// winutil.cpp
extern int windowClassOf(HWND hwnd, ...);
extern void mapWindowRect(HWND from, HWND to, RECT *r);
extern DWORD getStyle(HWND hwnd);
extern void setStyle(HWND hwnd, DWORD style);
extern DWORD getExStyle(HWND hwnd);
extern void setExStyle(HWND hwnd, DWORD exstyle);
extern void clientSizeToWindowSize(HWND hwnd, intmax_t *width, intmax_t *height, BOOL hasMenubar);
extern HWND parentOf(HWND child);
extern HWND parentToplevel(HWND child);
extern void setWindowInsertAfter(HWND hwnd, HWND insertAfter);

// text.cpp
extern WCHAR *windowTextAndLen(HWND hwnd, LRESULT *len);
extern WCHAR *windowText(HWND hwnd);
extern void setWindowText(HWND hwnd, WCHAR *wtext);

// init.cpp
extern HINSTANCE hInstance;
extern int nCmdShow;
extern HFONT hMessageFont;
extern HBRUSH hollowBrush;
extern uiInitOptions options;

// utilwin.cpp
extern HWND utilWindow;
extern const char *initUtilWindow(HICON hDefaultIcon, HCURSOR hDefaultCursor);
extern void uninitUtilWindow(void);

// main.cpp
extern void registerMessageFilter(void);
extern void unregisterMessageFilter(void);

// parent.cpp
extern void paintContainerBackground(HWND hwnd, HDC dc, RECT *paintRect);
extern BOOL handleParentMessages(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT *lResult);

// resize.cpp
extern void doResizes(void);

// d2dscratch.cpp
extern ATOM registerD2DScratchClass(HICON hDefaultIcon, HCURSOR hDefaultCursor);
extern void unregisterD2DScratchClass(void);
extern HWND newD2DScratch(HWND parent, RECT *rect, HMENU controlID, SUBCLASSPROC subclass, DWORD_PTR subclassData);

// area.cpp
#define areaClass L"libui_uiAreaClass"
extern ATOM registerAreaClass(HICON, HCURSOR);
extern void unregisterArea(void);

// areaevents.cpp
extern BOOL areaFilter(MSG *);

// window.cpp
extern ATOM registerWindowClass(HICON, HCURSOR);
extern void unregisterWindowClass(void);
extern void ensureMinimumWindowSize(uiWindow *);

// container.cpp
#define containerClass L"libui_uiContainerClass"
extern ATOM initContainer(HICON, HCURSOR);
extern void uninitContainer(void);
extern HWND newContainer(void);


// TODO
#include "_uipriv_migrate.hpp"

// TODO
// child.cpp
extern struct child *newChild(uiControl *child, uiControl *parent, HWND parentHWND);
extern struct child *newChildWithTabPage(uiControl *child, uiControl *parent, HWND parentHWND);
extern void childRemove(struct child *c);
extern void childDestroy(struct child *c);
extern HWND childHWND(struct child *c);
extern void childMinimumSize(struct child *c, uiWindowsSizing *d, intmax_t *width, intmax_t *height);
extern void childRelayout(struct child *c, intmax_t x, intmax_t y, intmax_t width, intmax_t height);
extern void childQueueRelayout(struct child *c);
extern int childVisible(struct child *c);
extern void childUpdateState(struct child *c);
extern void childAssignControlIDZOrder(struct child *c, LONG_PTR *controlID, HWND *insertAfter);
extern void childSetSoleControlID(struct child *c);
extern HWND childTabPage(struct child *c);
extern int childMargined(struct child *c);
extern void childSetMargined(struct child *c, int margined);
extern int childFlag(struct child *c);
extern void childSetFlag(struct child *c, int flag);
extern intmax_t childIntmax(struct child *c, int n);
extern void childSetIntmax(struct child *c, int n, intmax_t to);
