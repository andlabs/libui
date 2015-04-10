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

#define initErrorFormat L"error %s: %s%sGetLastError() == %I32u%s"
#define initErrorArgs wmessage, sysmsg, beforele, le, afterle

static const char *loadLastError(const char *message)
{
	WCHAR *sysmsg;
	BOOL hassysmsg;
	WCHAR *beforele;
	WCHAR *afterle;
	int n;
	WCHAR *wmessage;
	WCHAR *wstr;
	const char *str;
	DWORD le;

	le = GetLastError();
	if (FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, le, 0, (LPWSTR) (&sysmsg), 0, NULL) != 0) {
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
	swprintf_s(wstr, n + 1, initErrorFormat, initErrorArgs);
	str = toUTF8(wstr);
	uiFree(wstr);
	if (hassysmsg)
		if (LocalFree(sysmsg) != NULL)
			logLastError("error freeing system message in loadLastError()");
	uiFree(wmessage);
	return str;
}

uiInitOptions options;

const char *uiInit(uiInitOptions *o)
{
	STARTUPINFOW si;
	const char *ce;
	HICON hDefaultIcon;
	HCURSOR hDefaultCursor;
	NONCLIENTMETRICSW ncm;

	options = *o;

	hInstance = GetModuleHandle(NULL);
	if (hInstance == NULL)
		return loadLastError("getting program HINSTANCE");

	nCmdShow = SW_SHOWDEFAULT;
	GetStartupInfoW(&si);
	if ((si.dwFlags & STARTF_USESHOWWINDOW) != 0)
		nCmdShow = si.wShowWindow;

	ce = initCommonControls();
	if (ce != NULL)
		return loadLastError(ce);

	hDefaultIcon = LoadIconW(NULL, IDI_APPLICATION);
	if (hDefaultIcon == NULL)
		return loadLastError("loading default icon for window classes");
	hDefaultCursor = LoadCursorW(NULL, IDC_ARROW);
	if (hDefaultCursor == NULL)
		return loadLastError("loading default cursor for window classes");

	if (registerWindowClass(hDefaultIcon, hDefaultCursor) == 0)
		return loadLastError("registering uiWindow window class");

	ZeroMemory(&ncm, sizeof (NONCLIENTMETRICSW));
	ncm.cbSize = sizeof (NONCLIENTMETRICSW);
	if (SystemParametersInfoW(SPI_GETNONCLIENTMETRICS, sizeof (NONCLIENTMETRICSW), &ncm, sizeof (NONCLIENTMETRICSW)) == 0)
		return loadLastError("getting default fonts");
	hMessageFont = CreateFontIndirectW(&(ncm.lfMessageFont));
	if (hMessageFont == NULL)
		return loadLastError("loading default messagebox font; this is the default UI font");

	// give each control a reasonable initial parent
	// don't free the initial parent!
	// TODO tune this better; it shouldn't be closed, for instance
	initialParent = (HWND) uiWindowHandle(uiNewWindow("", 0, 0));

	return NULL;
}

void uiFreeInitError(const char *err)
{
	uiFree((void *) err);
}
