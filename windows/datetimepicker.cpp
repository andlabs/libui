// 22 may 2015
#include "uipriv_windows.hpp"

struct uiDateTimePicker {
	uiWindowsControl c;
	HWND hwnd;
	void (*onChanged)(uiDateTimePicker *, void *);
	void *onChangedData;
};

// utility functions

#define GLI(what, buf, n) GetLocaleInfoEx(LOCALE_NAME_USER_DEFAULT, what, buf, n)

// The real date/time picker does a manual replacement of "yy" with "yyyy" for DTS_SHORTDATECENTURYFORMAT.
// Because we're also duplicating its functionality (see below), we have to do it too.
static WCHAR *expandYear(WCHAR *dts, int n)
{
	WCHAR *out;
	WCHAR *p, *q;
	int ny = 0;

	// allocate more than we need to be safe
	out = (WCHAR *) uiprivAlloc((n * 3) * sizeof (WCHAR), "WCHAR[]");
	q = out;
	for (p = dts; *p != L'\0'; p++) {
		// first, if the current character is a y, increment the number of consecutive ys
		// otherwise, stop counting, and if there were only two, add two more to make four
		if (*p != L'y') {
			if (ny == 2) {
				*q++ = L'y';
				*q++ = L'y';
			}
			ny = 0;
		} else
			ny++;
		// next, handle quoted blocks
		// we do this AFTER the above so yy'abc' becomes yyyy'abc' and not yy'abc'yy
		// this handles the case of 'a''b' elegantly as well
		if (*p == L'\'') {
			// copy the opening quote
			*q++ = *p;
			// copy the contents
			for (;;) {
				p++;
				if (*p == L'\'')
					break;
				if (*p == L'\0')
					uiprivImplBug("unterminated quote in system-provided locale date string in expandYear()");
				*q++ = *p;
			}
			// and fall through to copy the closing quote
		}
		// copy the current character
		*q++ = *p;
	}
	// handle trailing yy
	if (ny == 2) {
		*q++ = L'y';
		*q++ = L'y';
	}
	*q++ = L'\0';
	return out;
}

// Windows has no combined date/time prebuilt constant; we have to build the format string ourselves
// TODO use a default format if one fails
static void setDateTimeFormat(HWND hwnd)
{
	WCHAR *unexpandedDate, *date;
	WCHAR *time;
	WCHAR *datetime;
	int ndate, ntime;

	ndate = GLI(LOCALE_SSHORTDATE, NULL, 0);
	if (ndate == 0)
		logLastError(L"error getting date string length");
	date = (WCHAR *) uiprivAlloc(ndate * sizeof (WCHAR), "WCHAR[]");
	if (GLI(LOCALE_SSHORTDATE, date, ndate) == 0)
		logLastError(L"error geting date string");
	unexpandedDate = date;		// so we can free it
	date = expandYear(unexpandedDate, ndate);
	uiprivFree(unexpandedDate);

	ntime = GLI(LOCALE_STIMEFORMAT, NULL, 0);
	if (ndate == 0)
		logLastError(L"error getting time string length");
	time = (WCHAR *) uiprivAlloc(ntime * sizeof (WCHAR), "WCHAR[]");
	if (GLI(LOCALE_STIMEFORMAT, time, ntime) == 0)
		logLastError(L"error geting time string");

	datetime = strf(L"%s %s", date, time);
	if (SendMessageW(hwnd, DTM_SETFORMAT, 0, (LPARAM) datetime) == 0)
		logLastError(L"error applying format string to date/time picker");

	uiprivFree(datetime);
	uiprivFree(time);
	uiprivFree(date);
}

// control implementation

static void uiDateTimePickerDestroy(uiControl *c)
{
	uiDateTimePicker *d = uiDateTimePicker(c);

	uiWindowsUnregisterReceiveWM_WININICHANGE(d->hwnd);
	uiWindowsUnregisterWM_NOTIFYHandler(d->hwnd);
	uiWindowsEnsureDestroyWindow(d->hwnd);
	uiFreeControl(uiControl(d));
}

uiWindowsControlAllDefaultsExceptDestroy(uiDateTimePicker)

// the height returned from DTM_GETIDEALSIZE is unreliable; see http://stackoverflow.com/questions/30626549/what-is-the-proper-use-of-dtm-getidealsize-treating-the-returned-size-as-pixels
// from http://msdn.microsoft.com/en-us/library/windows/desktop/dn742486.aspx#sizingandspacing
#define entryHeight 14

