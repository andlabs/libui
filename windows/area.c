// 8 september 2015
#include "uipriv_windows.h"

// TODOs
// - things look very wrong on initial draw
// - initial scrolling is not set properly
// - should background be inherited from parent control?

struct uiArea {
	uiWindowsControl c;
	HWND hwnd;
	uiAreaHandler *ah;
	intmax_t hscrollpos;
	intmax_t vscrollpos;
	int hwheelCarry;
	int vwheelCarry;
	clickCounter cc;
	BOOL capturing;
	ID2D1HwndRenderTarget *rt;
};

uiWindowsDefineControl(
	uiArea,								// type name
	uiAreaType							// type function
)

static HRESULT doPaint(uiArea *a, ID2D1RenderTarget *rt, RECT *clip)
{
	uiAreaHandler *ah = a->ah;
	uiAreaDrawParams dp;
	COLORREF bgcolorref;
	D2D1_COLOR_F bgcolor;
	D2D1_SIZE_F size;

	dp.Context = newContext(rt);

	ID2D1RenderTarget_GetSize(rt, &size);
	dp.ClientWidth = size.width;
	dp.ClientHeight = size.height;

	dp.ClipX = clip->left;
	dp.ClipY = clip->top;
	dp.ClipWidth = clip->right - clip->left;
	dp.ClipHeight = clip->bottom - clip->top;

	dp.HScrollPos = a->hscrollpos;
	dp.VScrollPos = a->vscrollpos;

	ID2D1RenderTarget_BeginDraw(rt);

	// TODO only clear the clip area
	// TODO clear with actual background color
	bgcolorref = GetSysColor(COLOR_BTNFACE);
	bgcolor.r = ((float) GetRValue(bgcolorref)) / 255.0;
	// due to utter apathy on Microsoft's part, GetGValue() does not work with MSVC's Run-Time Error Checks
	// it has not worked since 2008 and they have *never* fixed it
	bgcolor.g = ((float) ((BYTE) ((bgcolorref & 0xFF00) >> 8))) / 255.0;
	bgcolor.b = ((float) GetBValue(bgcolorref)) / 255.0;
	bgcolor.a = 1.0;
	ID2D1RenderTarget_Clear(rt, &bgcolor);

	// no need to save or restore the graphics state to reset transformations; it's handled by resetTarget() in draw.c, called by newContext() above
	(*(ah->Draw))(ah, a, &dp);

	freeContext(dp.Context);

	return ID2D1RenderTarget_EndDraw(rt, NULL, NULL);
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

	// TODO this isn't safe with Direct2D
	if (ScrollWindowEx(a->hwnd, xamount, yamount,
		NULL, NULL, NULL, NULL,
		SW_ERASE | SW_INVALIDATE) == ERROR)
		logLastError("error scrolling area in scrollto()");

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
	scrollby(a, which, p, -lines);
}

