// 25 november 2015
#ifdef __cplusplus
#error msbuild is being dumb and making this a C++ file
#endif
#include "unmanaged.h"

// TODO this won't work if initAlloc() failed

#define initErrorFormat L"error %s: %s%s%s %I32u (0x%I32X)%s"
#define initErrorArgs wmessage, sysmsg, beforele, label, value, value, afterle

static const char *initerr(const char *message, const WCHAR *label, DWORD value)
{
	// TODO
	return "";
#if 0
	WCHAR *sysmsg;
	BOOL hassysmsg;
	WCHAR *beforele;
	WCHAR *afterle;
	int n;
	WCHAR *wmessage;
	WCHAR *wstr;
	const char *str;

	if (FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, value, 0, (LPWSTR) (&sysmsg), 0, NULL) != 0) {
		hassysmsg = TRUE;
		beforele = L" (";
		afterle = L")";
	} else {
		hassysmsg = FALSE;
		sysmsg = L"";
		beforele = L"";
		afterle = L"";
	}
	wmessage = toUTF16(message);
	n = _scwprintf(initErrorFormat, initErrorArgs);
	wstr = (WCHAR *) uiAlloc((n + 1) * sizeof (WCHAR), "WCHAR[]");
	snwprintf(wstr, n + 1, initErrorFormat, initErrorArgs);
	str = toUTF8(wstr);
	uiFree(wstr);
	if (hassysmsg)
		if (LocalFree(sysmsg) != NULL)
			logLastError("error freeing system message in loadLastError()");
	uiFree(wmessage);
	return str;
#endif
}

static const char *loadLastError(const char *message)
{
	return initerr(message, L"GetLastError() ==", GetLastError());
}

static const char *loadHRESULT(const char *message, HRESULT hr)
{
	return initerr(message, L"HRESULT", (DWORD) hr);
}

// On the subject of CoInitialize(), or why this isn't in main.cpp:
// If we don't set up the current thread otherwise, the first time .net tries to call out to unmanaged code, it will automatically set up a MTA for COM.
// This is not what we want; we need a STA instead.
// Since we're not in control of main(), we can't stick a [STAThread] on it, so we have to do it ourselves.
// This is a separate .c file for two reasons:
// 1) To avoid the unmanaged jump that a call to CoInitialize() would do (it seems to detect a call to CoInitialize()/CoInitializeEx() but let's not rely on it)
// 2) To avoid mixing Windows API headers with .net
// See also http://stackoverflow.com/questions/24348205/how-do-i-solve-this-com-issue-in-c

uiInitOptions options;

const char *uiInit(uiInitOptions *o)
{
	HRESULT hr;

	options = *o;

	if (initAlloc() == 0)
		return loadLastError("error initializing memory allocations");

	// TODO https://msdn.microsoft.com/en-us/library/5s8ee185%28v=vs.71%29.aspx use CoInitializeEx()?
	hr = CoInitialize(NULL);
	if (hr != S_OK && hr != S_FALSE)
		return loadHRESULT("initializing COM", hr);

	// now do the rest of initialization on the managed side
	initWPF();

	return NULL;
}

void uiUninit(void)
{
	uninitWPF();
	CoUninitialize();
	uninitTypes();
	uninitAlloc();
}

void uiFreeInitError(const char *err)
{
	uiFree((void *) err);
}
