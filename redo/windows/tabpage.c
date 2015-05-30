// 30 may 2015
#include "uipriv_windows.h"

// This is a special internal control type that handles tab pages.
// This doesn't use the container class, but rather a subclassed WC_DIALOG, as that supports tab textures properly.
// The standard property sheet control oes the same thing.
// TODO (long term) figure out how to use uiContainer with this

// TODO verify that the tab page texture doesn't end too soon

struct tabPage {
	uiControl c;
	HWND hwnd;
	uiControl *control;		// TODO rename child
	int margined;
	void (*baseResize)(uiControl *, intmax_t, intmax_t, intmax_t, intmax_t, uiSizing *);
};

uiDefineControlType(tabPage, tabPageType, struct tabPage)

// TODO override methods
// TODO when overriding CommitDestroy, DO NOT DESTROY THE CHILD

static uintptr_t tabPageHandle(uiControl *c)
{
	struct tabPage *t = (struct tabPage *) c;

	return (uintptr_t) (t->hwnd);
}

// from http://msdn.microsoft.com/en-us/library/windows/desktop/bb226818%28v=vs.85%29.aspx
#define tabMargin 7

static void tabPageResize(uiControl *c, intmax_t x, intmax_t y, intmax_t width, intmax_t height, uiSizing *d)
{
	struct tabPage *t = (struct tabPage *) c;
	RECT r;
	uiSizing *dchild;

	(*(t->baseResize))(uiControl(t), x, y, width, height, d);

	dchild = uiControlSizing(uiControl(t));

	if (GetClientRect(t->hwnd, &r) == 0)
		logLastError("error getting tab page client rect in tabPageResize()");
	if (t->margined) {
		r.left += uiWindowsDlgUnitsToX(tabMargin, d->Sys->BaseX);
		r.top += uiWindowsDlgUnitsToY(tabMargin, d->Sys->BaseY);
		r.right -= uiWindowsDlgUnitsToX(tabMargin, d->Sys->BaseX);
		r.bottom -= uiWindowsDlgUnitsToY(tabMargin, d->Sys->BaseY);
	}
	// this rect is in client coordinates; we need toplevel window coordinates
	mapWindowRect(t->hwnd, dchild->Sys->CoordFrom, &r);

	uiControlResize(t->control, r.left, r.top, r.right - r.left, r.bottom - r.top, dchild);

	uiFreeSizing(dchild);
}

// dummy dialog function; see below for details
INT_PTR CALLBACK dlgproc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (uMsg == WM_INITDIALOG)
		return TRUE;
	return FALSE;
}

uiControl *newTabPage(uiControl *child)
{
	struct tabPage *t;
	HRESULT hr;

	t = (struct tabPage *) uiWindowsNewSingleHWNDControl(tabPageType());

	// unfortunately this needs to be a proper dialog for EnableThemeDialogTexture() to work; CreateWindowExW() won't suffice
	t->hwnd = CreateDialogW(hInstance, MAKEINTRESOURCE(rcTabPageDialog),
		utilWindow, dlgproc);
	if (t->hwnd == NULL)
		logLastError("error creating tab page in newTabPage()");

	// TODO figure out why this is not working
	hr = EnableThemeDialogTexture(t->hwnd, ETDT_ENABLE | ETDT_USETABTEXTURE | ETDT_ENABLETAB);
	if (hr != S_OK)
		logHRESULT("error setting tab page background in newTabPage()", hr);

	// TODO subclass hwnd to handle events

	// needs to be done here, otherwise the uiControlSetParent() below will crash
	// TODO split into separate functions
	uiControl(t)->Handle = tabPageHandle;

	t->control = child;
	uiControlSetParent(t->control, uiControl(t));

	t->baseResize = uiControl(t)->Resize;
	uiControl(t)->Resize = tabPageResize;

	return uiControl(t);
}
