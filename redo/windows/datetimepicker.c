// 22 may 2015
#include "uipriv_windows.h"

struct datetimepicker {
	uiDateTimePicker d;
	HWND hwnd;
	void (*baseCommitDestroy)(uiControl *);
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

static void datetimepickerCommitDestroy(uiControl *c)
{
	struct datetimepicker *d = (struct datetimepicker *) c;

	uiWindowsUnregisterReceiveWM_WININICHANGE(d->hwnd);
	(*(d->baseCommitDestroy))(uiControl(d));
}

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

	// automatically update date/time format when user changes locale settings
	// for the standard styles, this is in the date-time picker itself
	// for our date/time mode, we do it in a subclass assigned in uiNewDateTimePicker()
	uiWindowsRegisterReceiveWM_WININICHANGE(d->hwnd);

	d->baseCommitDestroy = uiControl(d)->CommitDestroy;
	uiControl(d)->CommitDestroy = datetimepickerCommitDestroy;
	uiControl(d)->Handle = datetimepickerHandle;
	uiControl(d)->PreferredSize = datetimepickerPreferredSize;

	return uiDateTimePicker(d);
}

static LRESULT CALLBACK datetimepickerSubProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData)
{
	switch (uMsg) {
	case WM_WININICHANGE:
		// we can optimize this by only doing it when the real date/time picker does it
		// unfortunately, I don't know when that is :/
		// hopefully this won't hurt
		setDateTimeFormat(hwnd);
		return 0;
	case WM_NCDESTROY:
		if (RemoveWindowSubclass(hwnd, datetimepickerSubProc, uIdSubclass) == FALSE)
			logLastError("error removing date-time picker locale change handling subclass in datetimepickerSubProc()");
		break;
	}
	return DefSubclassProc(hwnd, uMsg, wParam, lParam);
}

uiDateTimePicker *uiNewDateTimePicker(void)
{
	uiDateTimePicker *dtp;
	struct datetimepicker *d;

	dtp = finishNewDateTimePicker(0);
	d = (struct datetimepicker *) dtp;
	setDateTimeFormat(d->hwnd);
	if (SetWindowSubclass(d->hwnd, datetimepickerSubProc, 0, (DWORD_PTR) d) == FALSE)
		logLastError("error subclassing date-time-picker to assist in locale change handling in uiNewDateTimePicker()");
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
