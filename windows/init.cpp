// 6 april 2015
#include "uipriv_windows.hpp"
#include "attrstr.hpp"

HINSTANCE uipriv_hInstance = NULL;
int uipriv_nCmdShow;

//HFONT hMessageFont;

#define wantedICCClasses ( \
	ICC_STANDARD_CLASSES |	/* user32.dll controls */		\
	ICC_PROGRESS_CLASS |		/* progress bars */			\
	ICC_TAB_CLASSES |			/* tabs */					\
	ICC_LISTVIEW_CLASSES |		/* table headers */			\
	ICC_UPDOWN_CLASS |		/* spinboxes */			\
	ICC_BAR_CLASSES |			/* trackbar */				\
	ICC_DATE_CLASSES |		/* date/time picker */		\
	0)

// see https://devblogs.microsoft.com/oldnewthing/20041025-00/?p=37483
EXTERN_C IMAGE_DOS_HEADER __ImageBase;

#define errICCFailed "InitCommonControlsEx() failed"
#define errICCFailedNoLastError "InitCommonControlsEx() failed, but didn't specify why. This usually means you forgot the Common Controls v6 manifest; refer to the libui documentation for instructions."
#define errCoInitializeFailed "CoInitialize() failed"

#define errHRESULTInitErrorsSuffix ": 0x00000000"
static const char *initErrors[] = {
	errICCFailed errHRESULTInitErrorsSuffix,
	errICCFailedNoLastError,
	errCoInitializeFailed errHRESULTInitErrorsSuffix,
	NULL,
};
#define uiprivInitReturnHRESULT(err, msg, hr) uiprivInitReturnErrorf(err, "%s: 0x%08I32X", msg, hr)

int uiInit(void *options, uiInitError *err)
{
	STARTUPINFOW si;
	INITCOMMONCONTROLSEX icc;
	HRESULT hr;

	if (!uiprivInitCheckParams(options, err, initErrors))
		return 0;

	if (uipriv_hInstance == NULL)
		uipriv_hInstance = (HINSTANCE) (&__ImageBase);
	uipriv_nCmdShow = SW_SHOWDEFAULT;
	GetStartupInfoW(&si);
	if ((si.dwFlags & STARTF_USESHOWWINDOW) != 0)
		uipriv_nCmdShow = si.wShowWindow;

	ZeroMemory(&icc, sizeof (INITCOMMONCONTROLSEX));
	icc.dwSize = sizeof (INITCOMMONCONTROLSEX);
	icc.dwICC = wantedICCClasses;
	if (InitCommonControlsEx(&icc) == 0) {
		DWORD lasterr;

		lasterr = GetLastError();
		if (lasterr == 0)
			return uiprivInitReturnError(err, errICCFailedNoLastError);
		return uiprivInitReturnHRESULT(err, errICCFailed, HRESULT_FROM_WIN32(lasterr));
	}

	hr = CoInitialize(NULL);
	if (hr != S_OK && hr != S_FALSE)
		return ieHRESULT("initializing COM", hr);
	// LONGTERM initialize COM security
	// LONGTERM turn off COM exception handling

	uiprivMarkInitialized();
	return 1;
}

void uiUninit(void)
{
	CoUninitialize();
}

#ifndef uiStatic

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
	if (fdwReason == DLL_PROCESS_ATTACH)
		hInstance = hinstDLL;
	return TRUE;
}

#endif
