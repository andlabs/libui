// 28 april 2019

extern HRESULT WINAPI uiprivHrRegisterClassW(const WNDCLASSW *wc);
extern HRESULT WINAPI uiprivHrCreateWindowExW(DWORD exStyle, LPCWSTR className, LPCWSTR windowName, DWORD style, int x, int y, int width, int height, HWND parent, HMENU menu, HINSTANCE hInstance, LPVOID lpParam, HWND *hwnd);
extern HRESULT WINAPI uiprivHrGetMessageW(LPMSG msg, HWND hwnd, UINT filterMin, UINT filterMax, BOOL *ret);
extern HRESULT WINAPI uiprivHrPostMessageW(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
extern HRESULT WINAPI uiprivHrLoadIconW(HINSTANCE hInstance, LPCWSTR name, HICON *hIcon);
extern HRESULT WINAPI uiprivHrLoadCursorW(HINSTANCE hInstance, LPCWSTR name, HCURSOR *hCursor);
