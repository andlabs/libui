// 25 february 2015
#include "uipriv_windows.hpp"

// TODO disable logging and stopping on no-debug builds

// TODO are the newlines needed?
static void printDebug(WCHAR *msg)
{
	OutputDebugStringW(msg);
}

#define debugfmt L"%s:" L ## PRIiMAX L":%s()"

HRESULT _logLastError(debugargs, const WCHAR *func, const WCHAR *s)
{
	DWORD le;
	WCHAR *msg;
	WCHAR *formatted;
	BOOL useFormatted;

	le = GetLastError();

	useFormatted = FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, le, 0, (LPWSTR) (&formatted), 0, NULL) != 0;
	if (!useFormatted)
		formatted = L"\n";
	msg = debugstrf(L"[libui] " debugfmt L" %s: GetLastError() == %I32u %s",
		file, line, func,
		s, le, formatted);
	if (useFormatted)
		LocalFree(formatted);		// ignore error
	if (msg == NULL) {
		printDebug(L"[libui] (debugstrf() failed; printing raw) ");
		printDebug(file);
		printDebug(func);
		printDebug(s);
		printDebug(L"\n");
	} else {
		printDebug(msg);
		uiFree(msg);
	}
	DebugBreak();

	SetLastError(le);
	// a function does not have to set a last error
	// if the last error we get is actually 0, then HRESULT_FROM_WIN32(0) will return S_OK (0 cast to an HRESULT, since 0 <= 0), which we don't want
	// prevent this by returning E_FAIL
	if (le == 0)
		return E_FAIL;
	return HRESULT_FROM_WIN32(le);
}

HRESULT _logHRESULT(debugargs, const WCHAR *s, HRESULT hr)
{
	DWORD le;
	WCHAR *msg;
	WCHAR *formatted;
	BOOL useFormatted;

	useFormatted = FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, hr, 0, (LPWSTR) (&formatted), 0, NULL) != 0;
	if (!useFormatted)
		formatted = L"\n";
	msg = debugstrf(L"[libui] " debugfmt L" %s: HRESULT == 0x%08I32X %s",
		file, line, func,
		s, hr, formatted);
	if (useFormatted)
		LocalFree(formatted);		// ignore error
	if (msg == NULL) {
		printDebug(L"[libui] (debugstrf() failed; printing raw) ");
		printDebug(file);
		printDebug(func);
		printDebug(s);
		printDebug(L"\n");
	} else {
		printDebug(msg);
		uiFree(msg);
	}
	DebugBreak();

	return hr;
}

#define implbugmsg L"either you have or libui has a bug in a control implementation; if libui does, contact andlabs"

void _implbug(debugargs, const WCHAR *format, ...)
{
	va_list ap;
	WCHAR *formatted;
	WCHAR *full;
	const WCHAR *onerr;

	va_start(ap, format);
	formatted = debugvstrf(format, ap);
	va_end(ap);
	if (formatted == NULL) {
		onerr = s;
		goto bad;
	}

	full = debugstrf(L"[libui] " debugfmt L" " implbugmsg L" — %s\n",
		file, line, func,
		formatted);
	if (full == NULL) {
		onerr = formatted;
		goto bad;
	}

	printDebug(full);
	uiFree(full);
	uiFree(formatted);
	goto after;

bad:
	printDebug(L"[libui] (debugstrf() failed; printing raw) ");
	printDebug(implbugmsg);
	printDebug(file);
	printDebug(func);
	printDebug(onerr);
	printDebug(L"\n");

after:
	DebugBreak();
	abort();
}
