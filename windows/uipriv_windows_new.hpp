// 21 april 2016

// events.cpp
extern BOOL runWM_COMMAND(WPARAM wParam, LPARAM lParam, LRESULT *lResult);
extern BOOL runWM_NOTIFY(WPARAM wParam, LPARAM lParam, LRESULT *lResult);
extern BOOL runWM_HSCROLL(WPARAM wParam, LPARAM lParam, LRESULT *lResult);
extern void issueWM_WININICHANGE(WPARAM wParam, LPARAM lParam);

// utf16.cpp
extern WCHAR *toUTF16(const char *str);
extern char *toUTF8(const WCHAR *wstr);
extern WCHAR *strf(const WCHAR *format, ...);
extern WCHAR *vstrf(const WCHAR *format, va_list ap);
extern WCHAR *debugstrf(const WCHAR *format, ..);
extern WCHAR *debugvstrf(const WCHAR *format, va_list ap);

// debug.cpp
#define debugargs const WCHAR *file, uintmax_t line, const WCHAR *file
extern HRESULT _logLastError(debugargs, const WCHAR *func, const WCHAR *s);
#define logLastError(s) _logLastError(L ## __FILE__, __LINE__, L ## __func__, s)
extern HRESULT _logHRESULT(debugargs, const WCHAR *s, HRESULT hr);
#define logHRESULT(s, hr) _logHRESULT(L ## __FILE__, __LINE__, L ## __func__, s, hr)
extern void _implbug(debugargs, const WCHAR *format, ...);
#define implbug(...) _implbug(L ## __FILE__, __LINE__, L ## __func__, __VA_LIST__)
