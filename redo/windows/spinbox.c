// 8 april 2015
#include "uipriv_windows.h"

struct spinbox {
	uiSpinbox s;
	HWND hwnd;
	HWND updown;
	void (*baseResize)(uiControl *, intmax_t, intmax_t, intmax_t, intmax_t, uiSizing *);
};

static BOOL onWM_COMMAND(uiControl *c, WORD code, LRESULT *lResult)
{
	struct spinbox *s = (struct spinbox *) c;
	BOOL neededCap = FALSE;
	LRESULT val;

	if (code != EN_CHANGE)
		return FALSE;
	// This verifies the value put in, capping it automatically.
	// We don't need to worry about checking for an error; that flag should really be called "did we have to cap?".
	// We DO need to set the value in case of a cap though.
	// TODO if we have change events, we need to make sure doing this doesn't trigger a double event
	val = SendMessageW(s->updown, UDM_GETPOS32, 0, (LPARAM) (&neededCap));
	if (neededCap)
		SendMessageW(s->updown, UDM_SETPOS32, 0, (LPARAM) val);
	return FALSE;
}

static BOOL onWM_NOTIFY(uiControl *c, NMHDR *nm, LRESULT *lResult)
{
	return FALSE;
}

static void onDestroy(void *data)
{
	struct spinbox *s = (struct spinbox *) data;

	uiFree(s);
}

// from http://msdn.microsoft.com/en-us/library/windows/desktop/dn742486.aspx#sizingandspacing
#define entryWidth 107 /* this is actually the shorter progress bar width, but Microsoft only indicates as wide as necessary */
#define entryHeight 14

static void spinboxPreferredSize(uiControl *c, uiSizing *d, intmax_t *width, intmax_t *height)
{
	*width = uiWindowsDlgUnitsToX(entryWidth, d->Sys->BaseX);
	*height = uiWindowsDlgUnitsToY(entryHeight, d->Sys->BaseY);
}

// an up-down control will only properly position itself the first time
// stupidly, there are no messages to force a size calculation, nor can I seem to reset the buddy window to force a new position
// alas, we have to make a new up/down control each time :(
// TODO will we need to store a copy of the current position and range for this?
static void recreateUpDown(struct spinbox *s)
{
	HWND parent;

	parent = GetAncestor(s->hwnd, GA_PARENT);
	if (s->updown != NULL)
		if (DestroyWindow(s->updown) == 0)
			logLastError("error destroying old updown in recreateUpDown()");
	s->updown = CreateWindowExW(0,
		UPDOWN_CLASSW, L"",
		// no WS_VISIBLE; we set visibility ourselves
		WS_CHILD | UDS_ALIGNRIGHT | UDS_ARROWKEYS | UDS_HOTTRACK | UDS_NOTHOUSANDS | UDS_SETBUDDYINT,
		// this is important; it's necessary for autosizing to work
		0, 0, 0, 0,
		parent, NULL, hInstance, NULL);
	if (s->updown == NULL)
		logLastError("error creating updown in recreateUpDown()");
	SendMessageW(s->updown, UDM_SETBUDDY, (WPARAM) (s->hwnd), 0);
	// TODO
	SendMessageW(s->updown, UDM_SETRANGE32, 0, 100);
	SendMessageW(s->updown, UDM_SETPOS32, 0, 0);
	if (uiControlContainerVisible(uiControl(s)))
		ShowWindow(s->updown, SW_SHOW);
}

static void spinboxResize(uiControl *c, intmax_t x, intmax_t y, intmax_t width, intmax_t height, uiSizing *d)
{
	struct spinbox *s = (struct spinbox *) c;

	(*(s->baseResize))(uiControl(s), x, y, width, height, d);
	recreateUpDown(s);
}

uiSpinbox *uiNewSpinbox(void)
{
	struct spinbox *s;
	uiWindowsMakeControlParams p;

	s = uiNew(struct spinbox);
	uiTyped(s)->Type = uiTypeSpinbox();

	p.dwExStyle = WS_EX_CLIENTEDGE;
	p.lpClassName = L"edit";
	p.lpWindowName = L"";
	p.dwStyle = ES_AUTOHSCROLL | ES_LEFT | ES_NOHIDESEL | ES_NUMBER | WS_TABSTOP;
	p.hInstance = hInstance;
	p.lpParam = NULL;
	p.useStandardControlFont = TRUE;
	p.onWM_COMMAND = onWM_COMMAND;
	p.onWM_NOTIFY = onWM_NOTIFY;
	p.onDestroy = onDestroy;
	p.onDestroyData = s;
	uiWindowsMakeControl(uiControl(s), &p);

	s->hwnd = (HWND) uiControlHandle(uiControl(s));

	recreateUpDown(s);

	uiControl(s)->PreferredSize = spinboxPreferredSize;
	s->baseResize = uiControl(s)->Resize;
	uiControl(s)->Resize = spinboxResize;

	return uiSpinbox(s);
}
