// 28 april 2019
#include "winapi.hpp"
#include "winhresult.hpp"

// This file wraps standard Windows API functions that don't use HRESULTs to return HRESULTs.
// It also calls SetLastError(0) before each such call.

static inline HRESULT lastErrorCodeToHRESULT(DWORD lastError)
{
	if (lastError == 0)
		return E_FAIL;
	return HRESULT_FROM_WIN32(lastError);
}

static inline HRESULT lastErrorToHRESULT(void)
{
	return lastErrorCodeToHRESULT(GetLastError());
}

HRESULT WINAPI uiprivHrRegisterClassW(const WNDCLASSW *wc)
{
	ATOM a;

	SetLastError(0);
	a = RegisterClassW(wc);
	if (a == 0)
		return lastErrorToHRESULT();
	return S_OK;
}

HRESULT WINAPI uiprivHrCreateWindowExW(DWORD exStyle, LPCWSTR className, LPCWSTR windowName, DWORD style, int x, int y, int width, int height, HWND parent, HMENU menu, HINSTANCE hInstance, LPVOID lpParam, HWND *hwnd)
{
	SetLastError(0);
	*hwnd = CreateWindowExW(exStyle,
		className, windowName,
		style,
		x, y, width, height,
		parent, menu, hInstance, lpParam);
	if (*hwnd == NULL)
		return lastErrorToHRESULT();
	return S_OK;
}

HRESULT WINAPI uiprivHrGetMessageW(LPMSG msg, HWND hwnd, UINT filterMin, UINT filterMax)
{
	BOOL ret;

	SetLastError(0);
	ret = GetMessageW(msg, hwnd, filterMin, filterMax);
	if (ret < 0)
		return lastErrorToHRESULT();
	if (ret == 0)
		return S_FALSE;
	return S_OK;
}

HRESULT WINAPI uiprivHrPostMessageW(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	BOOL ret;

	SetLastError(0);
	ret = PostMessageW(hwnd, uMsg, wParam, lParam);
	if (ret == 0)
		return lastErrorToHRESULT();
	return S_OK;
}

HRESULT WINAPI uiprivHrLoadIconW(HINSTANCE hInstance, LPCWSTR name, HICON *hIcon)
{
	SetLastError(0);
	*hIcon = LoadIconW(hInstance, name);
	if (*hIcon == NULL)
		return lastErrorToHRESULT();
	return S_OK;
}

HRESULT WINAPI uiprivHrLoadCursorW(HINSTANCE hInstance, LPCWSTR name, HCURSOR *hCursor)
{
	SetLastError(0);
	*hCursor = LoadCursorW(hInstance, name);
	if (*hCursor == NULL)
		return lastErrorToHRESULT();
	return S_OK;
}

HRESULT WINAPI uiprivHrSetWindowTextW(HWND hwnd, LPCWSTR text)
{
	SetLastError(0);
	if (SetWindowTextW(hwnd, text) == 0)
		return lastErrorToHRESULT();
	return S_OK;
}

HRESULT WINAPI uiprivHrDestroyWindow(HWND hwnd)
{
	SetLastError(0);
	if (DestroyWindow(hwnd) == 0)
		return lastErrorToHRESULT();
	return S_OK;
}

HRESULT WINAPI uiprivHrGetWindowRect(HWND hwnd, LPRECT r)
{
	SetLastError(0);
	if (GetWindowRect(hwnd, r) == 0)
		// TODO set r to a zero rect?
		return lastErrorToHRESULT();
	return S_OK;
}

HRESULT WINAPI uiprivHrGetClientRect(HWND hwnd, LPRECT r)
{
	SetLastError(0);
	if (GetClientRect(hwnd, r) == 0)
		// TODO set r to a zero rect?
		return lastErrorToHRESULT();
	return S_OK;
}