static void uiDateTimePickerMinimumSize(uiWindowsControl *c, int *width, int *height)
{
	uiDateTimePicker *d = uiDateTimePicker(c);
	SIZE s;
	uiWindowsSizing sizing;
	int y;

	s.cx = 0;
	s.cy = 0;
	SendMessageW(d->hwnd, DTM_GETIDEALSIZE, 0, (LPARAM) (&s));
	*width = s.cx;

	y = entryHeight;
	uiWindowsGetSizing(d->hwnd, &sizing);
	uiWindowsSizingDlgUnitsToPixels(&sizing, NULL, &y);
	*height = y;
}

static BOOL onWM_NOTIFY(uiControl *c, HWND hwnd, NMHDR *nmhdr, LRESULT *lResult)
{
	uiDateTimePicker *d = uiDateTimePicker(c);

	if (nmhdr->code != DTN_DATETIMECHANGE)
		return FALSE;
	(*(d->onChanged))(d, d->onChangedData);
	*lResult = 0;
	return TRUE;
}

static void fromSystemTime(SYSTEMTIME *systime, struct tm *time)
{
	ZeroMemory(time, sizeof (struct tm));
	time->tm_sec = systime->wSecond;
	time->tm_min = systime->wMinute;
	time->tm_hour = systime->wHour;
	time->tm_mday = systime->wDay;
	time->tm_mon = systime->wMonth - 1;
	time->tm_year = systime->wYear - 1900;
	time->tm_wday = systime->wDayOfWeek;
	time->tm_isdst = -1;
}

static void toSystemTime(const struct tm *time, SYSTEMTIME *systime)
{
	ZeroMemory(systime, sizeof (SYSTEMTIME));
	systime->wYear = time->tm_year + 1900;
	systime->wMonth = time->tm_mon + 1;
	systime->wDayOfWeek = time->tm_wday;
	systime->wDay = time->tm_mday;
	systime->wHour = time->tm_hour;
	systime->wMinute = time->tm_min;
	systime->wSecond = time->tm_sec;
}

static void defaultOnChanged(uiDateTimePicker *d, void *data)
{
	// do nothing
}

void uiDateTimePickerTime(uiDateTimePicker *d, struct tm *time)
{
	SYSTEMTIME systime;

	if (SendMessageW(d->hwnd, DTM_GETSYSTEMTIME, 0, (LPARAM) (&systime)) != GDT_VALID)
		logLastError(L"error getting date and time");
	fromSystemTime(&systime, time);
}

void uiDateTimePickerSetTime(uiDateTimePicker *d, const struct tm *time)
{
	SYSTEMTIME systime;

	toSystemTime(time, &systime);
	if (SendMessageW(d->hwnd, DTM_SETSYSTEMTIME, GDT_VALID, (LPARAM) (&systime)) == 0)
		logLastError(L"error setting date and time");
}

void uiDateTimePickerOnChanged(uiDateTimePicker *d, void (*f)(uiDateTimePicker *, void *), void *data)
{
	d->onChanged = f;
	d->onChangedData = data;
}

static uiDateTimePicker *finishNewDateTimePicker(DWORD style)
{
	uiDateTimePicker *d;

	uiWindowsNewControl(uiDateTimePicker, d);

	d->hwnd = uiWindowsEnsureCreateControlHWND(WS_EX_CLIENTEDGE,
		DATETIMEPICK_CLASSW, L"",
		style | WS_TABSTOP,
		hInstance, NULL,
		TRUE);

	// automatically update date/time format when user changes locale settings
	// for the standard styles, this is in the date-time picker itself
	// for our date/time mode, we do it in a subclass assigned in uiNewDateTimePicker()
	uiWindowsRegisterReceiveWM_WININICHANGE(d->hwnd);
	uiWindowsRegisterWM_NOTIFYHandler(d->hwnd, onWM_NOTIFY, uiControl(d));
	uiDateTimePickerOnChanged(d, defaultOnChanged, NULL);

	return d;
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
			logLastError(L"error removing date-time picker locale change handling subclass");
		break;
	}
	return DefSubclassProc(hwnd, uMsg, wParam, lParam);
}

uiDateTimePicker *uiNewDateTimePicker(void)
{
	uiDateTimePicker *d;

	d = finishNewDateTimePicker(0);
	setDateTimeFormat(d->hwnd);
	if (SetWindowSubclass(d->hwnd, datetimepickerSubProc, 0, (DWORD_PTR) d) == FALSE)
		logLastError(L"error subclassing date-time-picker to assist in locale change handling");
		// TODO set a suitable default in this case
	return d;
}

uiDateTimePicker *uiNewDatePicker(void)
{
	return finishNewDateTimePicker(DTS_SHORTDATECENTURYFORMAT);
}

uiDateTimePicker *uiNewTimePicker(void)
{
	return finishNewDateTimePicker(DTS_TIMEFORMAT);
}
