// 10 april 2015
#include "uipriv_windows.h"

// All controls in package ui are children of a window of this class.
// This keeps everything together, makes hiding controls en masse (tab page switching, for instance) easy, and makes the overall design cleaner.
// In addition, controls that are first created or don't have a parent are considered children of the "initial parent", which is also of this class.
// This paxxxxxxxxxxxxxxxxxrent is invisible, disabled, and should not be interacted with.

// TODOs
// - wiith CTLCOLOR handler: [12:24] <ZeroOne> There's flickering between tabs
// - with CTLCOLOR handler: [12:24] <ZeroOne> And setting the button text blanked out the entire GUI until I ran my mouse over the elements / [12:25] <ZeroOne> https://dl.dropboxusercontent.com/u/15144168/GUI%20stuff.png / [12:41] <ZeroOne> https://dl.dropboxusercontent.com/u/15144168/stack.png here have another screenshot
// 	- I get this too

#define uiParentClass L"uiParentClass"

HWND initialParent;

static void paintControlBackground(HWND hwnd, HDC dc)
{
	HWND parent;
	RECT r;
	POINT pOrig;
	DWORD le;

	parent = hwnd;
	for (;;) {
		parent = GetParent(parent);
		if (parent == NULL)
			logLastError("error getting parent control of control in paintControlBackground()");
		// wine sends these messages early, yay...
		if (parent == initialParent)
			return;
		// skip groupboxes; they're (supposed to be) transparent
		if (windowClassOf(parent, L"button", NULL) != 0)
			break;
	}
	if (GetWindowRect(hwnd, &r) == 0)
		logLastError("error getting control's window rect in paintControlBackground()");
	// the above is a window rect in screen coordinates; convert to parent coordinates
	SetLastError(0);
	if (MapWindowRect(NULL, parent, &r) == 0) {
		le = GetLastError();
		SetLastError(le);		// just to be safe
		if (le != 0)
			logLastError("error getting client origin of control in paintControlBackground()");
	}
	if (SetWindowOrgEx(dc, r.left, r.top, &pOrig) == 0)
		logLastError("error moving window origin in paintControlBackground()");
	SendMessageW(parent, WM_PRINTCLIENT, (WPARAM) dc, PRF_CLIENT);
	if (SetWindowOrgEx(dc, pOrig.x, pOrig.y, NULL) == 0)
		logLastError("error resetting window origin in paintControlBackground()");
}

// from https://msdn.microsoft.com/en-us/library/windows/desktop/dn742486.aspx#sizingandspacing and https://msdn.microsoft.com/en-us/library/windows/desktop/bb226818%28v=vs.85%29.aspx
// this X value is really only for buttons but I don't see a better one :/
#define winXPadding 4
#define winYPadding 4

static void resize(uiControl *control, HWND parent, RECT r, RECT margin)
{
	uiSizing d;
	uiSizingSys sys;
	HDC dc;
	HFONT prevfont;
	TEXTMETRICW tm;
	SIZE size;

	size.cx = 0;
	size.cy = 0;
	ZeroMemory(&tm, sizeof (TEXTMETRICW));
	dc = GetDC(parent);
	if (dc == NULL)
		logLastError("error getting DC in resize()");
	prevfont = (HFONT) SelectObject(dc, hMessageFont);
	if (prevfont == NULL)
		logLastError("error loading control font into device context in resize()");
	if (GetTextMetricsW(dc, &tm) == 0)
		logLastError("error getting text metrics in resize()");
	if (GetTextExtentPoint32W(dc, L"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz", 52, &size) == 0)
		logLastError("error getting text extent point in resize()");
	sys.baseX = (int) ((size.cx / 26 + 1) / 2);
	sys.baseY = (int) tm.tmHeight;
	sys.internalLeading = tm.tmInternalLeading;
	if (SelectObject(dc, prevfont) != hMessageFont)
		logLastError("error restoring previous font into device context in resize()");
	if (ReleaseDC(parent, dc) == 0)
		logLastError("error releasing DC in resize()");
	r.left += uiDlgUnitsToX(margin.left, sys.baseX);
	r.top += uiDlgUnitsToY(margin.top, sys.baseY);
	r.right -= uiDlgUnitsToX(margin.right, sys.baseX);
	r.bottom -= uiDlgUnitsToY(margin.bottom, sys.baseY);
	d.xPadding = uiDlgUnitsToX(winXPadding, sys.baseX);
	d.yPadding = uiDlgUnitsToY(winYPadding, sys.baseY);
	d.sys = &sys;
	uiControlResize(control, r.left, r.top, r.right - r.left, r.bottom - r.top, &d);
}

struct parent {
	HWND hwnd;
	uiControl *child;
	intmax_t leftMargin;
	intmax_t topMargin;
	intmax_t rightMargin;
	intmax_t bottomMargin;
};

