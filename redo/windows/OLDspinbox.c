// 8 april 2015
#include "uipriv_windows.h"

struct spinbox {
	uiSpinbox s;
	HWND hwnd;
	HWND updown;
	void (*onChanged)(uiSpinbox *, void *);
	void *onChangedData;
	BOOL inhibitChanged;
};

uiDefineControlType(uiSpinbox, uiTypeSpinbox, struct spinbox)

// utility functions

static intmax_t value(struct spinbox *s)
{
	BOOL neededCap = FALSE;
	LRESULT val;

	// This verifies the value put in, capping it automatically.
	// We don't need to worry about checking for an error; that flag should really be called "did we have to cap?".
	// We DO need to set the value in case of a cap though.
	val = SendMessageW(s->updown, UDM_GETPOS32, 0, (LPARAM) (&neededCap));
	if (neededCap) {
		s->inhibitChanged = TRUE;
		SendMessageW(s->updown, UDM_SETPOS32, 0, (LPARAM) val);
		s->inhibitChanged = FALSE;
	}
	return (intmax_t) val;
}

// control implementation

static BOOL onWM_COMMAND(uiControl *c, HWND hwnd, WORD code, LRESULT *lResult)
{
	struct spinbox *s = (struct spinbox *) c;
	WCHAR *wtext;

	if (code != EN_CHANGE)
		return FALSE;
	if (s->inhibitChanged)
		return FALSE;
	// We want to allow typing negative numbers; the natural way to do so is to start with a -.
	// However, if we just have the code below, the up-down will catch the bare - and reject it.
	// Let's fix that.
	// This won't handle leading spaces, but spaces aren't allowed *anyway*.
	wtext = windowText(s->hwnd);
	if (wcscmp(wtext, L"-") == 0) {
		uiFree(wtext);
		return TRUE;
	}
	// value() does the work for us
	value(s);
	(*(s->onChanged))(uiSpinbox(s), s->onChangedData);
	return TRUE;
}

static void spinboxCommitDestroy(uiControl *c)
{
	struct spinbox *s = (struct spinbox *) c;

	uiWindowsUnregisterWM_COMMANDHandler(s->hwnd);
	if (DestroyWindow(s->updown) == 0)
		logLastError("error destroying updown in spinboxCommitDestroy()");
	uiWindowsUtilDestroy(s->hwnd);
}

// the edit control is the one to return here
// we can't return the updown because it gets recreated on resize
static uintptr_t spinboxHandle(uiControl *c)
{
	struct spinbox *s = (struct spinbox *) c;

	return (uintptr_t) (s->hwnd);
}

static void spinboxCommitSetParent(uiControl *c, uiControl *parent)
{
	struct spinbox *s = (struct spinbox *) c;

	uiWindowsUtilSetParent(s->hwnd, parent);
	uiWindowsUtilSetParent(s->updown, parent);
}

// from http://msdn.microsoft.com/en-us/library/windows/desktop/dn742486.aspx#sizingandspacing
#define entryWidth 107 /* this is actually the shorter progress bar width, but Microsoft only indicates as wide as necessary */
#define entryHeight 14

static void spinboxPreferredSize(uiControl *c, uiWindowsSizing *d, intmax_t *width, intmax_t *height)
{
	*width = uiWindowsDlgUnitsToX(entryWidth, d->Sys->BaseX);
	*height = uiWindowsDlgUnitsToY(entryHeight, d->Sys->BaseY);
}

// an up-down control will only properly position itself the first time
// stupidly, there are no messages to force a size calculation, nor can I seem to reset the buddy window to force a new position
// alas, we have to make a new up/down control each time :(
static void recreateUpDown(struct spinbox *s)
{
	HWND parent;
	BOOL preserve = FALSE;
	intmax_t current;
	// Microsoft's commctrl.h says to use this type
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
		// up-down control should not be a tab stop
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
	// preserve the z-order
	uiWindowsUtilSetZOrder(s->updown, (uintptr_t) (s->hwnd));
	if (uiControlContainerVisible(uiControl(s)))
		uiWindowsUtilShow(s->updown);
	s->inhibitChanged = FALSE;
}

