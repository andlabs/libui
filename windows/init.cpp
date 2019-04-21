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

int uiInit(void *options, uiInitError *err)
{
	STARTUPINFOW si;
	const char *ce;
	HICON hDefaultIcon;
	HCURSOR hDefaultCursor;
	NONCLIENTMETRICSW ncm;
	INITCOMMONCONTROLSEX icc;
	HRESULT hr;

	if (!uiprivInitCheckParams(options, err, NULL))
		return 0;

	if (uipriv_hInstance == NULL)
		uipriv_hInstance = (HINSTANCE) (&__ImageBase);
	uipriv_nCmdShow = SW_SHOWDEFAULT;
	GetStartupInfoW(&si);
	if ((si.dwFlags & STARTF_USESHOWWINDOW) != 0)
		uipriv_nCmdShow = si.wShowWindow;

	// LONGTERM set DPI awareness

	hDefaultIcon = LoadIconW(NULL, IDI_APPLICATION);
	if (hDefaultIcon == NULL)
		return ieLastErr("loading default icon for window classes");
	hDefaultCursor = LoadCursorW(NULL, IDC_ARROW);
	if (hDefaultCursor == NULL)
		return ieLastErr("loading default cursor for window classes");

	ce = initUtilWindow(hDefaultIcon, hDefaultCursor);
	if (ce != NULL)
		return initerr(ce, L"GetLastError() ==", GetLastError());

	if (registerWindowClass(hDefaultIcon, hDefaultCursor) == 0)
		return ieLastErr("registering uiWindow window class");

	ZeroMemory(&ncm, sizeof (NONCLIENTMETRICSW));
	ncm.cbSize = sizeof (NONCLIENTMETRICSW);
	if (SystemParametersInfoW(SPI_GETNONCLIENTMETRICS, sizeof (NONCLIENTMETRICSW), &ncm, sizeof (NONCLIENTMETRICSW)) == 0)
		return ieLastErr("getting default fonts");
	hMessageFont = CreateFontIndirectW(&(ncm.lfMessageFont));
	if (hMessageFont == NULL)
		return ieLastErr("loading default messagebox font; this is the default UI font");

	if (initContainer(hDefaultIcon, hDefaultCursor) == 0)
		return ieLastErr("initializing uiWindowsMakeContainer() window class");

	hollowBrush = (HBRUSH) GetStockObject(HOLLOW_BRUSH);
	if (hollowBrush == NULL)
		return ieLastErr("getting hollow brush");

	ZeroMemory(&icc, sizeof (INITCOMMONCONTROLSEX));
	icc.dwSize = sizeof (INITCOMMONCONTROLSEX);
	icc.dwICC = wantedICCClasses;
	if (InitCommonControlsEx(&icc) == 0)
		return ieLastErr("initializing Common Controls");

	hr = CoInitialize(NULL);
	if (hr != S_OK && hr != S_FALSE)
		return ieHRESULT("initializing COM", hr);
	// LONGTERM initialize COM security
	// LONGTERM (windows vista) turn off COM exception handling

	hr = initDraw();
	if (hr != S_OK)
		return ieHRESULT("initializing Direct2D", hr);

	hr = uiprivInitDrawText();
	if (hr != S_OK)
		return ieHRESULT("initializing DirectWrite", hr);

	if (registerAreaClass(hDefaultIcon, hDefaultCursor) == 0)
		return ieLastErr("registering uiArea window class");

	if (registerMessageFilter() == 0)
		return ieLastErr("registering libui message filter");

	if (registerD2DScratchClass(hDefaultIcon, hDefaultCursor) == 0)
		return ieLastErr("initializing D2D scratch window class");

	hr = uiprivInitImage();
	if (hr != S_OK)
		return ieHRESULT("initializing WIC", hr);

	return NULL;
}

void uiUninit(void)
{
}

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
	if (fdwReason == DLL_PROCESS_ATTACH)
		hInstance = hinstDLL;
	return TRUE;
}
