// 8 september 2015
#include "area.h"

#define areaClass L"libui_uiAreaClass"

struct uiArea {
//	uiWindowsControl c;
	HWND hwnd;
	uiAreaHandler *ah;
};

static void doPaint(uiArea *a, HDC dc, RECT *client, RECT *clip)
{
	uiAreaHandler *ah = a->ah;
	uiAreaDrawParams dp;

	dp.Context = newContext(dc);

	dp.ClientWidth = client->right - client->left;
	dp.ClientHeight = client->bottom - client->top;

	dp.ClipX = clip->left;
	dp.ClipY = clip->top;
	dp.ClipWidth = clip->right - clip->left;
	dp.ClipHeight = clip->bottom - clip->top;

	// TODO is this really the best for multimonitor setups?
	dp.DPIX = GetDeviceCaps(dc, LOGPIXELSX);
	dp.DPIY = GetDeviceCaps(dc, LOGPIXELSY);

/* TODO
	dp.HScrollPos = gtk_adjustment_get_value(ap->ha);
	dp.VScrollPos = gtk_adjustment_get_value(ap->va);
*/

	(*(ah->Draw))(ah, a, &dp);
}

static LRESULT CALLBACK areaWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	uiArea *a;
	CREATESTRUCTW *cs = (CREATESTRUCTW *) lParam;
	HDC dc;
	PAINTSTRUCT ps;
	RECT client;

	a = (uiArea *) GetWindowLongPtrW(hwnd, GWLP_USERDATA);
	if (a == NULL) {
		if (uMsg == WM_CREATE)
			SetWindowLongPtrW(hwnd, GWLP_USERDATA, (LONG_PTR) (cs->lpCreateParams));
		// fall through to DefWindowProcW() anyway
		return DefWindowProcW(hwnd, uMsg, wParam, lParam);
	}

	switch (uMsg) {
	case WM_PAINT:
		dc = BeginPaint(a->hwnd, &ps);
		if (dc == NULL)
			logLastError("error beginning paint in areaWndProc");
		if (GetClientRect(a->hwnd, &client) == 0)
			logLastError("error getting client rect in WM_PAINT in areaWndProc()");
		doPaint(a, dc, &client, &(ps.rcPaint));
		EndPaint(a->hwnd, &ps);
		return 0;
	case WM_PRINTCLIENT:
		if (GetClientRect(a->hwnd, &client) == 0)
			logLastError("error getting client rect in WM_PRINTCLIENT in areaWndProc()");
		doPaint(a, (HDC) wParam, &client, &client);
		return 0;
	}
	return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

ATOM registerAreaClass(void)
{
	WNDCLASSW wc;

	ZeroMemory(&wc, sizeof (WNDCLASSW));
	wc.lpszClassName = areaClass;
	wc.lpfnWndProc = areaWndProc;
	wc.hInstance = hInstance;
//TODO	wc.hIcon = hDefaultIcon;
//TODO	wc.hCursor = hDefaultCursor;
	wc.hbrBackground = (HBRUSH) (COLOR_BTNFACE + 1);
	wc.style = CS_HREDRAW | CS_VREDRAW;
	return RegisterClassW(&wc);
}

void unregisterAreaClass(void)
{
	if (UnregisterClassW(areaClass, hInstance) == 0)
		logLastError("error unregistering uiArea window class in unregisterAreaClass()");
}

HWND makeArea(DWORD exstyle, DWORD style, int x, int y, int cx, int cy, HWND parent, uiAreaHandler *ah)
{
	uiArea *a;

	// TODO
	a = malloc(sizeof (uiArea));

	a->ah = ah;

	a->hwnd = CreateWindowExW(exstyle,
		areaClass, L"",
		style | WS_HSCROLL | WS_VSCROLL,
		x, y, cx, cy,
		parent, NULL, hInstance, a);

	return a->hwnd;
}

void areaUpdateScroll(HWND area)
{
	// TODO
}
