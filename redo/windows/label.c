// 11 april 2015
#include "uipriv_windows.h"

struct label {
	uiLabel l;
	HWND hwnd;
};

uiDefineControlType(uiLabel, uiTypeLabel, struct label)

static uintptr_t labelHandle(uiControl *c)
{
	struct label *l = (struct label *) c;

	return (uintptr_t) (l->hwnd);
}

// via http://msdn.microsoft.com/en-us/library/windows/desktop/dn742486.aspx#sizingandspacing
#define labelHeight 8

static void labelPreferredSize(uiControl *c, uiSizing *d, intmax_t *width, intmax_t *height)
{
	struct label *l = (struct label *) c;

	*width = uiWindowsWindowTextWidth(l->hwnd);
	*height = uiWindowsDlgUnitsToY(labelHeight, d->Sys->BaseY);
}

static char *labelText(uiLabel *ll)
{
	struct label *l = (struct label *) ll;

	return uiWindowsUtilText(l->hwnd);
}

static void labelSetText(uiLabel *ll, const char *text)
{
	struct label *l = (struct label *) ll;

	uiWindowsUtilSetText(l->hwnd, text);
	// changing the text might necessitate a change in the label's size
	uiControlQueueResize(uiControl(l));
}

uiLabel *uiNewLabel(const char *text)
{
	struct label *l;
	WCHAR *wtext;

	l = (struct label *) uiWindowsNewSingleHWNDControl(uiTypeLabel());

	wtext = toUTF16(text);
	l->hwnd = uiWindowsUtilCreateControlHWND(0,
		L"static", wtext,
		// SS_LEFTNOWORDWRAP clips text past the end; SS_NOPREFIX avoids accelerator translation
		// controls are vertically aligned to the top by default (thanks Xeek in irc.freenode.net/#winapi)
		SS_LEFTNOWORDWRAP | SS_NOPREFIX,
		hInstance, NULL,
		TRUE);
	uiFree(wtext);

	uiControl(l)->Handle = labelHandle;
	uiControl(l)->PreferredSize = labelPreferredSize;

	uiLabel(l)->Text = labelText;
	uiLabel(l)->SetText = labelSetText;

	return uiLabel(l);
}
