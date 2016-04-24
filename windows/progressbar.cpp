// 19 may 2015
#include "uipriv_windows.hpp"

struct uiProgressBar {
	uiWindowsControl c;
	HWND hwnd;
};

uiWindowsDefineControl(
	uiProgressBar							// type name
)

// via http://msdn.microsoft.com/en-us/library/windows/desktop/dn742486.aspx#sizingandspacing
#define pbarWidth 237
#define pbarHeight 8

static void minimumSize(uiWindowsControl *c, uiWindowsSizing *d, intmax_t *width, intmax_t *height)
{
	*width = uiWindowsDlgUnitsToX(pbarWidth, d->BaseX);
	*height = uiWindowsDlgUnitsToY(pbarHeight, d->BaseY);
}

// unfortunately, as of Vista progress bars have a forced animation on increase
// we have to set the progress bar to value + 1 and decrease it back to value if we want an "instant" change
// see http://stackoverflow.com/questions/2217688/windows-7-aero-theme-progress-bar-bug
// it's not ideal/perfect, but it will have to do
void uiProgressBarSetValue(uiProgressBar *p, int value)
{
	if (value < 0 || value > 100)
		complain("value %d out of range in uiProgressBarSetValue()", value);
	if (value == 100) {			// because we can't 101
		SendMessageW(p->hwnd, PBM_SETRANGE32, 0, 101);
		SendMessageW(p->hwnd, PBM_SETPOS, 101, 0);
		SendMessageW(p->hwnd, PBM_SETPOS, 100, 0);
		SendMessageW(p->hwnd, PBM_SETRANGE32, 0, 100);
		return;
	}
	SendMessageW(p->hwnd, PBM_SETPOS, (WPARAM) (value + 1), 0);
	SendMessageW(p->hwnd, PBM_SETPOS, (WPARAM) value, 0);
}

uiProgressBar *uiNewProgressBar(void)
{
	uiProgressBar *p;

	p = (uiProgressBar *) uiNewControl(uiProgressBarType());

	p->hwnd = uiWindowsEnsureCreateControlHWND(0,
		PROGRESS_CLASSW, L"",
		PBS_SMOOTH,
		hInstance, NULL,
		FALSE);

	uiWindowsFinishNewControl(p, uiProgressBar);

	return p;
}