static void spinboxResize(uiControl *c, intmax_t x, intmax_t y, intmax_t width, intmax_t height, uiWindowsSizing *d)
{
	struct spinbox *s = (struct spinbox *) c;

	moveWindow(s->hwnd, x, y, width, height, d);
	recreateUpDown(s);
}

static uiWindowsSizing *spinboxSizing(uiControl *c)
{
	complain("attempt to call uiControlSizing() on uiSpinbox %p", c);
	return NULL;
}

#define COMMIT(n, f) \
	static void spinboxCommit ## n(uiControl *c) \
	{ \
		struct spinbox *s = (struct spinbox *) c; \
		f(s->hwnd); \
		f(s->updown); \
	}
COMMIT(Show, uiWindowsUtilShow)
COMMIT(Hide, uiWindowsUtilHide)
COMMIT(Enable, uiWindowsUtilEnable)
COMMIT(Disable, uiWindowsUtilDisable)

static uintptr_t spinboxStartZOrder(uiControl *c)
{
	struct spinbox *s = (struct spinbox *) c;

	return uiWindowsUtilStartZOrder(s->hwnd);
}

static uintptr_t spinboxSetZOrder(uiControl *c, uintptr_t insertAfter)
{
	struct spinbox *s = (struct spinbox *) c;

	uiWindowsUtilSetZOrder(s->hwnd, insertAfter);
	uiWindowsUtilSetZOrder(s->updown, (uintptr_t) (s->hwnd));
	return (uintptr_t) (s->updown);
}

static int spinboxHasTabStops(uiControl *c)
{
	struct spinbox *s = (struct spinbox *) c;

	return uiWindowsUtilHasTabStops(s->hwnd);
}

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

	if (min >= max)
		complain("error: min >= max in uiNewSpinbox()");

	s = (struct spinbox *) uiNewControl(uiTypeSpinbox());

	s->hwnd = uiWindowsEnsureCreateControlHWND(WS_EX_CLIENTEDGE,
		L"edit", L"",
		// don't use ES_NUMBER; it doesn't allow typing in a leading -
		ES_AUTOHSCROLL | ES_LEFT | ES_NOHIDESEL | WS_TABSTOP,
		hInstance, NULL,
		TRUE);

	uiWindowsRegisterWM_COMMANDHandler(s->hwnd, onWM_COMMAND, uiControl(s));

	recreateUpDown(s);
	s->inhibitChanged = TRUE;
	SendMessageW(s->updown, UDM_SETRANGE32, (WPARAM) min, (LPARAM) max);
	SendMessageW(s->updown, UDM_SETPOS32, 0, (LPARAM) min);
	s->inhibitChanged = FALSE;

	s->onChanged = defaultOnChanged;

	uiControl(s)->Handle = spinboxHandle;
	uiControl(s)->PreferredSize = spinboxPreferredSize;
	uiControl(s)->Resize = spinboxResize;
	uiControl(s)->Sizing = spinboxSizing;
	uiControl(s)->CommitDestroy = spinboxCommitDestroy;
	uiControl(s)->CommitSetParent = spinboxCommitSetParent;
	uiControl(s)->CommitShow = spinboxCommitShow;
	uiControl(s)->CommitHide = spinboxCommitHide;
	uiControl(s)->CommitEnable = spinboxCommitEnable;
	uiControl(s)->CommitDisable = spinboxCommitDisable;
	uiControl(s)->StartZOrder = spinboxStartZOrder;
	uiControl(s)->SetZOrder = spinboxSetZOrder;
	uiControl(s)->HasTabStops = spinboxHasTabStops;

	uiSpinbox(s)->Value = spinboxValue;
	uiSpinbox(s)->SetValue = spinboxSetValue;
	uiSpinbox(s)->OnChanged = spinboxOnChanged;

	return uiSpinbox(s);
}
