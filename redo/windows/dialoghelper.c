// 22 may 2015
#include "uipriv_windows.h"

// see http://stackoverflow.com/questions/25494914/is-there-something-like-cdn-filecancel-analogous-to-cdn-fileok-for-getting-when#comment40420049_25494914

struct dialogDisableWindow {
	HWND hwnd;
	uintmax_t n;
	BOOL prevstate;
	UT_hash_handle hh;
};

static struct dialogDisableWindow *windows = NULL;

void dialogHelperRegisterWindow(HWND hwnd)
{
	struct dialogDisableWindow *d;

	HASH_FIND_PTR(windows, &hwnd, d);
	if (d != NULL)
		complain("window handle %p already register in dialogHelperRegisterWindow()", hwnd);
	d = uiNew(struct dialogDisableWindow);
	d->hwnd = hwnd;
	HASH_ADD_PTR(windows, hwnd, d);
}

void dialogHelperUnregisterWindow(HWND hwnd)
{
	struct dialogDisableWindow *d;

	HASH_FIND_PTR(windows, &hwnd, d);
	if (d == NULL)
		complain("window handle %p not registered in dialogHelperUnregisterWindow()", hwnd);
	HASH_DEL(windows, d);
	uiFree(d);
}

static void dialogBegin(void)
{
	struct dialogDisableWindow *d;
	BOOL prevstate;

	for (d = windows; d != NULL; d = d->hh.next) {
		prevstate = EnableWindow(d->hwnd, FALSE);
		// store the previous state in case the window was already disabled by the user
		// (TODO test)
		// note the !; EnableWindow() returns TRUE if window was previously /disabled/
		if (d->n == 0)
			d->prevstate = !prevstate;
		d->n++;
	}
}

static void dialogEnd(void)
{
	struct dialogDisableWindow *d;

	for (d = windows; d != NULL; d = d->hh.next) {
		d->n--;
		if (d->n == 0)
			EnableWindow(d->hwnd, d->prevstate);
	}
}

#define dialogHelperClass L"libui_dialogHelperClass"

static LRESULT CALLBACK dialogHelperWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg) {
	case WM_CREATE:
		dialogBegin();
		break;
	case WM_ENABLE:
		if (wParam != (WPARAM) FALSE)		// enabling
			dialogEnd();
		break;
	}
	return DefWindowProcW(hwnd, uMsg, wParam, lParam);
}

ATOM initDialogHelper(HICON hDefaultIcon, HCURSOR hDefaultCursor)
{
	WNDCLASSW wc;

	ZeroMemory(&wc, sizeof (WNDCLASSW));
	wc.lpszClassName = dialogHelperClass;
	wc.lpfnWndProc = dialogHelperWndProc;
	wc.hInstance = hInstance;
	wc.hIcon = hDefaultIcon;
	wc.hCursor = hDefaultCursor;
	wc.hbrBackground = (HBRUSH) (COLOR_BTNFACE + 1);
	return RegisterClassW(&wc);
}

HWND beginDialogHelper(void)
{
	HWND hwnd;

	hwnd = CreateWindowExW(0,
		dialogHelperClass, L"libui dialog helper",
		WS_OVERLAPPEDWINDOW,
		0, 0, 100, 100,
		NULL, NULL, hInstance, NULL);
	if (hwnd == NULL)
		logLastError("error creating dialog helper in beginDialogHelper()");
	return hwnd;
}

void endDialogHelper(HWND hwnd)
{
	if (DestroyWindow(hwnd) == 0)
		logLastError("error cleaning up after dialog helper in endDialogHelper()");
}
