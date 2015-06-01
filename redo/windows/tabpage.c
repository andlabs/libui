// 30 may 2015
#include "uipriv_windows.h"

// This is a special internal control type that handles tab pages.
// This doesn't use the container class, but rather a subclassed WC_DIALOG, as that supports tab textures properly.
// The standard property sheet control oes the same thing.

// TODO tell wine that dialogs respond to WM_PRINTCLIENT on xp+

struct tabPage {
	uiControl c;
	HWND hwnd;
	uiControl *child;
	int margined;
	void (*baseResize)(uiControl *, intmax_t, intmax_t, intmax_t, intmax_t, uiSizing *);
};

uiDefineControlType(tabPage, tabPageType, struct tabPage)

// don't override CommitDestroy(); we destroy the child with a separate function

static uintptr_t tabPageHandle(uiControl *c)
{
	struct tabPage *t = (struct tabPage *) c;

	return (uintptr_t) (t->hwnd);
}

// from http://msdn.microsoft.com/en-us/library/windows/desktop/bb226818%28v=vs.85%29.aspx
#define tabMargin 7

static void tabPagePreferredSize(uiControl *c, uiSizing *d, intmax_t *width, intmax_t *height)
{
	struct tabPage *t = (struct tabPage *) c;

	*width = 0;
	*height = 0;
	if (t->child != NULL)
		uiControlPreferredSize(t->child, d, width, height);
	if (t->margined) {
		*width += 2 * uiWindowsDlgUnitsToX(tabMargin, d->Sys->BaseX);
		*height += 2 * uiWindowsDlgUnitsToY(tabMargin, d->Sys->BaseY);
	}
}

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

	uiControlResize(t->child, r.left, r.top, r.right - r.left, r.bottom - r.top, dchild);

	uiFreeSizing(dchild);
}

static void tabPageContainerUpdateState(uiControl *c)
{
	struct tabPage *t = (struct tabPage *) c;

	if (t->child != NULL)
		uiControlUpdateState(t->child);
}

// dummy dialog function; see below for details
static INT_PTR CALLBACK dlgproc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
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

	hr = EnableThemeDialogTexture(t->hwnd, ETDT_ENABLE | ETDT_USETABTEXTURE | ETDT_ENABLETAB);
	if (hr != S_OK)
		logHRESULT("error setting tab page background in newTabPage()", hr);

	// TODO subclass hwnd to handle events

	// needs to be done here, otherwise the uiControlSetParent() below will crash
	// TODO split into separate functions
	uiControl(t)->Handle = tabPageHandle;

	t->child = child;
	uiControlSetParent(t->child, uiControl(t));

	uiControl(t)->PreferredSize = tabPagePreferredSize;
	t->baseResize = uiControl(t)->Resize;
	uiControl(t)->Resize = tabPageResize;
	uiControl(t)->ContainerUpdateState = tabPageContainerUpdateState;

	return uiControl(t);
}

int tabPageMargined(uiControl *c)
{
	struct tabPage *t = (struct tabPage *) c;

	return t->margined;
}

void tabPageSetMargined(uiControl *c, int margined)
{
	struct tabPage *t = (struct tabPage *) c;

	t->margined = margined;
}

void tabPageDestroyChild(uiControl *c)
{
	struct tabPage *t = (struct tabPage *) c;

	uiControlSetParent(t->child, NULL);
	uiControlDestroy(t->child);
	t->child = NULL;
}
