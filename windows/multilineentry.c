// 8 april 2015
#include "uipriv_windows.h"

// TODO there's alpha darkening of text going on; something is up in our parent logic
// TODO resizing collapses newlines

struct uiMultilineEntry {
	uiWindowsControl c;
	HWND hwnd;
	void (*onChanged)(uiMultilineEntry *, void *);
	void *onChangedData;
	BOOL inhibitChanged;
};

uiWindowsDefineControlWithOnDestroy(
	uiMultilineEntry,						// type name
	uiMultilineEntryType,					// type function
	uiWindowsUnregisterWM_COMMANDHandler(this->hwnd);	// on destroy
)

static BOOL onWM_COMMAND(uiControl *c, HWND hwnd, WORD code, LRESULT *lResult)
{
	uiMultilineEntry *e = uiMultilineEntry(c);

	if (code != EN_CHANGE)
		return FALSE;
	if (e->inhibitChanged)
		return FALSE;
	(*(e->onChanged))(e, e->onChangedData);
	*lResult = 0;
	return TRUE;
}

// from http://msdn.microsoft.com/en-us/library/windows/desktop/dn742486.aspx#sizingandspacing
#define entryWidth 107 /* this is actually the shorter progress bar width, but Microsoft only indicates as wide as necessary */
// TODO change this for multiline text boxes
#define entryHeight 14

static void minimumSize(uiWindowsControl *c, uiWindowsSizing *d, intmax_t *width, intmax_t *height)
{
	*width = uiWindowsDlgUnitsToX(entryWidth, d->BaseX);
	*height = uiWindowsDlgUnitsToY(entryHeight, d->BaseY);
}

static void defaultOnChanged(uiMultilineEntry *e, void *data)
{
	// do nothing
}

char *uiMultilineEntryText(uiMultilineEntry *e)
{
	return uiWindowsUtilText(e->hwnd);
}

void uiMultilineEntrySetText(uiMultilineEntry *e, const char *text)
{
	// doing this raises an EN_CHANGED
	e->inhibitChanged = TRUE;
	uiWindowsUtilSetText(e->hwnd, text);
	e->inhibitChanged = FALSE;
	// don't queue the control for resize; entry sizes are independent of their contents
}

void uiMultilineEntryAppend(uiMultilineEntry *e, const char *text)
{
	LRESULT n;
	WCHAR *wtext;

	// TODO does doing this raise EN_CHANGED?
	// TODO preserve selection? caret? what if caret used to be at end?
	// TODO scroll to bottom?
	n = SendMessageW(e->hwnd, WM_GETTEXTLENGTH, 0, 0);
	SendMessageW(e->hwnd, EM_SETSEL, n, n);
	wtext = toUTF16(text);
	SendMessageW(e->hwnd, EM_REPLACESEL, FALSE, (LPARAM) wtext);
	uiFree(wtext);
}

void uiMultilineEntryOnChanged(uiMultilineEntry *e, void (*f)(uiMultilineEntry *, void *), void *data)
{
	e->onChanged = f;
	e->onChangedData = data;
}

int uiMultilineEntryReadOnly(uiMultilineEntry *e)
{
	return (getStyle(e->hwnd) & ES_READONLY) != 0;
}

void uiMultilineEntrySetReadOnly(uiMultilineEntry *e, int readonly)
{
	WPARAM ro;

	ro = (WPARAM) FALSE;
	if (readonly)
		ro = (WPARAM) TRUE;
	if (SendMessage(e->hwnd, EM_SETREADONLY, ro, 0) == 0)
		logLastError("error making uiMultilineEntry read-only in uiMultilineEntrySetReadOnly()");
}

uiMultilineEntry *uiNewMultilineEntry(void)
{
	uiMultilineEntry *e;

	e = (uiMultilineEntry *) uiNewControl(uiMultilineEntryType());

	e->hwnd = uiWindowsEnsureCreateControlHWND(WS_EX_CLIENTEDGE,
		L"edit", L"",
		ES_AUTOVSCROLL | ES_LEFT | ES_MULTILINE | ES_NOHIDESEL | ES_WANTRETURN | WS_TABSTOP | WS_VSCROLL,
		hInstance, NULL,
		TRUE);

	uiWindowsRegisterWM_COMMANDHandler(e->hwnd, onWM_COMMAND, uiControl(e));
	uiMultilineEntryOnChanged(e, defaultOnChanged, NULL);

	uiWindowsFinishNewControl(e, uiMultilineEntry);

	return e;
}