static void hscrollParams(uiArea *a, struct scrollParams *p)
{
	RECT r;

	ZeroMemory(p, sizeof (struct scrollParams));
	p->pos = &(a->hscrollpos);
	// TODO get rid of these and replace with points
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

static uiModifiers getModifiers(void)
{
	uiModifiers m = 0;

	if ((GetKeyState(VK_CONTROL) & 0x80) != 0)
		m |= uiModifierCtrl;
	if ((GetKeyState(VK_MENU) & 0x80) != 0)
		m |= uiModifierAlt;
	if ((GetKeyState(VK_SHIFT) & 0x80) != 0)
		m |= uiModifierShift;
	if ((GetKeyState(VK_LWIN) & 0x80) != 0)
		m |= uiModifierSuper;
	if ((GetKeyState(VK_RWIN) & 0x80) != 0)
		m |= uiModifierSuper;
	return m;
}

static void areaMouseEvent(uiArea *a, uintmax_t down, uintmax_t  up, WPARAM wParam, LPARAM lParam)
{
	uiAreaMouseEvent me;
	uintmax_t button;
	double xpix, ypix;
	FLOAT dpix, dpiy;
	D2D1_SIZE_F size;

	if (a->rt == NULL)
		a->rt = makeHWNDRenderTarget(a->hwnd);

	xpix = (double) GET_X_LPARAM(lParam);
	ypix = (double) GET_Y_LPARAM(lParam);
	// these are in pixels; we need points
	ID2D1HwndRenderTarget_GetDpi(a->rt, &dpix, &dpiy);
	// see https://msdn.microsoft.com/en-us/library/windows/desktop/dd756649%28v=vs.85%29.aspx (and others; search "direct2d mouse")
	me.X = (xpix * 96) / dpix;
	me.Y = (ypix * 96) / dpiy;

	rtGetSize((ID2D1RenderTarget *) (a->rt), &size);
	me.ClientWidth = size.width;
	me.ClientHeight = size.height;
	me.HScrollPos = a->hscrollpos;
	me.VScrollPos = a->vscrollpos;

	me.Down = down;
	me.Up = up;
	me.Count = 0;
	if (me.Down != 0)
		// GetMessageTime() returns LONG and GetDoubleClckTime() returns UINT, which are int32 and uint32, respectively, but we don't need to worry about the signedness because for the same bit widths and two's complement arithmetic, s1-s2 == u1-u2 if bits(s1)==bits(s2) and bits(u1)==bits(u2) (and Windows requires two's complement: http://blogs.msdn.com/b/oldnewthing/archive/2005/05/27/422551.aspx)
		// signedness isn't much of an issue for these calls anyway because http://stackoverflow.com/questions/24022225/what-are-the-sign-extension-rules-for-calling-windows-api-functions-stdcall-t and that we're only using unsigned values (think back to how you (didn't) handle signedness in assembly language) AND because of the above AND because the statistics below (time interval and width/height) really don't make sense if negative
		me.Count = clickCounterClick(&(a->cc), me.Down,
			me.X, me.Y,
			GetMessageTime(), GetDoubleClickTime(),
			GetSystemMetrics(SM_CXDOUBLECLK) / 2,
			GetSystemMetrics(SM_CYDOUBLECLK) / 2);

	// though wparam will contain control and shift state, let's just one function to get modifiers for both keyboard and mouse events; it'll work the same anyway since we have to do this for alt and windows key (super)
	me.Modifiers = getModifiers();

	button = me.Down;
	if (button == 0)
		button = me.Up;
	me.Held1To64 = 0;
	if (button != 1 && (wParam & MK_LBUTTON) != 0)
		me.Held1To64 |= 1 << 0;
	if (button != 2 && (wParam & MK_MBUTTON) != 0)
		me.Held1To64 |= 1 << 1;
	if (button != 3 && (wParam & MK_RBUTTON) != 0)
		me.Held1To64 |= 1 << 2;
	if (button != 4 && (wParam & MK_XBUTTON1) != 0)
		me.Held1To64 |= 1 << 3;
	if (button != 5 && (wParam & MK_XBUTTON2) != 0)
		me.Held1To64 |= 1 << 4;

	// on Windows, we have to capture on drag ourselves
	if (me.Down != 0 && !a->capturing) {
		SetCapture(a->hwnd);
		a->capturing = TRUE;
	}
	// only release capture when all buttons released
	if (me.Up != 0 && a->capturing && me.Held1To64 == 0) {
		// unset flag first as ReleaseCapture() sends WM_CAPTURECHANGED
		a->capturing = FALSE;
		if (ReleaseCapture() == 0)
			logLastError("error releasing capture on drag in areaMouseEvent()");
	}

	(*(a->ah->MouseEvent))(a->ah, a, &me);
}

// we use VK_SNAPSHOT as a sentinel because libui will never support the print screen key; that key belongs to the user
struct extkeymap {
	WPARAM vk;
	uiExtKey extkey;
};

// all mappings come from GLFW - https://github.com/glfw/glfw/blob/master/src/win32_window.c#L152
static const struct extkeymap numpadExtKeys[] = {
	{ VK_HOME, uiExtKeyN7 },
	{ VK_UP, uiExtKeyN8 },
	{ VK_PRIOR, uiExtKeyN9 },
	{ VK_LEFT, uiExtKeyN4 },
	{ VK_CLEAR, uiExtKeyN5 },
	{ VK_RIGHT, uiExtKeyN6 },
	{ VK_END, uiExtKeyN1 },
	{ VK_DOWN, uiExtKeyN2 },
	{ VK_NEXT, uiExtKeyN3 },
	{ VK_INSERT, uiExtKeyN0 },
	{ VK_DELETE, uiExtKeyNDot },
	{ VK_SNAPSHOT, 0 },
};

static const struct extkeymap extKeys[] = {
	{ VK_ESCAPE, uiExtKeyEscape },
	{ VK_INSERT, uiExtKeyInsert },
	{ VK_DELETE, uiExtKeyDelete },
	{ VK_HOME, uiExtKeyHome },
	{ VK_END, uiExtKeyEnd },
	{ VK_PRIOR, uiExtKeyPageUp },
	{ VK_NEXT, uiExtKeyPageDown },
	{ VK_UP, uiExtKeyUp },
	{ VK_DOWN, uiExtKeyDown },
	{ VK_LEFT, uiExtKeyLeft },
	{ VK_RIGHT, uiExtKeyRight },
	{ VK_F1, uiExtKeyF1 },
	{ VK_F2, uiExtKeyF2 },
	{ VK_F3, uiExtKeyF3 },
	{ VK_F4, uiExtKeyF4 },
	{ VK_F5, uiExtKeyF5 },
	{ VK_F6, uiExtKeyF6 },
	{ VK_F7, uiExtKeyF7 },
	{ VK_F8, uiExtKeyF8 },
	{ VK_F9, uiExtKeyF9 },
	{ VK_F10, uiExtKeyF10 },
	{ VK_F11, uiExtKeyF11 },
	{ VK_F12, uiExtKeyF12 },
	// numpad numeric keys and . are handled in events.c
	// numpad enter is handled in code below
	{ VK_ADD, uiExtKeyNAdd },
	{ VK_SUBTRACT, uiExtKeyNSubtract },
	{ VK_MULTIPLY, uiExtKeyNMultiply },
	{ VK_DIVIDE, uiExtKeyNDivide },
	{ VK_SNAPSHOT, 0 },
};

static const struct {
	WPARAM vk;
	uiModifiers mod;
} modKeys[] = {
	// even if the separate left/right aren't necessary, have them here anyway, just to be safe
	{ VK_CONTROL, uiModifierCtrl },
	{ VK_LCONTROL, uiModifierCtrl },
	{ VK_RCONTROL, uiModifierCtrl },
	{ VK_MENU, uiModifierAlt },
	{ VK_LMENU, uiModifierAlt },
	{ VK_RMENU, uiModifierAlt },
	{ VK_SHIFT, uiModifierShift },
	{ VK_LSHIFT, uiModifierShift },
	{ VK_RSHIFT, uiModifierShift },
	// there's no combined Windows key virtual-key code as there is with the others
	{ VK_LWIN, uiModifierSuper },
	{ VK_RWIN, uiModifierSuper },
	{ VK_SNAPSHOT, 0 },
};

static int areaKeyEvent(uiArea *a, int up, WPARAM wParam, LPARAM lParam)
{
	uiAreaKeyEvent ke;
	int righthand;
	int i;

	ke.Key = 0;
	ke.ExtKey = 0;
	ke.Modifier = 0;

	ke.Modifiers = getModifiers();

	ke.Up = up;

	// the numeric keypad keys when Num Lock is off are considered left-hand keys as the separate navigation buttons were added later
	// the numeric keypad enter, however, is a right-hand key because it has the same virtual-key code as the typewriter enter
	righthand = (lParam & 0x01000000) != 0;
	if (righthand) {
		if (wParam == VK_RETURN) {
			ke.ExtKey = uiExtKeyNEnter;
			goto keyFound;
		}
	} else
		// this is special handling for numpad keys to ignore the state of Num Lock and Shift; see http://blogs.msdn.com/b/oldnewthing/archive/2004/09/06/226045.aspx and https://github.com/glfw/glfw/blob/master/src/win32_window.c#L152
		for (i = 0; numpadExtKeys[i].vk != VK_SNAPSHOT; i++)
			if (numpadExtKeys[i].vk == wParam) {
				ke.ExtKey = numpadExtKeys[i].extkey;
				goto keyFound;
			}

	// okay, those above cases didn't match anything
	// first try the extended keys
	for (i = 0; extKeys[i].vk != VK_SNAPSHOT; i++)
		if (extKeys[i].vk == wParam) {
			ke.ExtKey = extKeys[i].extkey;
			goto keyFound;
		}

	// then try modifier keys
	for (i = 0; modKeys[i].vk != VK_SNAPSHOT; i++)
		if (modKeys[i].vk == wParam) {
			ke.Modifier = modKeys[i].mod;
			// and don't include the key in Modifiers
			ke.Modifiers &= ~ke.Modifier;
			goto keyFound;
		}

	// and finally everything else
	if (fromScancode((lParam >> 16) & 0xFF, &ke))
		goto keyFound;

	// not a supported key, assume unhandled
	// TODO the original code only did this if ke.Modifiers == 0 - why?
	return 0;

keyFound:
	return (*(a->ah->KeyEvent))(a->ah, a, &ke);
}

// We don't handle the standard Windows keyboard messages directly, to avoid both the dialog manager and TranslateMessage().
// Instead, we set up a message filter and do things there.
// That stuff is later in this file.
enum {
	// start at 0x40 to avoid clobbering dialog messages
	msgAreaKeyDown = WM_USER + 0x40,
	msgAreaKeyUp,
};

static LRESULT CALLBACK areaWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	uiArea *a;
	CREATESTRUCTW *cs = (CREATESTRUCTW *) lParam;
	RECT client, clip;
	WINDOWPOS *wp = (WINDOWPOS *) lParam;
	D2D1_SIZE_U size;
	HRESULT hr;

	a = (uiArea *) GetWindowLongPtrW(hwnd, GWLP_USERDATA);
	if (a == NULL) {
		if (uMsg == WM_CREATE)
			SetWindowLongPtrW(hwnd, GWLP_USERDATA, (LONG_PTR) (cs->lpCreateParams));
		// fall through to DefWindowProcW() anyway
		return DefWindowProcW(hwnd, uMsg, wParam, lParam);
	}

	switch (uMsg) {
	case WM_PAINT:
		if (a->rt == NULL)
			a->rt = makeHWNDRenderTarget(a->hwnd);
		// do not clear the update rect; we do that ourselves in doPaint()
		if (GetUpdateRect(a->hwnd, &clip, FALSE) == 0) {
			// set a zero clip rect just in case GetUpdateRect() didn't change clip
			clip.left = 0;
			clip.top = 0;
			clip.right = 0;
			clip.bottom = 0;
		}
		hr = doPaint(a, (ID2D1RenderTarget *) (a->rt), &clip);
		switch (hr) {
		case S_OK:
			if (ValidateRect(a->hwnd, NULL) == 0)
				logLastError("error validating rect ater WM_PAINT in areaWndProc()");
			break;
		case D2DERR_RECREATE_TARGET:
			// DON'T validate the rect
			// instead, simply drop the render target
			// we'll get another WM_PAINT and make the render target again
			// TODO would this require us to invalidate the entire client area?
			ID2D1HwndRenderTarget_Release(a->rt);
			a->rt = NULL;
			break;
		default:
			logHRESULT("error painting in WM_PAINT in areaWndProc()", hr);
		}
		return 0;
	case WM_PRINTCLIENT:
		if (GetClientRect(a->hwnd, &client) == 0)
			logLastError("error getting client rect in WM_PRINTCLIENT in areaWndProc()");
//TODO		doPaint(a, (HDC) wParam, &client);
		return 0;
	case WM_WINDOWPOSCHANGED:
		if ((wp->flags & SWP_NOSIZE) != 0)
			break;
		if (a->rt == NULL)
			a->rt = makeHWNDRenderTarget(a->hwnd);
		else {
			if (GetClientRect(a->hwnd, &client) == 0)
				logLastError("error getting client rect for resizing Direct2D render target in areaWndProc()");
			size.width = client.right - client.left;
			size.height = client.bottom - client.top;
			// don't track the error; we'll get that in EndDraw()
			// see https://msdn.microsoft.com/en-us/library/windows/desktop/dd370994%28v=vs.85%29.aspx
			ID2D1HwndRenderTarget_Resize(a->rt, &size);
		}
		// according to Rick Brewster, we must always redraw the entire client area after calling ID2D1RenderTarget::Resize() (see http://stackoverflow.com/a/33222983/3408572)
		// we used to have a uiAreaHandler.RedrawOnResize() method to decide this; now you know why we don't anymore
		// TODO consider changing the scrolling model
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
	case WM_ACTIVATE:
		// don't keep the double-click timer running if the user switched programs in between clicks
		clickCounterReset(&(a->cc));
		return 0;
	case WM_MOUSEMOVE:
		areaMouseEvent(a, 0, 0, wParam, lParam);
		return 0;
	case WM_LBUTTONDOWN:
		SetFocus(a->hwnd);
		areaMouseEvent(a, 1, 0, wParam, lParam);
		return 0;
	case WM_LBUTTONUP:
		areaMouseEvent(a, 0, 1, wParam, lParam);
		return 0;
	case WM_MBUTTONDOWN:
		SetFocus(a->hwnd);
		areaMouseEvent(a, 2, 0, wParam, lParam);
		return 0;
	case WM_MBUTTONUP:
		areaMouseEvent(a, 0, 2, wParam, lParam);
		return 0;
	case WM_RBUTTONDOWN:
		SetFocus(a->hwnd);
		areaMouseEvent(a, 3, 0, wParam, lParam);
		return 0;
	case WM_RBUTTONUP:
		areaMouseEvent(a, 0, 3, wParam, lParam);
		return 0;
	case WM_XBUTTONDOWN:
		SetFocus(a->hwnd);
		// values start at 1; we want them to start at 4
		areaMouseEvent(a,
			GET_XBUTTON_WPARAM(wParam) + 3, 0,
			GET_KEYSTATE_WPARAM(wParam), lParam);
		return TRUE;		// XBUTTON messages are different!
	case WM_XBUTTONUP:
		areaMouseEvent(a,
			0, GET_XBUTTON_WPARAM(wParam) + 3,
			GET_KEYSTATE_WPARAM(wParam), lParam);
		return TRUE;		// XBUTTON messages are different!
	case WM_CAPTURECHANGED:
		if (a->capturing) {
			a->capturing = FALSE;
			(*(a->ah->DragBroken))(a->ah, a);
		}
		return 0;
	case msgAreaKeyDown:
		return (LRESULT) areaKeyEvent(a, 0, wParam, lParam);
	case msgAreaKeyUp:
		return (LRESULT) areaKeyEvent(a, 1, wParam, lParam);
	}
	return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

// control implementation

static void minimumSize(uiWindowsControl *c, uiWindowsSizing *d, intmax_t *width, intmax_t *height)
{
	// TODO
	*width = 0;
	*height = 0;
}

ATOM registerAreaClass(HICON hDefaultIcon, HCURSOR hDefaultCursor)
{
	WNDCLASSW wc;

	ZeroMemory(&wc, sizeof (WNDCLASSW));
	wc.lpszClassName = areaClass;
	wc.lpfnWndProc = areaWndProc;
	wc.hInstance = hInstance;
	wc.hIcon = hDefaultIcon;
	wc.hCursor = hDefaultCursor;
	wc.hbrBackground = (HBRUSH) (COLOR_BTNFACE + 1);
	// TODO specify CS_HREDRAW/CS_VREDRAW in addition to or instead of calling InvalidateRect(NULL) in WM_WINDOWPOSCHANGED above, or not at all?
	return RegisterClassW(&wc);
}

static HHOOK areaFilter;

// TODO affect visibility properly
// TODO what did this mean
static LRESULT CALLBACK areaFilterProc(int code, WPARAM wParam, LPARAM lParam)
{
	MSG *msg = (MSG *) lParam;
	LRESULT handled;

	if (code < 0)
		goto callNext;

	// is the recipient an area?
	if (msg->hwnd == NULL)		// this can happen; for example, WM_TIMER
		goto callNext;
	if (windowClassOf(msg->hwnd, areaClass, NULL) != 0)
		goto callNext;		// nope

	handled = 0;
	switch (msg->message) {
	case WM_KEYDOWN:
	case WM_SYSKEYDOWN:
		handled = SendMessageW(msg->hwnd, msgAreaKeyDown, msg->wParam, msg->lParam);
		break;
	case WM_KEYUP:
	case WM_SYSKEYUP:
		handled = SendMessageW(msg->hwnd, msgAreaKeyUp, msg->wParam, msg->lParam);
		break;
	// otherwise handled remains 0, as we didn't handle this
	}
	if (!handled)
		goto callNext;

	// we handled it; discard the message so the dialog manager doesn't see it
	return 1;

callNext:
	return CallNextHookEx(areaFilter, code, wParam, lParam);
}

int registerAreaFilter(void)
{
	areaFilter = SetWindowsHookExW(WH_MSGFILTER,
		areaFilterProc,
		hInstance,
		GetCurrentThreadId());
	return areaFilter != NULL;
}

void unregisterArea(void)
{
	if (UnhookWindowsHookEx(areaFilter) == 0)
		logLastError("error unregistering uiArea message filter in unregisterArea()");
	if (UnregisterClassW(areaClass, hInstance) == 0)
		logLastError("error unregistering uiArea window class in unregisterArea()");
}

void uiAreaUpdateScroll(uiArea *a)
{
	// use a no-op scroll to simulate scrolling
	hscrollby(a, 0);
	vscrollby(a, 0);
}

void uiAreaQueueRedrawAll(uiArea *a)
{
	// don't erase the background; we do that ourselves in doPaint()
	if (InvalidateRect(a->hwnd, NULL, FALSE) == 0)
		logLastError("error queueing uiArea redraw in uiAreaQueueRedrawAll()");
}

uiArea *uiNewArea(uiAreaHandler *ah)
{
	uiArea *a;

	a = (uiArea *) uiNewControl(uiAreaType());

	a->ah = ah;
	clickCounterReset(&(a->cc));

	a->hwnd = uiWindowsEnsureCreateControlHWND(0,
		areaClass, L"",
		WS_HSCROLL | WS_VSCROLL,
		hInstance, a,
		FALSE);

	uiWindowsFinishNewControl(a, uiArea);

	return a;
}
