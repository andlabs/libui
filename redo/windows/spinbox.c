// 8 april 2015
#include "uipriv_windows.h"

struct uiSpinbox {
	uiWindowsControl c;
	HWND hwnd;
	HWND updown;
	void (*onChanged)(uiSpinbox *, void *);
	void *onChangedData;
	BOOL inhibitChanged;
};

static void onDestroy(uiSpinbox *);

uiWindowsDefineControlWithOnDestroy(
	uiSpinbox,							// type name
	uiSpinboxType,						// type function
	onDestroy(this);						// on destroy
)

// utility functions

static intmax_t value(uiSpinbox *s)
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
	uiSpinbox *s = (uiSpinbox *) c;
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
	(*(s->onChanged))(s, s->onChangedData);
	return TRUE;
}

static void onDestroy(uiSpinbox *s)
{
	uiWindowsUnregisterWM_COMMANDHandler(s->hwnd);
	if (DestroyWindow(s->updown) == 0)
		logLastError("error destroying updown in spinboxCommitDestroy()");
}

// TODO set spinbox parent

// from http://msdn.microsoft.com/en-us/library/windows/desktop/dn742486.aspx#sizingandspacing
#define entryWidth 107 /* this is actually the shorter progress bar width, but Microsoft only indicates as wide as necessary */
#define entryHeight 14

static void minimumSize(uiWindowsControl *c, uiWindowsSizing *d, intmax_t *width, intmax_t *height)
{
	*width = uiWindowsDlgUnitsToX(entryWidth, d->BaseX);
	*height = uiWindowsDlgUnitsToY(entryHeight, d->BaseY);
}

// an up-down control will only properly position itself the first time
// stupidly, there are no messages to force a size calculation, nor can I seem to reset the buddy window to force a new position
// alas, we have to make a new up/down control each time :(
static void recreateUpDown(uiSpinbox *s)
{
/* TODO
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
*/
}

static void spinboxRelayout(uiWindowsControl *c, intmax_t x, intmax_t y, intmax_t width, intmax_t height)
{
/* TODO
	uiSpinbox *s = uiSpinbox(c);

	moveWindow(s->hwnd, x, y, width, height, d);
	recreateUpDown(s);
*/
}

/* TODO
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
*/

static void defaultOnChanged(uiSpinbox *s, void *data)
{
	// do nothing
}

intmax_t uiSpinboxValue(uiSpinbox *s)
{
	return value(s);
}

void uiSpinboxSetValue(uiSpinbox *s, intmax_t value)
{
	s->inhibitChanged = TRUE;
	SendMessageW(s->updown, UDM_SETPOS32, 0, (LPARAM) value);
	s->inhibitChanged = FALSE;
}

void uiSpinboxOnChanged(uiSpinbox *s, void (*f)(uiSpinbox *, void *), void *data)
{
	s->onChanged = f;
	s->onChangedData = data;
}

uiSpinbox *uiNewSpinbox(intmax_t min, intmax_t max)
{
	uiSpinbox *s;

	if (min >= max)
		complain("error: min >= max in uiNewSpinbox()");

	s = (uiSpinbox *) uiNewControl(uiSpinboxType());

	s->hwnd = uiWindowsEnsureCreateControlHWND(WS_EX_CLIENTEDGE,
		L"edit", L"",
		// don't use ES_NUMBER; it doesn't allow typing in a leading -
		ES_AUTOHSCROLL | ES_LEFT | ES_NOHIDESEL | WS_TABSTOP,
		hInstance, NULL,
		TRUE);

	uiWindowsRegisterWM_COMMANDHandler(s->hwnd, onWM_COMMAND, uiControl(s));
	uiSpinboxOnChanged(s, defaultOnChanged, NULL);

	recreateUpDown(s);
	s->inhibitChanged = TRUE;
	SendMessageW(s->updown, UDM_SETRANGE32, (WPARAM) min, (LPARAM) max);
	SendMessageW(s->updown, UDM_SETPOS32, 0, (LPARAM) min);
	s->inhibitChanged = FALSE;

	uiWindowsFinishNewControl(s, uiSpinbox);
	uiWindowsControl(s)->Relayout = spinboxRelayout;

	return s;
}
