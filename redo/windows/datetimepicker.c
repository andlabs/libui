// 22 may 2015
#include "uipriv_windows.h"

// TODO does the control update builtin date and time formats with a locale change?

struct datetimepicker {
	uiDateTimePicker d;
	HWND hwnd;
};

uiDefineControlType(uiDateTimePicker, uiTypeDateTimePicker, struct datetimepicker)

static uintptr_t datetimepickerHandle(uiControl *c)
{
	struct datetimepicker *d = (struct datetimepicker *) c;

	return (uintptr_t) (d->hwnd);
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

uiDateTimePicker *finishNewDateTimePicker(DWORD style, WCHAR *format)
{
	struct datetimepicker *d;

	d = (struct datetimepicker *) uiWindowsNewSingleHWNDControl(uiTypeDateTimePicker());

	d->hwnd = uiWindowsUtilCreateControlHWND(0,		// TODO client edge?
		DATETIMEPICK_CLASSW, L"",
		style | WS_TABSTOP,
		hInstance, NULL,
		TRUE);

	if (format != NULL)
		if (SendMessageW(d->hwnd, DTM_SETFORMAT, 0, (LPARAM) format) == 0)
			logLastError("error applying format string to date/time picker in finishNewDateTimePicker()");

	uiControl(d)->Handle = datetimepickerHandle;
	uiControl(d)->PreferredSize = datetimepickerPreferredSize;

	return uiDateTimePicker(d);
}

#define GLI(what, buf, n) GetLocaleInfoEx(LOCALE_NAME_USER_DEFAULT, what, buf, n)

// Windows has no combined date/time prebuilt constant; we have to build the format string ourselves
uiDateTimePicker *uiNewDateTimePicker(void)
{
	WCHAR *date, *time, *datetime;
	int ndate, ntime;
	int n;
	uiDateTimePicker *dtp;

	// TODO verify that this always returns a century year
	ndate = GLI(LOCALE_SSHORTDATE, NULL, 0);
	if (ndate == 0)
		logLastError("error getting date string length in uiNewDateTimePicker()");
	date = (WCHAR *) uiAlloc(ndate * sizeof (WCHAR), "WCHAR[]");
	if (GLI(LOCALE_SSHORTDATE, date, ndate) == 0)
		logLastError("error geting date string in uiNewDateTimePicker()");

	ntime = GLI(LOCALE_STIMEFORMAT, NULL, 0);
	if (ndate == 0)
		logLastError("error getting time string length in uiNewDateTimePicker()");
	time = (WCHAR *) uiAlloc(ntime * sizeof (WCHAR), "WCHAR[]");
	if (GLI(LOCALE_STIMEFORMAT, time, ntime) == 0)
		logLastError("error geting time string in uiNewDateTimePicker()");

	n = _scwprintf(L"%s %s", date, time);
	datetime = (WCHAR *) uiAlloc((n + 1) * sizeof (WCHAR), "WCHAR[]");
	snwprintf(datetime, n + 1, L"%s %s", date, time);
	dtp = finishNewDateTimePicker(0, datetime);

	uiFree(datetime);
	uiFree(time);
	uiFree(date);

	return dtp;
}

uiDateTimePicker *uiNewDatePicker(void)
{
	return finishNewDateTimePicker(DTS_SHORTDATECENTURYFORMAT, NULL);
}

uiDateTimePicker *uiNewTimePicker(void)
{
	return finishNewDateTimePicker(DTS_TIMEFORMAT, NULL);
}
