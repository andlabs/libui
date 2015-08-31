// 11 april 2015
#include "uipriv_windows.h"

struct uiLabel {
	uiWindowsControl c;
	HWND hwnd;
};

uiWindowsDefineControl(
	uiLabel,								// type name
	uiLabelType							// type function
)

// via http://msdn.microsoft.com/en-us/library/windows/desktop/dn742486.aspx#sizingandspacing
#define labelHeight 8

static void minimumSize(uiControl *c, uiWindowsSizing *d, intmax_t *width, intmax_t *height)
{
	uiLabel *l = uiLabel(c);

	*width = uiWindowsWindowTextWidth(l->hwnd);
	*height = uiWindowsDlgUnitsToY(labelHeight, d->BaseY);
}

char *uiLabelText(uiLabel *l)
{
	return uiWindowsUtilText(l->hwnd);
}

void uiLabelSetText(uiLabel *l, const char *text)
{
	uiWindowsUtilSetText(l->hwnd, text);
	// changing the text might necessitate a change in the label's size
	uiControlQueueResize(uiControl(l));
}

uiLabel *uiNewLabel(const char *text)
{
	uiLabel *l;
	WCHAR *wtext;

	l = (uiLabel *) uiNewControl(uiLabelType());

	wtext = toUTF16(text);
	l->hwnd = uiWindowsEnsureCreateControlHWND(0,
		L"static", wtext,
		// SS_LEFTNOWORDWRAP clips text past the end; SS_NOPREFIX avoids accelerator translation
		// controls are vertically aligned to the top by default (thanks Xeek in irc.freenode.net/#winapi)
		SS_LEFTNOWORDWRAP | SS_NOPREFIX,
		hInstance, NULL,
		TRUE);
	uiFree(wtext);

	uiWindowsFinishNewControl(l, uiLabel);

	return l;
}
