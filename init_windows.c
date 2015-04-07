// 6 april 2015
#include "uipriv_windows.h"

HINSTANCE hInstance;
int nCmdShow;

HFONT hMessageFont;

struct uiInitError {
	char *msg;
	char failbuf[256];
};

static void loadLastError(uiInitError *err, char *message)
{
	DWORD le;

	le = GetLastError();
	// TODO FormatMessageW() it
	_snprintf_s(err->failbuf, 256, _TRUNCATE, "error %s (last error %I32u)", message, le);
	err->msg = err->failbuf;
}

uiInitError *uiInit(uiInitOptions *o)
{
	uiInitError *err;
	STARTUPINFOW si;
	const char *ce;
	HICON hDefaultIcon;
	HCURSOR hDefaultCursor;
	NONCLIENTMETRICSW ncm;

	err = (uiInitError *) uiAlloc(sizeof (uiInitError));

	hInstance = GetModuleHandle(NULL);
	if (hInstance == NULL) {
		loadLastError(err, "getting program HINSTANCE");
		return err;
	}

	nCmdShow = SW_SHOWDEFAULT;
	GetStartupInfoW(&si);
	if ((si.dwFlags & STARTF_USESHOWWINDOW) != 0)
		nCmdShow = si.wShowWindow;

	// TODO add "in initCommonControls()" to each of the messages this returns
	// TODO make loadLastError() return err directly
	ce = initCommonControls();
	if (ce != NULL) {
		loadLastError(err, ce);
		return err;
	}

	hDefaultIcon = LoadIconW(NULL, IDI_APPLICATION);
	if (hDefaultIcon == NULL) {
		loadLastError(err, "loading default icon for window classes");
		return err;
	}
	hDefaultCursor = LoadCursorW(NULL, IDC_ARROW);
	if (hDefaultCursor == NULL) {
		loadLastError(err, "loading default cursor for window classes");
		return err;
	}

	if (registerWindowClass(hDefaultIcon, hDefaultCursor) == 0) {
		loadLastError(err, "registering uiWindow window class");
		return err;
	}

	ZeroMemory(&ncm, sizeof (NONCLIENTMETRICSW));
	ncm.cbSize = sizeof (NONCLIENTMETRICSW);
	if (SystemParametersInfoW(SPI_GETNONCLIENTMETRICS, sizeof (NONCLIENTMETRICSW), &ncm, sizeof (NONCLIENTMETRICSW)) == 0) {
		loadLastError(err, "getting default fonts");
		return err;
	}
	hMessageFont = CreateFontIndirectW(&(ncm.lfMessageFont));
	if (hMessageFont == NULL) {
		loadLastError(err, "loading default messagebox font; this is the default UI font");
		return err;
	}

	uiFree(err);
	return NULL;
}

const char *uiInitErrorMessage(uiInitError *err)
{
	return err->msg;
}

void uiInitErrorFree(uiInitError *err)
{
	if (err->msg != err->failbuf)
		uiFree(err->msg);
	uiFree(err);
}
