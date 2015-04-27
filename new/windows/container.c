// 26 april 2015
#include "uipriv_windows.h"

#define containerClass L"libui_uiContainerClass"

HWND initialParent;

struct container {
	HWND hwnd;
	intmax_t marginLeft;
	intmax_t marginTop;
	intmax_t marginRight;
	intmax_t marginBottom;
};

static LRESULT CALLBACK containerWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	// these must always be run, even on the initial parent
	// why? http://blogs.msdn.com/b/oldnewthing/archive/2010/03/16/9979112.aspx
//TODO	switch (uMsg) {
//TODO	}

	return DefWindowProcW(hwnd, uMsg, wParam, lParam);
}

const char *initContainer(void)
{
	WNDCLASSW wc;

	ZeroMemory(&wc, sizeof (WNDCLASSW));
	wc.lpszClassName = containerClass;
	wc.lpfnWndProc = containerWndProc;
	wc.hInstance = hInstance;
	wc.hIcon = hDefaultIcon;
	wc.hCursor = hDefaultCursor;
	wc.hbrBackground = (HBRUSH) (COLOR_BTNFACE + 1);
	if (RegisterClassW(&wc) == 0)
		return "registering uiContainer window class";

	initialParent = CreateWindowExW(0,
		uiOSContainerClass, L"",
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

// TODO document
void uiWindowsMakeContainer(uiContainer *c, const WCHAR *class, BOOL isBin)
{
}

void uiMakeContainer(uiContainer *c)
{
	uiWindowsMakeContainer(c, containerClass, FALSE);
}
