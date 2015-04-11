// 10 april 2015
#include "uipriv_windows.h"

// for maximum safety, all controls that don't have a parent are made children of this, the "initial parent"
// it behaves like other containers due to bugs described in container_windows.c, but is never seen, is disabled, and cannot be interacted with by end users
// despite being called the initial parent, it is used whenever a control has no parent, even if it loses its parent at some later point during the execution of the program

#define uiInitialParentClass L"uiInitialParentClass"

HWND initialParent;

static LRESULT CALLBACK initialParentWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	LRESULT lResult;

	if (sharedWndProc(hwnd, uMsg, wParam, lParam, &lResult) != FALSE)
		return lResult;
	return DefWindowProcW(hwnd, uMsg, wParam, lParam);
}

const char *initInitialParent(HICON hDefaultIcon, HCURSOR hDefaultCursor)
{
	WNDCLASSW wc;

	ZeroMemory(&wc, sizeof (WNDCLASSW));
	wc.lpszClassName = uiInitialParentClass;
	wc.lpfnWndProc = initialParentWndProc;
	wc.hInstance = hInstance;
	wc.hIcon = hDefaultIcon;
	wc.hCursor = hDefaultCursor;
	wc.hbrBackground = (HBRUSH) (COLOR_BTNFACE + 1);
	if (RegisterClassW(&wc) == 0)
		return "registering initial parent window class";

	initialParent = CreateWindowExW(0,
		uiInitialParentClass, L"",
		WS_OVERLAPPEDWINDOW,
		0, 0,
		100, 100,
		NULL, NULL, hInstance, NULL);
	if (initialParent == NULL)
		return "creating initial parent window";

	// just to be safe, disable the initial parent so it can't be interacted with accidentally
	// if this causes issues for our controls, we can remove it
	EnableWindow(initialParent, FALSE);
	return NULL;
}
