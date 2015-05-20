// 19 may 2015
#include "uipriv_windows.h"

struct progressbar {
	uiProgressBar p;
	HWND hwnd;
};

static BOOL onWM_COMMAND(uiControl *c, WORD code, LRESULT *lResult)
{
	return FALSE;
}

static BOOL onWM_NOTIFY(uiControl *c, NMHDR *nm, LRESULT *lResult)
{
	return FALSE;
}

static void onDestroy(void *data)
{
	struct progressbar *p = (struct progressbar *) data;

	uiFree(p);
}

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
	struct progressbar *pbar;
	uiWindowsMakeControlParams p;

	pbar = uiNew(struct progressbar);
	uiTyped(pbar)->Type = uiTypeProgressBar();

	p.dwExStyle = 0;
	p.lpClassName = PROGRESS_CLASSW;
	p.lpWindowName = L"";
	p.dwStyle = PBS_SMOOTH;
	p.hInstance = hInstance;
	p.lpParam = NULL;
	p.useStandardControlFont = FALSE;
	p.onWM_COMMAND = onWM_COMMAND;
	p.onWM_NOTIFY = onWM_NOTIFY;
	p.onDestroy = onDestroy;
	p.onDestroyData = pbar;
	uiWindowsMakeControl(uiControl(pbar), &p);

	pbar->hwnd = (HWND) uiControlHandle(uiControl(pbar));

	uiControl(pbar)->PreferredSize = progressbarPreferredSize;

	uiProgressBar(pbar)->SetValue = progressbarSetValue;

	return uiProgressBar(pbar);
}
