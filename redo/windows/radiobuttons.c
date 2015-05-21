// 20 may 2015
#include "uipriv_windows.h"

struct radiobuttons {
	uiRadioButtons r;
	struct ptrArray *hwnds;
	uiControl *parent;
};

// TODO make sure this is the correct way to check radiobuttons
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

static void radiobuttonsDestroy(uiControl *c)
{
	// TODO
}

// radio buttons have more than one handle
// if we allow deletion, the handles are not permanent
static uintptr_t radiobuttonsHandle(uiControl *c)
{
	return 0;
}

static uiControl *radiobuttonsParent(uiControl *c)
{
	struct radiobuttons *r = (struct radiobuttons *) c;

	return r->parent;
}

static void radiobuttonsSetParent(uiControl *c, uiControl *parent)
{
	struct radiobuttons *r = (struct radiobuttons *) c;
	HWND newParentHWND;
	HWND hwnd;
	uintmax_t i;

	r->parent = parent;
	newParentHWND = utilWindow;
	if (r->parent != NULL)
		newParentHWND = (HWND) uiControlHandle(r->parent);
	for (i = 0; i < r->hwnds->len; i++) {
		hwnd = ptrArrayIndex(r->hwnds, HWND, i);
		if (SetParent(hwnd, newParentHWND) == NULL)
			logLastError("error setting radio button parent in radiobuttonsSetParent()");
	}
}

// TODO
// from http://msdn.microsoft.com/en-us/library/windows/desktop/dn742486.aspx#sizingandspacing
#define checkboxHeight 10
// from http://msdn.microsoft.com/en-us/library/windows/desktop/bb226818%28v=vs.85%29.aspx
#define checkboxXFromLeftOfBoxToLeftOfLabel 12

static void radiobuttonsPreferredSize(uiControl *c, uiSizing *d, intmax_t *width, intmax_t *height)
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
	*width = uiWindowsDlgUnitsToX(checkboxXFromLeftOfBoxToLeftOfLabel, d->Sys->BaseX) + maxwid;
	*height = uiWindowsDlgUnitsToY(checkboxHeight, d->Sys->BaseY) * r->hwnds->len;
}

// TODO clip to height
static void radiobuttonsResize(uiControl *c, intmax_t x, intmax_t y, intmax_t width, intmax_t height, uiSizing *d)
{
	struct radiobuttons *r = (struct radiobuttons *) c;
	intmax_t height1;
	uintmax_t i;
	HWND hwnd;

	height1 = uiWindowsDlgUnitsToY(checkboxHeight, d->Sys->BaseY);
	for (i = 0; i < r->hwnds->len; i++) {
		hwnd = ptrArrayIndex(r->hwnds, HWND, i);
		moveWindow(hwnd, x, y, width, height1, d);
		y += height1;
	}
}

static void radiobuttonsQueueResize(uiControl *c)
{
	queueResize(c);
}

static uiSizing *radiobuttonsSizing(uiControl *c)
{
	// TODO
	return NULL;
}

static int radiobuttonsContainerVisible(uiControl *c)
{
	// TODO
	return 1;
}

static void radiobuttonsSHED(uiControl *c)
{
	// TODO
}

static void radiobuttonsSysFunc(uiControl *c, uiControlSysFuncParams *p)
{
	// TODO
}

static int radiobuttonsStartZOrder(uiControl *c, uiControlSysFuncParams *p)
{
	// TODO
	return 0;
}

static void radiobuttonsAppend(uiRadioButtons *rr, const char *text)
{
	struct radiobuttons *r = (struct radiobuttons *) rr;
	HWND hwnd, parent;
	WCHAR *wtext;

	wtext = toUTF16(text);
	parent = utilWindow;
	if (r->parent != NULL)
		parent = (HWND) uiControlHandle(r->parent);
	hwnd = CreateWindowExW(0,
		L"button", wtext,
		BS_RADIOBUTTON | WS_TABSTOP | WS_CHILD | WS_VISIBLE,
		0, 0,
		100, 100,
		parent, NULL, hInstance, NULL);
	if (hwnd == NULL)
		logLastError("error creating radio button in radiobuttonsAppend()");
	uiWindowsRegisterWM_COMMANDHandler(hwnd, onWM_COMMAND, uiControl(r));
	ptrArrayAppend(r->hwnds, hwnd);
	uiControlQueueResize(uiControl(r));
}

uiRadioButtons *uiNewRadioButtons(void)
{
	struct radiobuttons *r;

	r = uiNew(struct radiobuttons);
	uiTyped(r)->Type = uiTypeRadioButtons();

	r->hwnds = newPtrArray();

	uiControl(r)->Destroy = radiobuttonsDestroy;
	uiControl(r)->Handle = radiobuttonsHandle;
	uiControl(r)->Parent = radiobuttonsParent;
	uiControl(r)->SetParent = radiobuttonsSetParent;
	uiControl(r)->PreferredSize = radiobuttonsPreferredSize;
	uiControl(r)->Resize = radiobuttonsResize;
	uiControl(r)->QueueResize = radiobuttonsQueueResize;
	uiControl(r)->Sizing = radiobuttonsSizing;
	uiControl(r)->ContainerVisible = radiobuttonsContainerVisible;
	uiControl(r)->Show = radiobuttonsSHED;
	uiControl(r)->Hide = radiobuttonsSHED;
	uiControl(r)->ContainerShow = radiobuttonsSHED;
	uiControl(r)->ContainerHide = radiobuttonsSHED;
	uiControl(r)->Enable = radiobuttonsSHED;
	uiControl(r)->Disable = radiobuttonsSHED;
	uiControl(r)->ContainerEnable = radiobuttonsSHED;
	uiControl(r)->ContainerDisable = radiobuttonsSHED;
	uiControl(r)->SysFunc = radiobuttonsSysFunc;
	uiControl(r)->StartZOrder = radiobuttonsStartZOrder;

	uiRadioButtons(r)->Append = radiobuttonsAppend;

	return uiRadioButtons(r);
}
