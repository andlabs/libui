// 22 may 2015
#include "uipriv_windows.h"

struct datetimepicker {
	uiDateTimePicker d;
	HWND hwnd;
};

uiDefineControlType(uiDateTimePicker, uiTypeDateTimePicker, struct datetimepicker)

// utility functions

#define GLI(what, buf, n) GetLocaleInfoEx(LOCALE_NAME_USER_DEFAULT, what, buf, n)

// Windows has no combined date/time prebuilt constant; we have to build the format string ourselves
static void setDateTimeFormat(HWND hwnd)
{
	WCHAR *date, *time, *datetime;
	int ndate, ntime;
	int n;

	// TODO verify that this always returns a century year
	ndate = GLI(LOCALE_SSHORTDATE, NULL, 0);
	if (ndate == 0)
		logLastError("error getting date string length in setDateTimeFormat()");
	date = (WCHAR *) uiAlloc(ndate * sizeof (WCHAR), "WCHAR[]");
	if (GLI(LOCALE_SSHORTDATE, date, ndate) == 0)
		logLastError("error geting date string in setDateTimeFormat()");

	ntime = GLI(LOCALE_STIMEFORMAT, NULL, 0);
	if (ndate == 0)
		logLastError("error getting time string length in setDateTimeFormat()");
	time = (WCHAR *) uiAlloc(ntime * sizeof (WCHAR), "WCHAR[]");
	if (GLI(LOCALE_STIMEFORMAT, time, ntime) == 0)
		logLastError("error geting time string in setDateTimeFormat()");

	n = _scwprintf(L"%s %s", date, time);
	datetime = (WCHAR *) uiAlloc((n + 1) * sizeof (WCHAR), "WCHAR[]");
	snwprintf(datetime, n + 1, L"%s %s", date, time);
	if (SendMessageW(hwnd, DTM_SETFORMAT, 0, (LPARAM) datetime) == 0)
		logLastError("error applying format string to date/time picker in setDateTimeFormat()");

	uiFree(datetime);
	uiFree(time);
	uiFree(date);
}

// control implementation

static uintptr_t datetimepickerHandle(uiControl *c)
{
	struct datetimepicker *d = (struct datetimepicker *) c;

	return (uintptr_t) (d->hwnd);
}

// TODO
// TODO DTM_GETIDEALSIZE results in something too big
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

	d = (struct datetimepicker *) uiWindowsNewSingleHWNDControl(uiTypeDateTimePicker());

	d->hwnd = uiWindowsUtilCreateControlHWND(WS_EX_CLIENTEDGE,
		DATETIMEPICK_CLASSW, L"",
		style | WS_TABSTOP,
		hInstance, NULL,
		TRUE);

	uiControl(d)->Handle = datetimepickerHandle;
	uiControl(d)->PreferredSize = datetimepickerPreferredSize;

	return uiDateTimePicker(d);
}

// Fortunately, because the date/time picker (on Vista, at least) does NOT respond to date/time format changes with its standard format styles, we only need to do this when creating the control as well.
// TODO really we need to send any WM_WININICHANGE messages back...
uiDateTimePicker *uiNewDateTimePicker(void)
{
	uiDateTimePicker *dtp;
	struct datetimepicker *d;

	dtp = finishNewDateTimePicker(0);
	d = (struct datetimepicker *) dtp;
	setDateTimeFormat(d->hwnd);
	return dtp;
}

uiDateTimePicker *uiNewDatePicker(void)
{
	return finishNewDateTimePicker(DTS_SHORTDATECENTURYFORMAT);
}

uiDateTimePicker *uiNewTimePicker(void)
{
	return finishNewDateTimePicker(DTS_TIMEFORMAT);
}
