// 8 april 2015
#include "uipriv_windows.h"

struct entry {
	uiEntry e;
	HWND hwnd;
	void (*onChanged)(uiEntry *, void *);
	void *onChangedData;
	BOOL inhibitChanged;
};

static BOOL onWM_COMMAND(uiControl *c, WORD code, LRESULT *lResult)
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

static BOOL onWM_HSCROLL(uiControl *c, WORD code, LRESULT *lResult)
{
	return FALSE;
}

static void onDestroy(void *data)
{
	struct entry *e = (struct entry *) data;

	uiWindowsUnregisterWM_COMMANDHandler(e->hwnd);
	uiFree(e);
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
	return uiWindowsControlText(uiControl(e));
}

static void entrySetText(uiEntry *ee, const char *text)
{
	struct entry *e = (struct entry *) ee;

	// doing this raises an EN_CHANGED
	e->inhibitChanged = TRUE;
	uiWindowsControlSetText(uiControl(e), text);
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
	uiWindowsMakeControlParams p;

	e = uiNew(struct entry);
	uiTyped(e)->Type = uiTypeEntry();

	p.dwExStyle = WS_EX_CLIENTEDGE;
	p.lpClassName = L"edit";
	p.lpWindowName = L"";
	p.dwStyle = ES_AUTOHSCROLL | ES_LEFT | ES_NOHIDESEL | WS_TABSTOP;
	p.hInstance = hInstance;
	p.lpParam = NULL;
	p.useStandardControlFont = TRUE;
	p.onWM_HSCROLL = onWM_HSCROLL;
	p.onDestroy = onDestroy;
	p.onDestroyData = e;
	uiWindowsMakeControl(uiControl(e), &p);

	e->hwnd = (HWND) uiControlHandle(uiControl(e));
	uiWindowsRegisterWM_COMMANDHandler(e->hwnd, onWM_COMMAND, uiControl(e));

	e->onChanged = defaultOnChanged;

	uiControl(e)->PreferredSize = entryPreferredSize;

	uiEntry(e)->Text = entryText;
	uiEntry(e)->SetText = entrySetText;
	uiEntry(e)->OnChanged = entryOnChanged;
	uiEntry(e)->ReadOnly = entryReadOnly;
	uiEntry(e)->SetReadOnly = entrySetReadOnly;

	return uiEntry(e);
}
