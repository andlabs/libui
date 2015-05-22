// 22 may 2015
#include "uipriv_windows.h"

struct datetimepicker {
	uiDateTimePicker d;
	HWND hwnd;
};

static void onDestroy(void *data)
{
	struct datetimepicker *d = (struct datetimepicker *) data;

	uiFree(d);
}

// TODO
// TODO use DTM_GETIDEALSIZE when making Vista-only
// from http://msdn.microsoft.com/en-us/library/windows/desktop/dn742486.aspx#sizingandspacing
#define entryWidth 107 /* this is actually the shorter progress bar width, but Microsoft only indicates as wide as necessary */
#define entryHeight 14

static void datetimepickerPreferredSize(uiControl *c, uiSizing *d, intmax_t *width, intmax_t *height)
{
	*width = uiWindowsDlgUnitsToX(entryWidth, d->Sys->BaseX);
	*height = uiWindowsDlgUnitsToY(entryHeight, d->Sys->BaseY);
}

uiDateTimePicker *finishNewDateTimePicker(DWORD style)
{
	struct datetimepicker *d;
	uiWindowsMakeControlParams p;

	d = uiNew(struct datetimepicker);
	uiTyped(d)->Type = uiTypeDateTimePicker();

	p.dwExStyle = 0;		// TODO client edge?
	p.lpClassName = DATETIMEPICK_CLASSW;
	p.lpWindowName = L"";
	p.dwStyle = style | WS_TABSTOP;
	p.hInstance = hInstance;
	p.lpParam = NULL;
	p.useStandardControlFont = TRUE;
	p.onDestroy = onDestroy;
	p.onDestroyData = d;
	uiWindowsMakeControl(uiControl(d), &p);

	d->hwnd = (HWND) uiControlHandle(uiControl(d));

	uiControl(d)->PreferredSize = datetimepickerPreferredSize;

	return uiDateTimePicker(d);
}

// TODO
uiDateTimePicker *uiNewDateTimePicker(void)
{
	return finishNewDateTimePicker(0);
}

uiDateTimePicker *uiNewDatePicker(void)
{
	return finishNewDateTimePicker(DTS_SHORTDATECENTURYFORMAT);
}

uiDateTimePicker *uiNewTimePicker(void)
{
	return finishNewDateTimePicker(DTS_TIMEFORMAT);
}
