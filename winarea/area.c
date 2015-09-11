// 8 september 2015
#include "area.h"

#define areaClass L"libui_uiAreaClass"

struct uiArea {
//	uiWindowsControl c;
	HWND hwnd;
	uiAreaHandler *ah;
	intmax_t hscrollpos;
	intmax_t vscrollpos;
	int hwheelCarry;
	int vwheelCarry;
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

	dp.HScrollPos = a->hscrollpos;
	dp.VScrollPos = a->vscrollpos;

	(*(ah->Draw))(ah, a, &dp);
}

struct scrollParams {
	intmax_t *pos;
	intmax_t pagesize;
	intmax_t length;
	int *wheelCarry;
	UINT wheelSPIAction;
};

void scrollto(uiArea *a, int which, struct scrollParams *p, intmax_t pos)
{
	SCROLLINFO si;
	intmax_t xamount, yamount;

	// note that the pos < 0 check is /after/ the p->length - p->pagesize check
	// it used to be /before/; this was actually a bug in Raymond Chen's original algorithm: if there are fewer than a page's worth of items, p->length - p->pagesize will be negative and our content draw at the bottom of the window
	// this SHOULD have the same effect with that bug fixed and no others introduced... (thanks to devin on irc.badnik.net for confirming this logic)
	// TODO verify this still holds with uiArea
	if (pos > p->length - p->pagesize)
		pos = p->length - p->pagesize;
	if (pos < 0)
		pos = 0;

	// negative because ScrollWindowEx() is "backwards"
	xamount = -(pos - *(p->pos));
	yamount = 0;
	if (which == SB_VERT) {
		yamount = xamount;
		xamount = 0;
	}

	if (ScrollWindowEx(a->hwnd, xamount, yamount,
		NULL, NULL, NULL, NULL,
		SW_ERASE | SW_INVALIDATE) == ERROR)
;//TODO		logLastError("error scrolling area in scrollto()");

	*(p->pos) = pos;

	// now commit our new scrollbar setup...
	ZeroMemory(&si, sizeof (SCROLLINFO));
	si.cbSize = sizeof (SCROLLINFO);
	si.fMask = SIF_PAGE | SIF_POS | SIF_RANGE;
	si.nPage = p->pagesize;
	si.nMin = 0;
	si.nMax = p->length - 1;		// endpoint inclusive
	si.nPos = *(p->pos);
	SetScrollInfo(a->hwnd, which, &si, TRUE);
}

void scrollby(uiArea *a, int which, struct scrollParams *p, intmax_t delta)
{
	scrollto(a, which, p, *(p->pos) + delta);
}

void scroll(uiArea *a, int which, struct scrollParams *p, WPARAM wParam, LPARAM lParam)
{
	intmax_t pos;
	SCROLLINFO si;

	pos = *(p->pos);
	switch (LOWORD(wParam)) {
	case SB_LEFT:			// also SB_TOP
		pos = 0;
		break;
	case SB_RIGHT:		// also SB_BOTTOM
		pos = p->length - p->pagesize;
		break;
	case SB_LINELEFT:		// also SB_LINEUP
		pos--;
		break;
	case SB_LINERIGHT:		// also SB_LINEDOWN
		pos++;
		break;
	case SB_PAGELEFT:		// also SB_PAGEUP
		pos -= p->pagesize;
		break;
	case SB_PAGERIGHT:	// also SB_PAGEDOWN
		pos += p->pagesize;
		break;
	case SB_THUMBPOSITION:
		ZeroMemory(&si, sizeof (SCROLLINFO));
		si.cbSize = sizeof (SCROLLINFO);
		si.fMask = SIF_POS;
		if (GetScrollInfo(a->hwnd, which, &si) == 0)
			logLastError("error getting thumb position for area in scroll()");
		pos = si.nPos;
		break;
	case SB_THUMBTRACK:
		ZeroMemory(&si, sizeof (SCROLLINFO));
		si.cbSize = sizeof (SCROLLINFO);
		si.fMask = SIF_TRACKPOS;
		if (GetScrollInfo(a->hwnd, which, &si) == 0)
			logLastError("error getting thumb track position for area in scroll()");
		pos = si.nTrackPos;
		break;
	}
	scrollto(a, which, p, pos);
}

static void wheelscroll(uiArea *a, int which, struct scrollParams *p, WPARAM wParam, LPARAM lParam)
{
	int delta;
	int lines;
	UINT scrollAmount;

	delta = GET_WHEEL_DELTA_WPARAM(wParam);
	if (SystemParametersInfoW(p->wheelSPIAction, 0, &scrollAmount, 0) == 0)
		// TODO use scrollAmount == 3 (for both v and h) instead?
		logLastError("error getting area wheel scroll amount in wheelscroll()");
	if (scrollAmount == WHEEL_PAGESCROLL)
		scrollAmount = p->pagesize;
	if (scrollAmount == 0)		// no mouse wheel scrolling (or t->pagesize == 0)
		return;
	// the rest of this is basically http://blogs.msdn.com/b/oldnewthing/archive/2003/08/07/54615.aspx and http://blogs.msdn.com/b/oldnewthing/archive/2003/08/11/54624.aspx
	// see those pages for information on subtleties
	delta += *(p->wheelCarry);
	lines = delta * ((int) scrollAmount) / WHEEL_DELTA;
	*(p->wheelCarry) = delta - lines * WHEEL_DELTA / ((int) scrollAmount);
	return scrollby(a, which, p, -lines);
}

