// 10 april 2015
#include "uipriv_windows.h"

// All controls in package ui are children of a window of this class.
// This keeps everything together, makes hiding controls en masse (tab page switching, for instance) easy, and makes the overall design cleaner.
// In addition, controls that are first created or don't have a parent are considered children of the "initial parent", which is also of this class.
// This parent is invisible, disabled, and should not be interacted with.

// TODOs
// - wiith CTLCOLOR handler: [12:24] <ZeroOne> There's flickering between tabs
// - with CTLCOLOR handler: [12:24] <ZeroOne> And setting the button text blanked out the entire GUI until I ran my mouse over the elements / [12:25] <ZeroOne> https://dl.dropboxusercontent.com/u/15144168/GUI%20stuff.png / [12:41] <ZeroOne> https://dl.dropboxusercontent.com/u/15144168/stack.png here have another screenshot
// 	- I get this too

static void paintControlBackground(HWND hwnd, HDC dc)
{
	HWND parent;
	RECT r;
	POINT pOrig;
	int class;
	DWORD le;

	parent = hwnd;
	for (;;) {
		parent = GetParent(parent);
		if (parent == NULL)
			logLastError("error getting parent control of control in paintControlBackground()");
		// wine sends these messages early, yay...
		if (parent == initialOSContainer)
			return;
		// skip groupboxes; they're (supposed to be) transparent
		// skip uiOSContainers for reasons described below
		class = windowClassOf(parent, L"button", uiOSContainerClass, NULL);
		if (class != 0 && class != 1)
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

static void resize(uiControl *control, HWND parent, RECT r, RECT margin)
{
	r.left += uiDlgUnitsToX(margin.left, sys.baseX);
	r.top += uiDlgUnitsToY(margin.top, sys.baseY);
	r.right -= uiDlgUnitsToX(margin.right, sys.baseX);
	r.bottom -= uiDlgUnitsToY(margin.bottom, sys.baseY);
}

// TODO make this a uiOSContainer directly
struct parent {
	HWND hwnd;
	uiControl *mainControl;
	intmax_t marginLeft;
	intmax_t marginTop;
	intmax_t marginRight;
	intmax_t marginBottom;
	BOOL canDestroy;
};

static LRESULT CALLBACK parentWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	uiOSContainer *p;
	struct parent *pp;
	CREATESTRUCTW *cs = (CREATESTRUCTW *) lParam;
	HWND control;
	NMHDR *nm = (NMHDR *) lParam;
	WINDOWPOS *wp = (WINDOWPOS *) lParam;
	RECT r, margin;
	HDC dc;
	PAINTSTRUCT ps;

	// these must always be executed, even on the initial parent
	// why? http://blogs.msdn.com/b/oldnewthing/archive/2010/03/16/9979112.aspx
	switch (uMsg) {
	case WM_COMMAND:
		// bounce back to the control in question
		// except if to the initial parent, in which case act as if the message was ignored
		control = (HWND) lParam;
		if (control != NULL && IsChild(initialOSContainer, control) == 0)
			return SendMessageW(control, msgCOMMAND, wParam, lParam);
		return DefWindowProcW(hwnd, uMsg, wParam, lParam);
	case WM_NOTIFY:
		// same as WM_COMMAND
		control = nm->hwndFrom;
		if (control != NULL && IsChild(initialOSContainer, control) == 0)
			return SendMessageW(control, msgNOTIFY, wParam, lParam);
		return DefWindowProcW(hwnd, uMsg, wParam, lParam);
	case WM_CTLCOLORSTATIC:
	case WM_CTLCOLORBTN:
/*TODO		// read-only TextFields and Textboxes are exempt
		// this is because read-only edit controls count under WM_CTLCOLORSTATIC
		if (windowClassOf((HWND) lParam, L"edit", NULL) == 0)
			if (textfieldReadOnly((HWND) lParam))
				return DefWindowProcW(hwnd, uMsg, wParam, lParam);
*/		if (SetBkMode((HDC) wParam, TRANSPARENT) == 0)
			logLastError("error setting transparent background mode to controls in parentWndProc()");
		paintControlBackground((HWND) lParam, (HDC) wParam);
		return (LRESULT) hollowBrush;
	}

	// these are only executed on actual parents
	p = (uiOSContainer *) GetWindowLongPtrW(hwnd, GWLP_USERDATA);
	if (p == NULL) {
		if (uMsg == WM_NCCREATE) {
			p = (uiOSContainer *) (cs->lpCreateParams);
			// this will be NULL for the initial parent; that's what we want
			SetWindowLongPtrW(hwnd, GWLP_USERDATA, (LONG_PTR) p);
			// fall through to DefWindowProcW()
		}
		// this is the return the initial parent will always use
		return DefWindowProcW(hwnd, uMsg, wParam, lParam);
	}
	pp = (struct parent *) (p->Internal);
	switch (uMsg) {
	case WM_NCDESTROY:
		if (!pp->canDestroy)
			complain("attempt to destroy uiOSContainer at %p before uiOSContainerDestroy()", p);
		uiFree(p->Internal);
		uiFree(p);
		break;		// fall through to DefWindowPocW()
	case WM_PAINT:
		dc = BeginPaint(pp->hwnd, &ps);
		if (dc == NULL)
			logLastError("TODO write this");
		paintControlBackground(pp->hwnd, dc);
		EndPaint(pp->hwnd, &ps);
		return 0;
	// don't implement WM_PRINTCLIENT
	// if paintControlBackground() asks us to draw our background, we have to ask our parent to draw its, which causes weird origin issues
	// instead, we skip uiOSContainers in paintControlBackground()
	}

	return DefWindowProcW(hwnd, uMsg, wParam, lParam);
}

static void parentDestroy(uiOSContainer *pp)
{
	struct parent *p = (struct parent *) (pp->Internal);

	// first destroy the main control, if any
	if (p->mainControl != NULL) {
		// we have to do this before we can destroy controls
		uiControlSetOSContainer(p->mainControl, NULL);
		uiControlDestroy(p->mainControl);
		p->mainControl = NULL;
	}
	// then mark that we are ready to destroy
	p->canDestroy = TRUE;
	// and finally destroy
	if (DestroyWindow(p->hwnd) == 0)
		logLastError("error destroying uiOSContainer window in parentDestroy()");
}

static uintptr_t parentHandle(uiOSContainer *p)
{
	struct parent *pp = (struct parent *) (p->Internal);

	return (uintptr_t) (pp->hwnd);
}

static void parentSetMainControl(uiOSContainer *pp, uiControl *mainControl)
{
	struct parent *p = (struct parent *) (pp->Internal);

	if (p->mainControl != NULL)
		uiControlSetOSContainer(p->mainControl, NULL);
	p->mainControl = mainControl;
	if (p->mainControl != NULL)
		uiControlSetOSContainer(p->mainControl, pp);
}

static void parentSetMargins(uiOSContainer *p, intmax_t left, intmax_t top, intmax_t right, intmax_t bottom)
{
	struct parent *pp = (struct parent *) (p->Internal);

	pp->marginLeft = left;
	pp->marginTop = top;
	pp->marginRight = right;
	pp->marginBottom = bottom;
}

static void parentUpdate(uiOSContainer *p)
{
	struct parent *pp = (struct parent *) (p->Internal);

	SendMessageW(pp->hwnd, msgUpdateChild, 0, 0);
}

uiOSContainer *uiNewOSContainer(uintptr_t osOSContainer)
{
	uiOSContainer *p;
	struct parent *pp;

	p = uiNew(uiOSContainer);
	p->Internal = uiNew(struct parent);		// set now in case the parent class window procedure uses it
	pp = (struct parent *) (p->Internal);
	pp->hwnd = CreateWindowExW(0,
		uiOSContainerClass, L"",
		WS_CHILD | WS_VISIBLE,
		0, 0,
		0, 0,
		(HWND) osOSContainer, NULL, hInstance, p);
	if (pp->hwnd == NULL)
		logLastError("error creating uiOSContainer window in uiNewOSContainer()");

	p->Destroy = parentDestroy;
	p->Handle = parentHandle;
	p->SetMainControl = parentSetMainControl;
	p->SetMargins = parentSetMargins;
	p->Update = parentUpdate;

	return p;
}
