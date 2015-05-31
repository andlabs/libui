// 20 may 2015
#include "uipriv_windows.h"

// references:
// - http://stackoverflow.com/questions/2892703/how-do-i-draw-separators
// - https://msdn.microsoft.com/en-us/library/windows/desktop/dn742405%28v=vs.85%29.aspx

struct separator {
	uiSeparator s;
	HWND hwnd;
};

uiDefineControlType(uiSeparator, uiTypeSeparator, struct separator)

// via http://msdn.microsoft.com/en-us/library/windows/desktop/dn742486.aspx#sizingandspacing
// TODO
// TODO DPI independence?

static uintptr_t separatorHandle(uiControl *c)
{
	struct separator *s = (struct separator *) c;

	return (uintptr_t) (s->hwnd);
}

static void separatorPreferredSize(uiControl *c, uiSizing *d, intmax_t *width, intmax_t *height)
{
	*width = 1;		// TODO
	*height = 2;		// what Spy++ says Windows XP's Explorer uses
}

uiSeparator *uiNewHorizontalSeparator(void)
{
	struct separator *s;

	s = (struct separator *) uiWindowsNewSingleHWNDControl(uiTypeSeparator());

	s->hwnd = uiWindowsUtilCreateControlHWND(0,
		L"static", L"",
		SS_ETCHEDHORZ,
		hInstance, NULL,
		TRUE);

	uiControl(s)->Handle = separatorHandle;
	uiControl(s)->PreferredSize = separatorPreferredSize;

	return uiSeparator(s);
}
