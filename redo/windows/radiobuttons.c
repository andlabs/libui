// 20 may 2015
#include "uipriv_windows.h"

// desired behavior:
// - tab moves between /entire groups/
// - arrow keys navigate between radio buttons

struct uiRadioButtons {
	uiWindowsControl c;
	HWND hwnd;				// of the container
	struct ptrArray *hwnds;		// of the buttons
};

static void onDestroy(uiRadioButtons *);

uiWindowsDefineControlWithOnDestroy(
	uiRadioButtons,						// type name
	uiRadioButtonsType,						// type function
	onDestroy(this);						// on destroy
)

// TODO arrow keys don't work for changing items

// TODO this wrecks the z-order
static BOOL onWM_COMMAND(uiControl *c, HWND clicked, WORD code, LRESULT *lResult)
{
	uiRadioButtons *r = uiRadioButtons(c);
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

static void onDestroy(uiRadioButtons *r)
{
	HWND hwnd;

	while (r->hwnds->len != 0) {
		hwnd = ptrArrayIndex(r->hwnds, HWND, 0);
		ptrArrayDelete(r->hwnds, 0);
		uiWindowsUnregisterWM_COMMANDHandler(hwnd);
		uiWindowsUtilDestroy(hwnd);
	}
	ptrArrayDestroy(r->hwnds);
}

// from http://msdn.microsoft.com/en-us/library/windows/desktop/dn742486.aspx#sizingandspacing
#define radiobuttonHeight 10
// from http://msdn.microsoft.com/en-us/library/windows/desktop/bb226818%28v=vs.85%29.aspx
#define radiobuttonXFromLeftOfBoxToLeftOfLabel 12

static void minimumSize(uiWindowsControl *c, uiWindowsSizing *d, intmax_t *width, intmax_t *height)
{
	uiRadioButtons *r = uiRadioButtons(c);
	uintmax_t i;
	intmax_t wid, maxwid;

	maxwid = 0;
	for (i = 0; i < r->hwnds->len; i++) {
		wid = uiWindowsWindowTextWidth(ptrArrayIndex(r->hwnds, HWND, i));
		if (maxwid < wid)
			maxwid = wid;
	}
	*width = uiWindowsDlgUnitsToX(radiobuttonXFromLeftOfBoxToLeftOfLabel, d->BaseX) + maxwid;
	*height = uiWindowsDlgUnitsToY(radiobuttonHeight, d->BaseY) * r->hwnds->len;
}

static void radiobuttonsRelayout(uiWindowsControl *c, intmax_t x, intmax_t y, intmax_t width, intmax_t height)
{
/* TODO
	struct radiobuttons *r = (struct radiobuttons *) c;
	intmax_t height1;
	intmax_t h;
	uintmax_t i;
	HWND hwnd;

	// TODO resize the main hwnd

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
*/
}

// TODO container update state

/* TODO
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
*/

void uiRadioButtonsAppend(uiRadioButtons *r, const char *text)
{
/* TODO
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
*/
}

uiRadioButtons *uiNewRadioButtons(void)
{
	uiRadioButtons *r;

	r = (uiRadioButtons *) uiNewControl(uiRadioButtonsType());

	r->hwnd = newContainer();

	r->hwnds = newPtrArray();

	uiWindowsFinishNewControl(r, uiRadioButtons);
	uiWindowsControl(r)->Relayout = radiobuttonsRelayout;

	return r;
}
