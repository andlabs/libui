// 28 april 2019

extern HRESULT WINAPI uiprivHrRegisterClassW(const WNDCLASSW *wc);
extern HRESULT WINAPI uiprivHrCreateWindowExW(DWORD exStyle, LPCWSTR className, LPCWSTR windowName, DWORD style, int x, int y, int width, int height, HWND parent, HMENU menu, HINSTANCE hInstance, LPVOID lpParam, HWND *hwnd);
// Note: if no error, returns S_FALSE on WM_QUIT, and S_OK otherwise.
extern HRESULT WINAPI uiprivHrGetMessageW(LPMSG msg, HWND hwnd, UINT filterMin, UINT filterMax);
extern HRESULT WINAPI uiprivHrPostMessageW(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
extern HRESULT WINAPI uiprivHrLoadIconW(HINSTANCE hInstance, LPCWSTR name, HICON *hIcon);
extern HRESULT WINAPI uiprivHrLoadCursorW(HINSTANCE hInstance, LPCWSTR name, HCURSOR *hCursor);
extern HRESULT WINAPI uiprivHrSetWindowTextW(HWND hwnd, LPCWSTR text);
extern HRESULT WINAPI uiprivHrDestroyWindow(HWND hwnd);
extern HRESULT WINAPI uiprivHrGetWindowRect(HWND hwnd, LPRECT r);
extern HRESULT WINAPI uiprivHrGetClientRect(HWND hwnd, LPRECT r);
