// 20 may 2015
#include "uipriv_windows.h"

// desired behavior:
// - tab moves between /entire groups/
// - arrow keys navigate between radio buttons

struct radiobuttons {
	uiRadioButtons r;
	struct ptrArray *hwnds;
	uiControl *parent;
	uintptr_t insertAfter;			// safe to be 0 initially (either not in a container or trully the first in the z-order)
};

uiDefineControlType(uiRadioButtons, uiTypeRadioButtons, struct radiobuttons)

// TODO arrow keys don't work for changing items

// TODO this wrecks the z-order
static BOOL onWM_COMMAND(uiControl *c, HWND clicked, WORD code, LRESULT *lResult)
{
	struct radiobuttons *r = (struct radiobuttons *) c;
	WPARAM check;
	uintmax_t i;
	HWND hwnd;

	if (code != BN_CLICKED)
		return FALSE;
	for (i = 0; i < r->hwnds->len; i++) {
		hwnd = ptrArrayIndex(r->hwnds, HWND, i);
		check = BST_UNCHECKED;
		if (clicked == hwnd)
			check = BST_CHECKED;
		SendMessage(hwnd, BM_SETCHECK, check, 0);
	}
	*lResult = 0;
	return TRUE;
}

static void radiobuttonsCommitDestroy(uiControl *c)
{
	struct radiobuttons *r = (struct radiobuttons *) c;
	HWND hwnd;

	while (r->hwnds->len != 0) {
		hwnd = ptrArrayIndex(r->hwnds, HWND, 0);
		ptrArrayDelete(r->hwnds, 0);
		uiWindowsUnregisterWM_COMMANDHandler(hwnd);
		uiWindowsUtilDestroy(hwnd);
	}
	ptrArrayDestroy(r->hwnds);
}

// radio buttons have more than one handle
// if we allow deletion, the handles are not permanent
static uintptr_t radiobuttonsHandle(uiControl *c)
{
	return 0;
}

static void radiobuttonsCommitSetParent(uiControl *c, uiControl *parent)
{
	struct radiobuttons *r = (struct radiobuttons *) c;
	HWND hwnd;
	uintmax_t i;

	r->parent = parent;
	for (i = 0; i < r->hwnds->len; i++) {
		hwnd = ptrArrayIndex(r->hwnds, HWND, i);
		uiWindowsUtilSetParent(hwnd, r->parent);
	}
}

// from http://msdn.microsoft.com/en-us/library/windows/desktop/dn742486.aspx#sizingandspacing
#define radiobuttonHeight 10
// from http://msdn.microsoft.com/en-us/library/windows/desktop/bb226818%28v=vs.85%29.aspx
#define radiobuttonXFromLeftOfBoxToLeftOfLabel 12

static void radiobuttonsPreferredSize(uiControl *c, uiWindowsSizing *d, intmax_t *width, intmax_t *height)
{
	struct radiobuttons *r = (struct radiobuttons *) c;
	uintmax_t i;
	intmax_t wid, maxwid;

	maxwid = 0;
	for (i = 0; i < r->hwnds->len; i++) {
		wid = uiWindowsWindowTextWidth(ptrArrayIndex(r->hwnds, HWND, i));
		if (maxwid < wid)
			maxwid = wid;
	}
	*width = uiWindowsDlgUnitsToX(radiobuttonXFromLeftOfBoxToLeftOfLabel, d->Sys->BaseX) + maxwid;
	*height = uiWindowsDlgUnitsToY(radiobuttonHeight, d->Sys->BaseY) * r->hwnds->len;
}

static void radiobuttonsResize(uiControl *c, intmax_t x, intmax_t y, intmax_t width, intmax_t height, uiWindowsSizing *d)
{
	struct radiobuttons *r = (struct radiobuttons *) c;
	intmax_t height1;
	intmax_t h;
	uintmax_t i;
	HWND hwnd;

	height1 = uiWindowsDlgUnitsToY(radiobuttonHeight, d->Sys->BaseY);
	for (i = 0; i < r->hwnds->len; i++) {
		hwnd = ptrArrayIndex(r->hwnds, HWND, i);
		h = height1;
		if (h > height)		// clip to height
			h = height;
		moveWindow(hwnd, x, y, width, h, d);
		y += height1;
		height -= height1;
		if (height <= 0)		// clip to height
			break;
	}
}

