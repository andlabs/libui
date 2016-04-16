// 20 may 2015
#include "uipriv_windows.h"

// references:
// - http://stackoverflow.com/q/2892703/3646475
// - https://msdn.microsoft.com/en-us/library/windows/desktop/dn742405%28v=vs.85%29.aspx

struct uiSeparator {
	uiWindowsControl c;
	HWND hwnd;
};

uiWindowsDefineControl(
	uiSeparator,							// type name
	uiSeparatorType						// type function
)

// via https://msdn.microsoft.com/en-us/library/windows/desktop/bb226818%28v=vs.85%29.aspx
#define separatorHeight 1

static void minimumSize(uiWindowsControl *c, uiWindowsSizing *d, intmax_t *width, intmax_t *height)
{
	*width = 1;		// TODO
	*height = uiWindowsDlgUnitsToY(separatorHeight, d->BaseY);
}

uiSeparator *uiNewHorizontalSeparator(void)
{
	uiSeparator *s;

	s = (uiSeparator *) uiNewControl(uiSeparatorType());

	s->hwnd = uiWindowsEnsureCreateControlHWND(0,
		L"static", L"",
		SS_ETCHEDHORZ,
		hInstance, NULL,
		TRUE);

	uiWindowsFinishNewControl(s, uiSeparator);

	return s;
}
