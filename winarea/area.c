// 8 september 2015
#include "area.h"

#define areaClass L"libui_uiAreaClass"

static void doPaint(uiArea *a, uiAreaHandler *ah, HDC dc, RECT *client, RECT *clip)
{
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

struct areainit {
	uiArea *a;
	uiAreaHandler *ah;
};

#define gwlpArea 0
#define gwlpAreaHandler (sizeof (uiArea *))

static LRESULT CALLBACK areaWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	uiArea *a;
	uiAreaHandler *ah;
	struct areainit *ai;
	CREATESTRUCTW *cs = (CREATESTRUCTW *) lParam;
	HDC dc;
	PAINTSTRUCT ps;
	RECT client;

	a = (uiArea *) GetWindowLongPtrW(hwnd, gwlpArea);
	ah = (uiAreaHandler *) GetWindowLongPtrW(hwnd, gwlpAreaHandler);
	if (a == NULL) {
		if (uMsg == WM_NCCREATE) {
			ai = (struct areainit *) (cs->lpCreateParams);
			SetWindowLongPtrW(hwnd, gwlpArea, (LONG_PTR) (ai->a));
			SetWindowLongPtrW(hwnd, gwlpAreaHandler, (LONG_PTR) (ai->ah));
		}
		// fall through to DefWindowProcW() anyway
		return DefWindowProcW(hwnd, uMsg, wParam, lParam);
	}

	switch (uMsg) {
	case WM_PAINT:
		dc = BeginPaint(hwnd, &ps);
		if (dc == NULL)
			logLastError("error beginning paint in areaWndProc");
		if (GetClientRect(hwnd, &client) == 0)
			logLastError("error getting client rect in WM_PAINT in areaWndProc()");
		doPaint(a, ah, dc, &client, &(ps.rcPaint));
		EndPaint(hwnd, &ps);
		return 0;
	case WM_PRINTCLIENT:
		if (GetClientRect(hwnd, &client) == 0)
			logLastError("error getting client rect in WM_PRINTCLIENT in areaWndProc()");
		doPaint(a, ah, (HDC) wParam, &client, &client);
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
	wc.cbWndExtra = sizeof (uiArea *) + sizeof (uiAreaHandler *);
	return RegisterClassW(&wc);
}

void unregisterAreaClass(void)
{
	if (UnregisterClassW(areaClass, hInstance) == 0)
		logLastError("error unregistering uiArea window class in unregisterAreaClass()");
}

HWND makeArea(DWORD exstyle, DWORD style, int x, int y, int cx, int cy, HWND parent, uiAreaHandler *ah)
{
	struct areainit ai;

	ai.a = NULL;
	ai.ah = ah;
	return CreateWindowExW(exstyle,
		areaClass, L"",
		style | WS_HSCROLL | WS_VSCROLL,
		x, y, cx, cy,
		parent, NULL, hInstance, &ai);
}
