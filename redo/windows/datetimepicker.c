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

// The real date/time picker does a manual replacement of "yy" with "yyyy" for DTS_SHORTDATECENTURYFORMAT.
// Because we're also duplicating its functionality (see below), we have to do it too.
static WCHAR *expandYear(WCHAR *dts, int n)
{
	WCHAR *out;
	WCHAR *p, *q;
	int ny = 0;

	// allocate more than we need to be safe
	out = (WCHAR *) uiAlloc((n * 3) * sizeof (WCHAR), "WCHAR[]");
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
					complain("unterminated quote in system-provided locale date string in expandYear()");
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
static void setDateTimeFormat(HWND hwnd)
{
	WCHAR *unexpandedDate, *date;
	WCHAR *time;
	WCHAR *datetime;
	int ndate, ntime;
	int n;

	ndate = GLI(LOCALE_SSHORTDATE, NULL, 0);
	if (ndate == 0)
		logLastError("error getting date string length in setDateTimeFormat()");
	date = (WCHAR *) uiAlloc(ndate * sizeof (WCHAR), "WCHAR[]");
	if (GLI(LOCALE_SSHORTDATE, date, ndate) == 0)
		logLastError("error geting date string in setDateTimeFormat()");
	unexpandedDate = date;		// so we can free it
	date = expandYear(unexpandedDate, ndate);
	uiFree(unexpandedDate);

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

// the height returned from DTM_GETIDEALSIZE is unreliable; see http://stackoverflow.com/questions/30626549/what-is-the-proper-use-of-dtm-getidealsize-treating-the-returned-size-as-pixels
// from http://msdn.microsoft.com/en-us/library/windows/desktop/dn742486.aspx#sizingandspacing
#define entryHeight 14

static void datetimepickerPreferredSize(uiControl *c, uiSizing *d, intmax_t *width, intmax_t *height)
{
	struct datetimepicker *dd = (struct datetimepicker *) c;
	SIZE s;

	s.cx = 0;
	s.cy = 0;
	SendMessageW(dd->hwnd, DTM_GETIDEALSIZE, 0, (LPARAM) (&s));
	*width = s.cx;
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
