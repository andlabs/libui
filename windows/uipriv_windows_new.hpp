// 21 april 2016

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
extern WCHAR *debugstrf(const WCHAR *format, ..);
extern WCHAR *debugvstrf(const WCHAR *format, va_list ap);
extern char *LFtoCRLF(const char *lfonly);
extern void CRLFtoLF(const char *s);

// debug.cpp
#define debugargs const WCHAR *file, uintmax_t line, const WCHAR *file
extern HRESULT _logLastError(debugargs, const WCHAR *func, const WCHAR *s);
#define logLastError(s) _logLastError(L ## __FILE__, __LINE__, L ## __func__, s)
extern HRESULT _logHRESULT(debugargs, const WCHAR *s, HRESULT hr);
#define logHRESULT(s, hr) _logHRESULT(L ## __FILE__, __LINE__, L ## __func__, s, hr)
extern void _implbug(debugargs, const WCHAR *format, ...);
#define implbug(...) _implbug(L ## __FILE__, __LINE__, L ## __func__, __VA_LIST__)

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
