// 11 april 2015
#include "uipriv_windows.h"

struct label {
	uiLabel l;
	HWND hwnd;
};

uiDefineControlType(uiLabel, uiTypeLabel, struct label)

// via http://msdn.microsoft.com/en-us/library/windows/desktop/dn742486.aspx#sizingandspacing
#define labelHeight 8

static void labelPreferredSize(uiControl *c, uiSizing *d, intmax_t *width, intmax_t *height)
{
	struct label *l = (struct label *) c;

	*width = uiWindowsWindowTextWidth(l->hwnd);
	*height = uiWindowsDlgUnitsToY(labelHeight, d->Sys->BaseY);
}

static char *labelText(uiLabel *l)
{
	return uiWindowsControlText(uiControl(l));
}

static void labelSetText(uiLabel *l, const char *text)
{
	uiWindowsControlSetText(uiControl(l), text);
}

uiLabel *uiNewLabel(const char *text)
{
	struct label *l;
	WCHAR *wtext;

	l = (struct label *) uiWindowsNewSingleHWNDControl(uiTypeLabel());

	wtext = toUTF16(text);
	l->hwnd = uiWindowsNewSingleHWNDControl(0,
		L"static", wtext,
		// SS_LEFTNOWORDWRAP clips text past the end; SS_NOPREFIX avoids accelerator translation
		// controls are vertically aligned to the top by default (thanks Xeek in irc.freenode.net/#winapi)
		SS_LEFTNOWORDWRAP | SS_NOPREFIX,
		hInstance, NULL,
		TRUE);
	uiFree(wtext);

	uiControl(l)->PreferredSize = labelPreferredSize;

	uiLabel(l)->Text = labelText;
	uiLabel(l)->SetText = labelSetText;

	return uiLabel(l);
}