static void hscrollParams(uiArea *a, struct scrollParams *p)
{
	RECT r;

	ZeroMemory(p, sizeof (struct scrollParams));
	p->pos = &(a->hscrollpos);
	if (GetClientRect(a->hwnd, &r) == 0)
		logLastError("error getting area client rect in hscrollParams()");
	p->pagesize = r.right - r.left;
	p->length = (*(a->ah->HScrollMax))(a->ah, a);
	p->wheelCarry = &(a->hwheelCarry);
	p->wheelSPIAction = SPI_GETWHEELSCROLLCHARS;
}

static void hscrollto(uiArea *a, intmax_t pos)
{
	struct scrollParams p;

	hscrollParams(a, &p);
	scrollto(a, SB_HORZ, &p, pos);
}

static void hscrollby(uiArea *a, intmax_t delta)
{
	struct scrollParams p;

	hscrollParams(a, &p);
	scrollby(a, SB_HORZ, &p, delta);
}

static void hscroll(uiArea *a, WPARAM wParam, LPARAM lParam)
{
	struct scrollParams p;

	hscrollParams(a, &p);
	scroll(a, SB_HORZ, &p, wParam, lParam);
}

static void hwheelscroll(uiArea *a, WPARAM wParam, LPARAM lParam)
{
	struct scrollParams p;

	hscrollParams(a, &p);
	wheelscroll(a, SB_HORZ, &p, wParam, lParam);
}

static void vscrollParams(uiArea *a, struct scrollParams *p)
{
	RECT r;

	ZeroMemory(p, sizeof (struct scrollParams));
	p->pos = &(a->vscrollpos);
	if (GetClientRect(a->hwnd, &r) == 0)
		logLastError("error getting area client rect in vscrollParams()");
	p->pagesize = r.bottom - r.top;
	p->length = (*(a->ah->VScrollMax))(a->ah, a);
	p->wheelCarry = &(a->vwheelCarry);
	p->wheelSPIAction = SPI_GETWHEELSCROLLLINES;
}

static void vscrollto(uiArea *a, intmax_t pos)
{
	struct scrollParams p;

	vscrollParams(a, &p);
	scrollto(a, SB_VERT, &p, pos);
}

static void vscrollby(uiArea *a, intmax_t delta)
{
	struct scrollParams p;

	vscrollParams(a, &p);
	scrollby(a, SB_VERT, &p, delta);
}

static void vscroll(uiArea *a, WPARAM wParam, LPARAM lParam)
{
	struct scrollParams p;

	vscrollParams(a, &p);
	scroll(a, SB_VERT, &p, wParam, lParam);
}

static void vwheelscroll(uiArea *a, WPARAM wParam, LPARAM lParam)
{
	struct scrollParams p;

	vscrollParams(a, &p);
	wheelscroll(a, SB_VERT, &p, wParam, lParam);
}

static LRESULT CALLBACK areaWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	uiArea *a;
	CREATESTRUCTW *cs = (CREATESTRUCTW *) lParam;
	HDC dc;
	PAINTSTRUCT ps;
	RECT client;
	WINDOWPOS *wp = (WINDOWPOS *) lParam;

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
	case WM_WINDOWPOSCHANGED:
		if ((wp->flags & SWP_NOSIZE) != 0)
			break;
		if ((*(a->ah->RedrawOnResize))(a->ah, a))
			if (InvalidateRect(a->hwnd, NULL, TRUE) == 0)
				logLastError("error redrawing area on resize in areaWndProc()");
		return 0;
	case WM_HSCROLL:
		hscroll(a, wParam, lParam);
		return 0;
	case WM_MOUSEHWHEEL:
		hwheelscroll(a, wParam, lParam);
		return 0;
	case WM_VSCROLL:
		vscroll(a, wParam, lParam);
		return 0;
	case WM_MOUSEWHEEL:
		vwheelscroll(a, wParam, lParam);
		return 0;
	case WM_LBUTTONDOWN:
		// TODO
		SetFocus(a->hwnd);
		break;
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
	// don't specify CS_HREDRAW or CS_VREDRAW; that's decided by the uiAreaHandler in RedrawOnResize()
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
	uiArea *a;

	a = (uiArea *) GetWindowLongPtrW(area, GWLP_USERDATA);
	// use a no-op scroll to simulate scrolling
	hscrollby(a, 0);
	vscrollby(a, 0);
}