static LRESULT CALLBACK parentWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	uiParent *p;
	struct parent *pp;
	CREATESTRUCTW *cs = (CREATESTRUCTW *) lParam;
	HWND control;
	NMHDR *nm = (NMHDR *) lParam;
	RECT r, margin;

	p = (uiParent *) GetWindowLongPtrW(hwnd, GWLP_USERDATA);
	if (p == NULL) {
		if (uMsg == WM_NCCREATE) {
			p = (uiParent *) (cs->lpCreateParams);
			SetWindowLongPtrW(hwnd, GWLP_USERDATA, (LONG_PTR) p);
			// fall through to DefWindowProcW()
		}
		return DefWindowProcW(hwnd, uMsg, wParam, lParam);
	}
	pp = (struct parent *) (p->Internal);

	switch (uMsg) {
	case WM_NCDESTROY:
		// no need to explicitly destroy children; they're already gone by this point (and so are their data structures; they clean up after themselves)
		uiFree(p->Internal);
		uiFree(p);
		break;		// fall through to DefWindowPocW()
	case WM_COMMAND:
		// bounce back to the control in question
		// except if to the initial parent, in which case act as if the message was ignored
		control = (HWND) lParam;
		if (control != NULL && IsChild(initialParent, control) == 0)
			return SendMessageW(control, msgCOMMAND, wParam, lParam);
		break;		// fall through to DefWindowPocW()
	case WM_NOTIFY:
		// same as WM_COMMAND
		control = nm->hwndFrom;
		if (control != NULL && IsChild(initialParent, control) == 0)
			return SendMessageW(control, msgNOTIFY, wParam, lParam);
		break;		// fall through to DefWindowProcW()
	case WM_CTLCOLORSTATIC:
	case WM_CTLCOLORBTN:
/*TODO		// read-only TextFields and Textboxes are exempt
		// this is because read-only edit controls count under WM_CTLCOLORSTATIC
		if (windowClassOf((HWND) lParam, L"edit", NULL) == 0)
			if (textfieldReadOnly((HWND) lParam))
				break;		// fall through to DefWindowProcW()
*/		if (SetBkMode((HDC) wParam, TRANSPARENT) == 0)
			logLastError("error setting transparent background mode to controls in sharedWndProc()");
		paintControlBackground((HWND) lParam, (HDC) wParam);
		return (LRESULT) hollowBrush;
	case WM_WINDOWPOSCHANGED:
		if ((wp->flags & SWP_NOSIZE) != 0)
			break;
		// fall through
	case msgUpdateChild:
		if (pp->child == NULL)
			break;
		if (GetClientRect(p->hwnd, &r) == 0)
			logLastError("error getting client rect for resize in parentWndProc()");
		margin.left = pp->marginLeft;
		margin.top = pp->marginTop;
		margin.right = pp->marginRight;
		margin.bottom = pp->marginBottom;
		resize(pp->child, pp->hwnd, r, margin);
		return 0;
	}

	return DefWindowProcW(hwnd, uMsg, wParam, lParam);
}

const char *initParent(HICON hDefaultIcon, HCURSOR hDefaultCursor)
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
		return "registering parent window class";

	initialParent = CreateWindowExW(0,
		uiParentClass, L"",
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

static uintptr_t parentHandle(uiParent *p)
{
	struct parent *pp = (struct parent *) (p->Internal);

	return (uintptr_t) (p->hwnd);
}

static void parentSetChild(uiParent *p, uiChild *child)
{
	struct parent *pp = (struct parent *) (p->internal);

	pp->child = child;
	if (pp->child != NULL)
		uiControlSetParent(child, p);
}

static void parentSetMargins(uiParent *p, intmax_t left, intmax_t top, intmax_t right, intmax_t bottom)
{
	struct parent *pp = (struct parent *) (p->internal);

	pp->marginLeft = left;
	pp->marginTop = top;
	pp->marginRight = right;
	pp->marginBottom = bottom;
}

static void parentUpdate(uiParent *p)
{
	struct parent *pp = (struct parent *) (p->internal);

	SendMessageW(pp->hwnd, msgUpdateChild, 0, 0);
}

uiParent *uiNewParent(uintptr_t osParent)
{
	uiParent *p;

	p = uiNew(uiParent);
	p->Internal = uiNew(struct parent);
	p->hwnd = CreateWindowExW(0,
		xxxxxxx, L"",
		WS_CHILD | WS_VISIBLE,
		0, 0,
		0, 0,
		(HWND) osParent, NULL, hInstance, p);
	if (p->hwnd == NULL)
		logLastError("error creating uiParent window in uiNewParent()");
	p->Handle = parentHandle;
	p->SetChild = parentSetChild;
	p->SetMargins = parentSetMargins;
	p->Update = parentUpdate;
	return p;
}
