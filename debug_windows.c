// 25 february 2015
#include "uipriv_windows.h"

// uncomment the following line to enable debug messages
#define tableDebug
// uncomment the following line to halt on a debug message
#define tableDebugStop

#ifdef tableDebug

#include <stdio.h>

HRESULT logLastError(const char *context)
{
	DWORD le;
	WCHAR *msg;
	int parenthesize = 0;

	le = GetLastError();
	fprintf(stderr, "%s: ", context);
	if (FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, le, 0, (LPWSTR) (&msg), 0, NULL) != 0) {
		fprintf(stderr, "%S (", msg);
		// TODO check error
		LocalFree(msg);
		parenthesize = 1;
	}
	fprintf(stderr, "GetLastError() == %I32u", le);
	if (parenthesize)
		fprintf(stderr, ")");
	fprintf(stderr, "\n");
#ifdef tableDebugStop
	DebugBreak();
#endif
	SetLastError(le);
	// a function does not have to set a last error
	// if the last error we get is actually 0, then HRESULT_FROM_WIN32(0) will return S_OK (0 cast to an HRESULT, since 0 <= 0), which we don't want
	// prevent this by returning E_FAIL, so the rest of the Table code doesn't barge onward
	if (le == 0)
		return E_FAIL;
	return HRESULT_FROM_WIN32(le);
}

HRESULT logHRESULT(const char *context, HRESULT hr)
{
	WCHAR *msg;
	int parenthesize = 0;

	fprintf(stderr, "%s: ", context);
	// this isn't technically documented, but everyone does it, including Microsoft (see the implementation of _com_error::ErrorMessage() in a copy of comdef.h that comes with the Windows DDK)
	if (FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, (DWORD) hr, 0, (LPWSTR) (&msg), 0, NULL) != 0) {
		fprintf(stderr, "%S (", msg);
		// TODO check error
		LocalFree(msg);
		parenthesize = 1;
	}
	fprintf(stderr, "HRESULT == 0x%I32X", hr);
	if (parenthesize)
		fprintf(stderr, ")");
	fprintf(stderr, "\n");
#ifdef tableDebugStop
	DebugBreak();
#endif
	return hr;
}

HRESULT logMemoryExhausted(const char *reason)
{
	fprintf(stderr, "memory exhausted %s\n", reason);
#ifdef tableDebugStop
	DebugBreak();
#endif
	return E_OUTOFMEMORY;
}

#else

HRESULT logLastError(const char *reason)
{
	DWORD le;

	le = GetLastError();
	// technically (I think? TODO) we don't need to do this, but let's do this anyway just to be safe
	SetLastError(le);
	if (le == 0)
		return E_FAIL;
	return HRESULT_FROM_WIN32(le);
}

HRESULT logHRESULT(const char *reason, HRESULT hr)
{
	return hr;
}

HRESULT logMemoryExhausted(const char *reason)
{
	return E_OUTOFMEMORY;
}

#endif
