// 8 april 2015
#include "uipriv_windows.h"

struct entry {
	uiEntry e;
	HWND hwnd;
	void (*onChanged)(uiEntry *, void *);
	void *onChangedData;
};

static BOOL onWM_COMMAND(uiControl *c, WORD code, LRESULT *lResult)
{
	struct entry *e = (struct entry *) c;

	if (code != EN_CHANGE)
		return FALSE;
	(*(e->onChanged))(uiEntry(e), e->onChangedData);
	// TODO EN_CHANGE return value
	*lResult = 0;
	return TRUE;
}

static BOOL onWM_NOTIFY(uiControl *c, NMHDR *nm, LRESULT *lResult)
{
	return FALSE;
}

static void onDestroy(void *data)
{
	struct entry *e = (struct entry *) data;

	uiFree(e);
}

// from http://msdn.microsoft.com/en-us/library/windows/desktop/dn742486.aspx#sizingandspacing
#define entryWidth 107 /* this is actually the shorter progress bar width, but Microsoft only indicates as wide as necessary */
#define entryHeight 14

static void entryPreferredSize(uiControl *c, uiSizing *d, intmax_t *width, intmax_t *height)
{
	*width = uiDlgUnitsToX(entryWidth, d->sys->baseX);
	*height = uiDlgUnitsToY(entryHeight, d->sys->baseY);
}

static void defaultOnChanged(uiEntry *e, void *data)
{
	// do nothing
}

static char *entryText(uiEntry *e)
{
	return uiWindowsControlText(uiControl(e));
}

static void entrySetText(uiEntry *e, const char *text)
{
	uiWindowsControlSetText(uiControl(e), text);
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

	p.dwExStyle = WS_EX_CLIENTEDGE;
	p.lpClassName = L"edit";
	p.lpWindowName = L"";
	p.dwStyle = ES_AUTOHSCROLL | ES_LEFT | ES_NOHIDESEL | WS_TABSTOP;
	p.hInstance = hInstance;
	p.useStandardControlFont = TRUE;
	p.onWM_COMMAND = onWM_COMMAND;
	p.onWM_NOTIFY = onWM_NOTIFY;
	p.onDestroy = onDestroy;
	p.onDestroyData = e;
	uiWindowsMakeControl(uiControl(e), &p);

	e->hwnd = (HWND) uiControlHandle(uiControl(e));

	e->onChanged = defaultOnChanged;

	uiControl(e)->PreferredSize = entryPreferredSize;

	uiEntry(e)->Text = entryText;
	uiEntry(e)->SetText = entrySetText;
	uiEntry(e)->OnChanged = entryOnChanged;
	uiEntry(e)->ReadOnly = entryReadOnly;
	uiEntry(e)->SetReadOnly = entrySetReadOnly;

	return uiEntry(e);
}