static uiWindowsSizing *radiobuttonsSizing(uiControl *c)
{
	complain("attempt to call uiControlSizing() on uiRadioButtons %p", c);
	return NULL;
}

#define COMMIT(n, f) \
	static void radiobuttonsCommit ## n(uiControl *c) \
	{ \
		struct radiobuttons *r = (struct radiobuttons *) c; \
		uintmax_t i; \
		HWND hwnd; \
		for (i = 0; i < r->hwnds->len; i++) { \
			hwnd = ptrArrayIndex(r->hwnds, HWND, i); \
			f(hwnd); \
		} \
	}
COMMIT(Show, uiWindowsUtilShow)
COMMIT(Hide, uiWindowsUtilHide)
COMMIT(Enable, uiWindowsUtilEnable)
COMMIT(Disable, uiWindowsUtilDisable)

static uintptr_t radiobuttonsStartZOrder(uiControl *c)
{
	struct radiobuttons *r = (struct radiobuttons *) c;

	return r->insertAfter;
}

static uintptr_t radiobuttonsSetZOrder(uiControl *c, uintptr_t insertAfter)
{
	struct radiobuttons *r = (struct radiobuttons *) c;
	uintmax_t i;
	HWND hwnd;

	r->insertAfter = insertAfter;
	for (i = 0; i < r->hwnds->len; i++) {
		hwnd = ptrArrayIndex(r->hwnds, HWND, i);
		uiWindowsUtilSetZOrder(hwnd, insertAfter);
		insertAfter = (uintptr_t) hwnd;
	}
	return insertAfter;
}

static int radiobuttonsHasTabStops(uiControl *c)
{
	struct radiobuttons *r = (struct radiobuttons *) c;

	return r->hwnds->len != 0;
}

static void radiobuttonsAppend(uiRadioButtons *rr, const char *text)
{
	struct radiobuttons *r = (struct radiobuttons *) rr;
	HWND hwnd;
	WCHAR *wtext;
	HWND after;

	wtext = toUTF16(text);
	hwnd = uiWindowsEnsureCreateControlHWND(0,
		L"button", wtext,
		BS_RADIOBUTTON | WS_TABSTOP,
		hInstance, NULL,
		TRUE);
	uiFree(wtext);
	uiWindowsUtilSetParent(hwnd, r->parent);
	uiWindowsRegisterWM_COMMANDHandler(hwnd, onWM_COMMAND, uiControl(r));

	// maintain z-order
	if (r->hwnds->len == 0)		// first item
		uiWindowsUtilSetZOrder(hwnd, r->insertAfter);
	else {
		after = ptrArrayIndex(r->hwnds, HWND, r->hwnds->len - 1);
		uiWindowsUtilSetZOrder(hwnd, (uintptr_t) after);
	}

	ptrArrayAppend(r->hwnds, hwnd);
	uiControlQueueResize(uiControl(r));
}

uiRadioButtons *uiNewRadioButtons(void)
{
	struct radiobuttons *r;

	r = (struct radiobuttons *) uiNewControl(uiTypeRadioButtons());

	r->hwnds = newPtrArray();

	uiControl(r)->CommitDestroy = radiobuttonsCommitDestroy;
	uiControl(r)->Handle = radiobuttonsHandle;
	uiControl(r)->CommitSetParent = radiobuttonsCommitSetParent;
	uiControl(r)->PreferredSize = radiobuttonsPreferredSize;
	uiControl(r)->Resize = radiobuttonsResize;
	uiControl(r)->Sizing = radiobuttonsSizing;
	uiControl(r)->CommitShow = radiobuttonsCommitShow;
	uiControl(r)->CommitHide = radiobuttonsCommitHide;
	uiControl(r)->CommitEnable = radiobuttonsCommitEnable;
	uiControl(r)->CommitDisable = radiobuttonsCommitDisable;
	uiControl(r)->StartZOrder = radiobuttonsStartZOrder;
	uiControl(r)->SetZOrder = radiobuttonsSetZOrder;
	uiControl(r)->HasTabStops = radiobuttonsHasTabStops;

	uiRadioButtons(r)->Append = radiobuttonsAppend;

	return uiRadioButtons(r);
}
