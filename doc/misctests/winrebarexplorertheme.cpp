// 9 october 2018
#define UNICODE
#define _UNICODE
#define STRICT
#define STRICT_TYPED_ITEMIDS
#define WINVER			0x0600	/* from Microsoft's winnls.h */
#define _WIN32_WINNT		0x0600
#define _WIN32_WINDOWS	0x0600	/* from Microsoft's pdh.h */
#define _WIN32_IE			0x0700
#define NTDDI_VERSION		0x06000000
#include <windows.h>
#include <commctrl.h>
#include <uxtheme.h>
#include <vsstyle.h>
#include <vssym32.h>
#include <stdio.h>
#include <stdlib.h>

void diele(const char *func)
{
	DWORD le;

	le = GetLastError();
	fprintf(stderr, "%s: %I32u\n", func, le);
	exit(EXIT_FAILURE);
}

void diehr(const char *func, HRESULT hr)
{
	fprintf(stderr, "%s: 0x%08I32X\n", func, hr);
	exit(EXIT_FAILURE);
}

HINSTANCE hInstance;
HWND rebar;
HWND leftbar;
HWND rightbar;

static struct {
	const WCHAR *text;
	BOOL dropdown;
} leftbarButtons[] = {
	{ L"Organize", TRUE },
	{ L"Include in library", TRUE },
	{ L"Share with", TRUE },
	{ L"Burn", FALSE },
	{ L"New folder", FALSE },
};

void onWM_CREATE(HWND hwnd)
{
	TBBUTTON tbb[5];
	REBARBANDINFOW rbi;
	int i;

#if 0
	rebar = CreateWindowExW(0,
		REBARCLASSNAMEW, NULL,
		WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | CCS_NODIVIDER | CCS_TOP,
		0, 0, 0, 0,
		hwnd, (HMENU) 100, hInstance, NULL);
	if (rebar == NULL)
		diele("CreateWindowExW(REBARCLASSNAMEW)");
#endif

	leftbar = CreateWindowExW(0,
		TOOLBARCLASSNAMEW, NULL,
		WS_CHILD | /*CCS_NOPARENTALIGN | CCS_NORESIZE | */TBSTYLE_FLAT | TBSTYLE_LIST | TBSTYLE_TRANSPARENT,
		0, 0, 0, 0,
		hwnd, (HMENU) 101, hInstance, NULL);
	SendMessageW(leftbar, TB_BUTTONSTRUCTSIZE, sizeof (TBBUTTON), 0);
	ZeroMemory(tbb, 5 * sizeof (TBBUTTON));
	for (i = 0; i < 5; i++) {
		tbb[i].iBitmap = I_IMAGENONE;
		tbb[i].idCommand = i;
		tbb[i].fsState = TBSTATE_ENABLED;
		tbb[i].fsStyle = BTNS_AUTOSIZE | BTNS_BUTTON | BTNS_NOPREFIX | BTNS_SHOWTEXT;
		if (leftbarButtons[i].dropdown)
			tbb[i].fsStyle |= BTNS_DROPDOWN | BTNS_WHOLEDROPDOWN;
		tbb[i].iString = (INT_PTR) (leftbarButtons[i].text);
	}
	if (SendMessageW(leftbar, TB_ADDBUTTONS, 5, (LPARAM) tbb) == FALSE)
		diele("TB_ADDBUTTONS");

#if 0
	ZeroMemory(&rbi, sizeof (REBARBANDINFOW));
	rbi.cbSize = sizeof (REBARBANDINFOW);
	rbi.fMask = RBBIM_CHILD | RBBIM_STYLE;
	rbi.fStyle = RBBS_NOGRIPPER | RBBS_USECHEVRON | RBBS_HIDETITLE;
	rbi.hwndChild = leftbar;
	if (SendMessageW(rebar, RB_INSERTBANDW, (WPARAM) (-1), (LPARAM) (&rbi)) == 0)
		diele("RB_INSERTBANDW leftbar");
#endif
	SendMessageW(leftbar, TB_AUTOSIZE, 0, 0);
	ShowWindow(leftbar, SW_SHOW);
}

LRESULT CALLBACK wndproc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg) {
	case WM_CREATE:
		onWM_CREATE(hwnd);
		break;
	case WM_CLOSE:
		PostQuitMessage(0);
		break;
	}
	return DefWindowProcW(hwnd, uMsg, wParam, lParam);
}

EXTERN_C IMAGE_DOS_HEADER __ImageBase;

int main(void)
{
	STARTUPINFOW si;
	int nCmdShow;
	INITCOMMONCONTROLSEX icc;
	HICON hDefaultIcon;
	HCURSOR hDefaultCursor;
	WNDCLASSW wc;
	HWND mainwin;
	MSG msg;

	hInstance = (HINSTANCE) (&__ImageBase);
	nCmdShow = SW_SHOWDEFAULT;
	GetStartupInfoW(&si);
	if ((si.dwFlags & STARTF_USESHOWWINDOW) != 0)
		nCmdShow = si.wShowWindow;

	ZeroMemory(&icc, sizeof (INITCOMMONCONTROLSEX));
	icc.dwSize = sizeof (INITCOMMONCONTROLSEX);
	icc.dwICC = ICC_STANDARD_CLASSES | ICC_BAR_CLASSES | ICC_COOL_CLASSES;
	if (InitCommonControlsEx(&icc) == 0)
		diele("InitCommonControlsEx()");

	hDefaultIcon = LoadIconW(NULL, IDI_APPLICATION);
	if (hDefaultIcon == NULL)
		diele("LoadIconW(IDI_APPLICATION)");
	hDefaultCursor = LoadCursorW(NULL, IDC_ARROW);
	if (hDefaultCursor == NULL)
		diele("LoadCursorW(IDC_ARROW)");

	ZeroMemory(&wc, sizeof (WNDCLASSW));
	wc.lpszClassName = L"mainwin";
	wc.lpfnWndProc = wndproc;
	wc.hInstance = hInstance;
	wc.hIcon = hDefaultIcon;
	wc.hCursor = hDefaultCursor;
	wc.hbrBackground = (HBRUSH) (COLOR_BTNFACE + 1);
	if (RegisterClassW(&wc) == 0)
		diele("RegisterClassW()");

	mainwin = CreateWindowExW(0,
		L"mainwin", L"Main Window",
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT,
		CW_USEDEFAULT, CW_USEDEFAULT,
		NULL, NULL, hInstance, NULL);
	if (mainwin == NULL)
		diele("CreateWindowExW(L\"mainwin\")");

	ShowWindow(mainwin, nCmdShow);
	if (UpdateWindow(mainwin) == 0)
		diele("UpdateWindow()");

	for (;;) {
		int res;

		res = GetMessageW(&msg, NULL, 0, 0);
		if (res < 0)
			diele("GetMessageW()");
		if (res == 0)
			break;
		TranslateMessage(&msg);
		DispatchMessageW(&msg);
	}
	return 0;
}
