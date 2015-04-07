// 7 april 2015
#include "uipriv_windows.h"

struct button {
	uiControl *c;
	void (*onClicked)(uiControl *, void *);
	void *onClickedData;
};

#define B(x) ((struct button *) (x))

static BOOL onWM_COMMAND(uiControl *c, WPARAM wParam, LPARAM lParam, void *data, LRESULT *lResult)
{
	if (HIWORD(wParam) != BN_CLICKED)
		return FALSE;
	(*(B(data)->onClicked))(c, B(data)->onClickedData);
	*lResult = 0;
	return TRUE;
}

static BOOL onWM_NOTIFY(uiControl *c, WPARAM wParam, LPARAM lParam, void *data, LRESULT *lResult)
{
	return FALSE;
}

static void preferredSize(uiControl *c, int baseX, int baseY, LONG internalLeading, intmax_t *width, intmax_t *height)
{
	// TODO
}

static void defaultOnClicked(uiControl *c, void *data)
{
	// do nothing
}

uiControl *uiNewButton(const char *text)
{
	struct button *b;
	uiWindowsNewControlParams p;
	WCHAR *wtext;
	HWND hwnd;

	b = uiNew(struct button);

	p.dwExStyle = 0;
	p.lpClassName = L"button";
	p.dwStyle = BS_PUSHBUTTON;
	p.hInstance = hInstance;
	p.onWM_COMMAND = onWM_COMMAND;
	p.onWM_NOTIFY = onWM_NOTIFY;
	p.onCommandNotifyData = b;
	p.preferredSize = preferredSize;
	p.data = b;
	b->c = uiWindowsNewControl(&p);

	hwnd = (HWND) uiControlHandle(b->c);
	wtext = toUTF16(text);
	if (SetWindowTextW(hwnd, wtext) == 0)
		logLastError("error setting button text in uiNewButton()");
	uiFree(wtext);
	SendMessageW(hwnd, WM_SETFONT, (WPARAM) hMessageFont, (LPARAM) TRUE);

	b->onClicked = defaultOnClicked;

	return b->c;
}

// TODO text

void uiButtonOnClicked(uiControl *c, void (*f)(uiControl *, void *), void *data)
{
	struct button *b;

	b = (struct button *) uiWindowsControlData(c);
	b->onClicked = f;
	b->onClickedData = data;
}
