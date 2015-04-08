// 6 april 2015
#include "uipriv_windows.h"

HINSTANCE hInstance;
int nCmdShow;

HFONT hMessageFont;

HWND initialParent;

struct uiInitError {
	char *msg;
	char failbuf[256];
};

static uiInitError *loadLastError(uiInitError *err, const char *message)
{
	DWORD le;

	le = GetLastError();
	// TODO FormatMessageW() it
	// TODO make sure argument is right; _snprintf_s() isn't supported on Windows XP
	snprintf(err->failbuf, 256, "error %s (last error %I32u)", message, le);
	err->msg = err->failbuf;
	return err;
}

uiInitError *uiInit(uiInitOptions *o)
{
	uiInitError *err;
	STARTUPINFOW si;
	const char *ce;
	HICON hDefaultIcon;
	HCURSOR hDefaultCursor;
	NONCLIENTMETRICSW ncm;

	err = uiNew(uiInitError);

	hInstance = GetModuleHandle(NULL);
	if (hInstance == NULL)
		return loadLastError(err, "getting program HINSTANCE");

	nCmdShow = SW_SHOWDEFAULT;
	GetStartupInfoW(&si);
	if ((si.dwFlags & STARTF_USESHOWWINDOW) != 0)
		nCmdShow = si.wShowWindow;

	ce = initCommonControls();
	if (ce != NULL)
		return loadLastError(err, ce);

	hDefaultIcon = LoadIconW(NULL, IDI_APPLICATION);
	if (hDefaultIcon == NULL)
		return loadLastError(err, "loading default icon for window classes");
	hDefaultCursor = LoadCursorW(NULL, IDC_ARROW);
	if (hDefaultCursor == NULL)
		return loadLastError(err, "loading default cursor for window classes");

	if (registerWindowClass(hDefaultIcon, hDefaultCursor) == 0)
		return loadLastError(err, "registering uiWindow window class");

	ZeroMemory(&ncm, sizeof (NONCLIENTMETRICSW));
	ncm.cbSize = sizeof (NONCLIENTMETRICSW);
	if (SystemParametersInfoW(SPI_GETNONCLIENTMETRICS, sizeof (NONCLIENTMETRICSW), &ncm, sizeof (NONCLIENTMETRICSW)) == 0)
		return loadLastError(err, "getting default fonts");
	hMessageFont = CreateFontIndirectW(&(ncm.lfMessageFont));
	if (hMessageFont == NULL)
		return loadLastError(err, "loading default messagebox font; this is the default UI font");

	// give each control a reasonable initial parent
	// don't free the initial parent!
	// TODO tune this better; it shouldn't be closed, for instance
	initialParent = (HWND) uiWindowHandle(uiNewWindow("", 0, 0));

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
