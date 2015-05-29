// 19 may 2015
#include "uipriv_windows.h"

struct progressbar {
	uiProgressBar p;
	HWND hwnd;
};

uiDefineControlType(uiProgressBar, uiTypeProgressBar, struct progressbar)

// via http://msdn.microsoft.com/en-us/library/windows/desktop/dn742486.aspx#sizingandspacing
#define pbarWidth 237
#define pbarHeight 8

static void progressbarPreferredSize(uiControl *c, uiSizing *d, intmax_t *width, intmax_t *height)
{
	struct label *l = (struct label *) c;

	*width = uiWindowsDlgUnitsToX(pbarWidth, d->Sys->BaseX);
	*height = uiWindowsDlgUnitsToY(pbarHeight, d->Sys->BaseY);
}

// TODO bypass Aero animations
static void progressbarSetValue(uiProgressBar *pp, int value)
{
	struct progressbar *p = (struct progressbar *) pp;

	SendMessageW(p->hwnd, PBM_SETPOS, (WPARAM) value, 0);
}

uiProgressBar *uiNewProgressBar(void)
{
	struct progressbar *p;

	p = (struct progressbar *) uiWindowsNewSingleHWNDControl(uiTypeProgressBar());

	p->hwnd = uiWindowsNewSingleHWNDControl(0,
		PROGRESS_CLASSW, L"",
		PBS_SMOOTH,
		hInstance, NULL,
		FALSE);

	uiControl(p)->PreferredSize = progressbarPreferredSize;

	uiProgressBar(p)->SetValue = progressbarSetValue;

	return uiProgressBar(p);
}
