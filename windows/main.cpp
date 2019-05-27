// 6 april 2015
#include "uipriv_windows.hpp"

HINSTANCE uipriv_hInstance = NULL;
int uipriv_nCmdShow;

#ifndef uiStatic

BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD fdwReason, LPVOID lpvReserved)
{
	if (fdwReason == DLL_PROCESS_ATTACH)
		uipriv_hInstance = hInstance;
	return TRUE;
}

static inline void setHInstance(void)
{
	// do nothing; set by DllMain() above
}

#else

// see https://devblogs.microsoft.com/oldnewthing/20041025-00/?p=37483
EXTERN_C IMAGE_DOS_HEADER __ImageBase;

static inline void setHInstance(void)
{
	uipriv_hInstance = (HINSTANCE) (&__ImageBase);
}

#endif

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

#define errLoadDefaultIconFailed "failed to load default icon"
#define errLoadDefaultCursorFailed "failed to load default cursor"
#define errInitUtilWindowFailed "failed to initialize the utility window"
#define errICCFailed "InitCommonControlsEx() failed"
#define errICCFailedNoLastError "InitCommonControlsEx() failed, but didn't specify why. This usually means you forgot the Common Controls v6 manifest; refer to the libui documentation for instructions."
#define errCoInitializeFailed "CoInitialize() failed"

#define errHRESULTInitErrorsSuffix ": 0x00000000"
static const char *initErrors[] = {
	errLoadDefaultIconFailed errHRESULTInitErrorsSuffix
	errLoadDefaultCursorFailed errHRESULTInitErrorsSuffix
	errInitUtilWindowFailed errHRESULTInitErrorsSuffix
	errICCFailed errHRESULTInitErrorsSuffix,
	errICCFailedNoLastError,
	errCoInitializeFailed errHRESULTInitErrorsSuffix,
	NULL,
};
#define uiprivInitReturnHRESULT(err, msg, hr) uiprivInitReturnErrorf(err, "%s: 0x%08I32X", msg, hr)

const char **uiprivSysInitErrors(void)
{
	return initErrors;
}

int uiprivSysInit(void *options, uiInitError *err)
{
	STARTUPINFOW si;
	HICON hDefaultIcon;
	HCURSOR hDefaultCursor;
	INITCOMMONCONTROLSEX icc;
	HRESULT hr;

	setHInstance();
	uipriv_nCmdShow = SW_SHOWDEFAULT;
	GetStartupInfoW(&si);
	if ((si.dwFlags & STARTF_USESHOWWINDOW) != 0)
		uipriv_nCmdShow = si.wShowWindow;

	hr = uiprivHrLoadIconW(NULL, IDI_APPLICATION, &hDefaultIcon);
	if (hr != S_OK)
		return uiprivInitReturnHRESULT(err, errLoadDefaultIconFailed, hr);
	hr = uiprivHrLoadCursorW(NULL, IDC_ARROW, &hDefaultCursor);
	if (hr != S_OK)
		return uiprivInitReturnHRESULT(err, errLoadDefaultCursorFailed, hr);

	hr = uiprivInitUtilWindow(hDefaultIcon, hDefaultCursor);
	if (hr != S_OK)
		return uiprivInitReturnHRESULT(err, errInitUtilWindowFailed, hr);

	ZeroMemory(&icc, sizeof (INITCOMMONCONTROLSEX));
	icc.dwSize = sizeof (INITCOMMONCONTROLSEX);
	icc.dwICC = wantedICCClasses;
	if (InitCommonControlsEx(&icc) == 0) {
		DWORD lasterr;

		lasterr = GetLastError();
		if (lasterr == 0)
			return uiprivInitReturnError(err, errICCFailedNoLastError);
		hr = HRESULT_FROM_WIN32(lasterr);
		return uiprivInitReturnHRESULT(err, errICCFailed, hr);
	}

/*	hr = CoInitialize(NULL);
	if (hr != S_OK && hr != S_FALSE)
		return ieHRESULT("initializing COM", hr);
	// LONGTERM initialize COM security
	// LONGTERM turn off COM exception handling
*/
	return 1;
}

void uiMain(void)
{
	MSG msg;
	HRESULT hr;

	for (;;) {
		hr = uiprivHrGetMessageW(&msg, NULL, 0, 0);
		if (hr == S_FALSE)		// WM_QUIT
			return;
		if (hr != S_OK) {
			// TODO log error
			return;
		}
		// TODO IsDialogMessage()
		TranslateMessage(&msg);
		DispatchMessageW(&msg);
	}
}

void uiQuit(void)
{
	PostQuitMessage(0);
}

void uiQueueMain(void (*f)(void *data), void *data)
{
	HRESULT hr;

	hr = uiprivHrPostMessageW(uiprivUtilWindow, uiprivUtilWindowMsgQueueMain, (WPARAM) f, (LPARAM) data);
	if (hr != S_OK) {
		// TODO handle error
	}
}

void uiprivReportError(const char *prefix, const char *msg, const char *suffix, bool internal)
{
	// Print to both stderr and the debugger; the former handles the case of a console and the latter handles the case of not.
	// TODO find a reliable way to check if stderr is connected (the mere presence of a console window is not one)
	fprintf(stderr, "*** %s: %s. %s\n", prefix, msg, suffix);
	// Use OutputDebugStringA() directly because these *are* runtime MBCS strings.
	OutputDebugStringA(prefix);
	OutputDebugStringA(": ");
	OutputDebugStringA(msg);
	OutputDebugStringA(". ");
	OutputDebugStringA(suffix);
	OutputDebugStringA("\n");
	DebugBreak();
	abort();		// we shouldn't reach here
}
