// 30 may 2015
#include "uipriv_windows.h"

// This is a special internal control type that handles tab pages.
// This doesn't use the container class, but rather a subclassed WC_DIALOG, as that supports tab textures properly.
// The standard property sheet control oes the same thing.
// TODO (long term) figure out how to use uiContainer with this

struct tabPage {
	uiControl c;
	HWND hwnd;
	uiControl *control;
	int margined;
};

uiDefineControlType(tabPage, tabPageType, struct tabPage)

// TODO override methods
// TODO when overriding CommitDestroy, DO NOT DESTROY THE CHILD

static uintptr_t tabPageHandle(uiControl *c)
{
	struct tabPage *t = (struct tabPage *) c;

	return (uintptr_t) (t->hwnd);
}

uiControl *newTabPage(uiControl *child)
{
	struct tabPage *t;
	HRESULT hr;

	t = (struct tabPage *) uiWindowsNewSingleHWNDControl(tabPageType());

	t->hwnd = uiWindowsUtilCreateControlHWND(WS_EX_CONTROLPARENT,
		WC_DIALOG, L"",
		0,
		hInstance, NULL,
		FALSE);

	hr = EnableThemeDialogTexture(t->hwnd, ETDT_ENABLE | ETDT_USETABTEXTURE | ETDT_ENABLETAB);
	if (hr != S_OK)
		logHRESULT("error setting tab page background in newTabPage()", hr);

	// TODO subclass hwnd to handle events

	// needs to be done here, otherwise the uiControlSetParent() below will crash
	// TODO split into separate functions
	uiControl(t)->Handle = tabPageHandle;

	t->control = child;
	uiControlSetParent(t->control, uiControl(t));

	return uiControl(t);
}
