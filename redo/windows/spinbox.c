// 8 april 2015
#include "uipriv_windows.h"

struct spinbox {
	uiSpinbox s;
	HWND hwnd;
	HWND updown;
	void (*baseResize)(uiControl *, intmax_t, intmax_t, intmax_t, intmax_t, uiSizing *);
	void (*onChanged)(uiSpinbox *, void *);
	void *onChangedData;
	BOOL inhibitChanged;
};

// utility functions

static intmax_t value(struct spinbox *s)
{
	BOOL neededCap = FALSE;
	LRESULT val;

	// This verifies the value put in, capping it automatically.
	// We don't need to worry about checking for an error; that flag should really be called "did we have to cap?".
	// We DO need to set the value in case of a cap though.
	// TODO wine only?
	val = SendMessageW(s->updown, UDM_GETPOS32, 0, (LPARAM) (&neededCap));
	if (neededCap) {
		s->inhibitChanged = TRUE;
		SendMessageW(s->updown, UDM_SETPOS32, 0, (LPARAM) val);
		s->inhibitChanged = FALSE;
	}
	return (intmax_t) val;
}

// control implementation

static BOOL onWM_COMMAND(uiControl *c, WORD code, LRESULT *lResult)
{
	struct spinbox *s = (struct spinbox *) c;

	if (code != EN_CHANGE)
		return FALSE;
	if (s->inhibitChanged)
		return FALSE;
	// value() does the work for us
	value(s);
	(*(s->onChanged))(uiSpinbox(s), s->onChangedData);
	return TRUE;
}

static BOOL onWM_HSCROLL(uiControl *c, WORD code, LRESULT *lResult)
{
	return FALSE;
}

static void onDestroy(void *data)
{
	struct spinbox *s = (struct spinbox *) data;

	uiWindowsUnregisterWM_COMMANDHandler(s->hwnd);
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
	BOOL preserve = FALSE;
	intmax_t current;
	// wine uses this type
	INT min, max;

	parent = GetAncestor(s->hwnd, GA_PARENT);
	if (s->updown != NULL) {
		preserve = TRUE;
		current = value(s);
		SendMessageW(s->updown, UDM_GETRANGE32, (WPARAM) (&min), (LPARAM) (&max));
		if (DestroyWindow(s->updown) == 0)
			logLastError("error destroying old updown in recreateUpDown()");
	}
	s->inhibitChanged = TRUE;
	s->updown = CreateWindowExW(0,
		UPDOWN_CLASSW, L"",
		// no WS_VISIBLE; we set visibility ourselves
		// TODO tab stop?
		// TODO maintain Z-order
		WS_CHILD | UDS_ALIGNRIGHT | UDS_ARROWKEYS | UDS_HOTTRACK | UDS_NOTHOUSANDS | UDS_SETBUDDYINT,
		// this is important; it's necessary for autosizing to work
		0, 0, 0, 0,
		parent, NULL, hInstance, NULL);
	if (s->updown == NULL)
		logLastError("error creating updown in recreateUpDown()");
	SendMessageW(s->updown, UDM_SETBUDDY, (WPARAM) (s->hwnd), 0);
	if (preserve) {
		SendMessageW(s->updown, UDM_SETRANGE32, (WPARAM) min, (LPARAM) max);
		SendMessageW(s->updown, UDM_SETPOS32, 0, (LPARAM) current);
	}
	if (uiControlContainerVisible(uiControl(s)))
		ShowWindow(s->updown, SW_SHOW);
	s->inhibitChanged = FALSE;
}

static void spinboxResize(uiControl *c, intmax_t x, intmax_t y, intmax_t width, intmax_t height, uiSizing *d)
{
	struct spinbox *s = (struct spinbox *) c;

	(*(s->baseResize))(uiControl(s), x, y, width, height, d);
	recreateUpDown(s);
}

// TODO does it go here relative of other things?
static void defaultOnChanged(uiSpinbox *s, void *data)
{
	// do nothing
}

static intmax_t spinboxValue(uiSpinbox *ss)
{
	struct spinbox *s = (struct spinbox *) ss;

	return value(s);
}

static void spinboxSetValue(uiSpinbox *ss, intmax_t value)
{
	struct spinbox *s = (struct spinbox *) ss;

	s->inhibitChanged = TRUE;
	SendMessageW(s->updown, UDM_SETPOS32, 0, (LPARAM) value);
	s->inhibitChanged = FALSE;
}

static void spinboxOnChanged(uiSpinbox *ss, void (*f)(uiSpinbox *, void *), void *data)
{
	struct spinbox *s = (struct spinbox *) ss;

	s->onChanged = f;
	s->onChangedData = data;
}

uiSpinbox *uiNewSpinbox(intmax_t min, intmax_t max)
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
	p.onWM_HSCROLL = onWM_HSCROLL;
	p.onDestroy = onDestroy;
	p.onDestroyData = s;
	uiWindowsMakeControl(uiControl(s), &p);

	s->hwnd = (HWND) uiControlHandle(uiControl(s));
	uiWindowsRegisterWM_COMMANDHandler(s->hwnd, onWM_COMMAND, uiControl(s));

	recreateUpDown(s);
	s->inhibitChanged = TRUE;
	SendMessageW(s->updown, UDM_SETRANGE32, (WPARAM) min, (LPARAM) max);
	SendMessageW(s->updown, UDM_SETPOS32, 0, (LPARAM) min);
	s->inhibitChanged = FALSE;

	s->onChanged = defaultOnChanged;

	uiControl(s)->PreferredSize = spinboxPreferredSize;
	s->baseResize = uiControl(s)->Resize;
	uiControl(s)->Resize = spinboxResize;

	uiSpinbox(s)->Value = spinboxValue;
	uiSpinbox(s)->SetValue = spinboxSetValue;
	uiSpinbox(s)->OnChanged = spinboxOnChanged;

	return uiSpinbox(s);
}
