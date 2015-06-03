// 8 april 2015
#include "uipriv_windows.h"

struct entry {
	uiEntry e;
	HWND hwnd;
	void (*onChanged)(uiEntry *, void *);
	void *onChangedData;
	BOOL inhibitChanged;
	void (*baseCommitDestroy)(uiControl *);
};

uiDefineControlType(uiEntry, uiTypeEntry, struct entry)

static BOOL onWM_COMMAND(uiControl *c, HWND hwnd, WORD code, LRESULT *lResult)
{
	struct entry *e = (struct entry *) c;

	if (code != EN_CHANGE)
		return FALSE;
	if (e->inhibitChanged)
		return FALSE;
	(*(e->onChanged))(uiEntry(e), e->onChangedData);
	*lResult = 0;
	return TRUE;
}

static void entryCommitDestroy(uiControl *c)
{
	struct entry *e = (struct entry *) c;

	uiWindowsUnregisterWM_COMMANDHandler(e->hwnd);
	(*(e->baseCommitDestroy))(uiControl(e));
}

static uintptr_t entryHandle(uiControl *c)
{
	struct entry *e = (struct entry *) c;

	return (uintptr_t) (e->hwnd);
}

// from http://msdn.microsoft.com/en-us/library/windows/desktop/dn742486.aspx#sizingandspacing
#define entryWidth 107 /* this is actually the shorter progress bar width, but Microsoft only indicates as wide as necessary */
#define entryHeight 14

static void entryPreferredSize(uiControl *c, uiSizing *d, intmax_t *width, intmax_t *height)
{
	*width = uiWindowsDlgUnitsToX(entryWidth, d->Sys->BaseX);
	*height = uiWindowsDlgUnitsToY(entryHeight, d->Sys->BaseY);
}

static void defaultOnChanged(uiEntry *e, void *data)
{
	// do nothing
}

static char *entryText(uiEntry *e)
{
	return uiWindowsSingleHWNDControlText(uiControl(e));
}

static void entrySetText(uiEntry *ee, const char *text)
{
	struct entry *e = (struct entry *) ee;

	// doing this raises an EN_CHANGED
	e->inhibitChanged = TRUE;
	uiWindowsSingleHWNDControlSetText(uiControl(e), text);
	e->inhibitChanged = FALSE;
}

static void entryOnChanged(uiEntry *ee, void (*f)(uiEntry *, void *), void *data)
{
	struct entry *e = (struct entry *) ee;

	e->onChanged = f;
	e->onChangedData = data;
}

static int entryReadOnly(uiEntry *ee)
{
	struct entry *e = (struct entry *) ee;

	return (getStyle(e->hwnd) & ES_READONLY) != 0;
}

static void entrySetReadOnly(uiEntry *ee, int readonly)
{
	struct entry *e = (struct entry *) ee;
	WPARAM ro;

	ro = (WPARAM) FALSE;
	if (readonly)
		ro = (WPARAM) TRUE;
	if (SendMessage(e->hwnd, EM_SETREADONLY, ro, 0) == 0)
		logLastError("error making uiEntry read-only in entrySetReadOnly()");
}

uiEntry *uiNewEntry(void)
{
	struct entry *e;

	e = (struct entry *) uiWindowsNewSingleHWNDControl(uiTypeEntry());

	e->hwnd = uiWindowsUtilCreateControlHWND(WS_EX_CLIENTEDGE,
		L"edit", L"",
		ES_AUTOHSCROLL | ES_LEFT | ES_NOHIDESEL | WS_TABSTOP,
		hInstance, NULL,
		TRUE);

	uiWindowsRegisterWM_COMMANDHandler(e->hwnd, onWM_COMMAND, uiControl(e));

	e->onChanged = defaultOnChanged;

	uiControl(e)->Handle = entryHandle;
	uiControl(e)->PreferredSize = entryPreferredSize;
	e->baseCommitDestroy = uiControl(e)->CommitDestroy;
	uiControl(e)->CommitDestroy = entryCommitDestroy;

	uiEntry(e)->Text = entryText;
	uiEntry(e)->SetText = entrySetText;
	uiEntry(e)->OnChanged = entryOnChanged;
	uiEntry(e)->ReadOnly = entryReadOnly;
	uiEntry(e)->SetReadOnly = entrySetReadOnly;

	return uiEntry(e);
}
