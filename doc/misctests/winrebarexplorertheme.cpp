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
HWND rebarCombo;

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

static const WCHAR *buttons[] = {
	L"SetWindowTheme()",
	L"Custom Draw Vista",
	L"Custom Draw 7",
	NULL,
};

static const WCHAR *rebarThemes[] = {
	L"NULL",
	L"",
	L"AlternateRebar",
	L"BrowserTabBar",
	L"Communications",
	L"Default",
	L"ExplorerBar",
	L"Help",
	L"InactiveNavbar",
	L"InactiveNavbarComposited",
	L"ITBarBase",
	L"MaxInactiveNavbar",
	L"MaxInactiveNavbarComposited",
	L"MaxNavbar",
	L"MaxNavbarComposited",
	L"Media",
	L"Navbar",
	L"NavbarBase",
	L"NavbarComposited",
	L"NavbarNonTopmost",
	L"Rebar",
	L"RebarStyle",
	L"TaskBar",
	L"TaskBarComposited",
	NULL,
};

// TODO toolbarThemes

void onWM_CREATE(HWND hwnd)
{
	TBBUTTON tbb[5];
	RECT btnrect;
	DWORD tbbtnsize;
	LONG tbsizex, tbsizey;
	REBARBANDINFOW rbi;
	HWND button;
	LONG buttonx, buttony;
	LONG combox, comboy;
	int i;

	rebar = CreateWindowExW(0,
		REBARCLASSNAMEW, NULL,
		WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | CCS_NODIVIDER | CCS_TOP,
		0, 0, 0, 0,
		hwnd, (HMENU) 100, hInstance, NULL);
	if (rebar == NULL)
		diele("CreateWindowExW(REBARCLASSNAMEW)");

	leftbar = CreateWindowExW(0,
		TOOLBARCLASSNAMEW, NULL,
		WS_CHILD | CCS_NODIVIDER | CCS_NOPARENTALIGN | CCS_NORESIZE | TBSTYLE_FLAT | TBSTYLE_LIST | TBSTYLE_TRANSPARENT,
		0, 0, 0, 0,
		hwnd, (HMENU) 101, hInstance, NULL);
	SendMessageW(leftbar, TB_BUTTONSTRUCTSIZE, sizeof (TBBUTTON), 0);
	// I_IMAGENONE causes the button text to be left-aligned; don't use it
	if (SendMessageW(leftbar, TB_SETBITMAPSIZE, 0, 0) == FALSE)
		diele("TB_SETBITMAPSIZE");
	SendMessageW(leftbar, TB_SETEXTENDEDSTYLE, 0, TBSTYLE_EX_DRAWDDARROWS | TBSTYLE_EX_HIDECLIPPEDBUTTONS | TBSTYLE_EX_MIXEDBUTTONS);
	// TODO this *should* be DIPs...
	// TODO figure out where the *2 is documented
	SendMessageW(leftbar, TB_SETPADDING, 0, MAKELONG(6 * 2, 5 * 2));
	ZeroMemory(tbb, 5 * sizeof (TBBUTTON));
	for (i = 0; i < 5; i++) {
		tbb[i].iBitmap = 0;
		tbb[i].idCommand = i;
		tbb[i].fsState = TBSTATE_ENABLED;
		tbb[i].fsStyle = BTNS_AUTOSIZE | BTNS_BUTTON | BTNS_NOPREFIX | BTNS_SHOWTEXT;
		if (leftbarButtons[i].dropdown)
			tbb[i].fsStyle |= BTNS_DROPDOWN | BTNS_WHOLEDROPDOWN;
		tbb[i].iString = (INT_PTR) (leftbarButtons[i].text);
	}
	if (SendMessageW(leftbar, TB_ADDBUTTONSW, 5, (LPARAM) tbb) == FALSE)
		diele("TB_ADDBUTTONSW");

	tbsizex = 0;
	for (i = 0; i < 5; i++) {
		if (SendMessageW(leftbar, TB_GETITEMRECT, (WPARAM) i, (LPARAM) (&btnrect)) == FALSE)
			diele("TB_GETITEMRECT");
		tbsizex += btnrect.right - btnrect.left;
	}
	tbbtnsize = (DWORD) SendMessageW(leftbar, TB_GETBUTTONSIZE, 0, 0);
	tbsizey = HIWORD(tbbtnsize);

	ZeroMemory(&rbi, sizeof (REBARBANDINFOW));
	rbi.cbSize = sizeof (REBARBANDINFOW);
	rbi.fMask = RBBIM_CHILD | RBBIM_STYLE | RBBIM_SIZE | RBBIM_CHILDSIZE | RBBIM_IDEALSIZE;
	rbi.fStyle = RBBS_NOGRIPPER | RBBS_USECHEVRON | RBBS_HIDETITLE;
	rbi.hwndChild = leftbar;
	rbi.cx = tbsizex;
	rbi.cyChild = tbsizey;
	rbi.cxMinChild = 0;
	rbi.cyMinChild = tbsizey;
	rbi.cxIdeal = tbsizex;
	if (SendMessageW(rebar, RB_INSERTBANDW, (WPARAM) (-1), (LPARAM) (&rbi)) == 0)
		diele("RB_INSERTBANDW leftbar");

	buttonx = 10;
	buttony = 40;
#define buttonwid 200
#define buttonht 25
	for (i = 0; buttons[i] != NULL; i++) {
		button = CreateWindowExW(0,
			L"BUTTON", buttons[i],
			WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
			buttonx, buttony,
			buttonwid, buttonht,
			hwnd, (HMENU) (200 + i), hInstance, NULL);
		if (button == NULL)
			diele("CreateWIndowExW(L\"BUTTON\")");
		if (i == 0) {
			combox = buttonx + buttonwid + 5;
			comboy = buttony;
		}
		buttony += buttonht + 5;
	}
	rebarCombo = CreateWindowExW(WS_EX_CLIENTEDGE,
		L"COMBOBOX", L"",
		WS_CHILD | WS_VISIBLE | CBS_DROPDOWN,
		combox, comboy,
		buttonwid, buttonht,
		hwnd, (HMENU) 300, hInstance, NULL);
	if (rebarCombo == NULL)
		diele("CreateWindowExW(L\"COMBOBOX\")");
	for (i = 0; rebarThemes[i] != NULL; i++)
		// TODO check error
		SendMessageW(rebarCombo, CB_ADDSTRING, 0, (LPARAM) (rebarThemes[i]));
}

// TODO it seems like I shouldn't have to do this?
void repositionRebar(HWND hwnd)
{
	RECT win, rb;

	if (GetClientRect(hwnd, &win) == 0)
		diele("GetClientRect()");
	if (GetWindowRect(rebar, &rb) == 0)
		diele("GetWindowRect()");
	if (SetWindowPos(rebar, NULL,
		0, 0, win.right - win.left, rb.bottom - rb.top,
		SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOOWNERZORDER | SWP_NOZORDER) == 0)
		diele("SetWindowPos()");
	SendMessageW(rebar, RB_SETBANDWIDTH, 0, win.right - win.left);
}

// TODO check errors
void handleEvents(HWND hwnd, WPARAM wParam)
{
	LRESULT n;
	const WCHAR *selRebar = NULL, *selToolbar = NULL;
	WCHAR *bufRebar = NULL, *bufToolbar = NULL;
	BOOL changeRebar = FALSE, changeToolbar = FALSE;

	switch (wParam) {
	case MAKEWPARAM(300, CBN_SELCHANGE):
		n = SendMessageW(rebarCombo, CB_GETCURSEL, 0, 0);
		selRebar = rebarThemes[n];
		changeRebar = TRUE;
		break;
	case MAKEWPARAM(200, BN_CLICKED):
		n = SendMessageW(rebarCombo, WM_GETTEXTLENGTH, 0, 0);
		bufRebar = new WCHAR[n + 1];
		GetWindowTextW(rebarCombo, bufRebar, n + 1);
		selRebar = bufRebar;
		selToolbar = bufRebar;
		changeRebar = TRUE;
		changeToolbar = TRUE;
		break;
	}
	if (changeRebar) {
		if (selRebar != NULL && wcscmp(selRebar, L"NULL") == 0)
			selRebar = NULL;
		if (selRebar != NULL && *selRebar != L'\0')
			SendMessageW(rebar, RB_SETWINDOWTHEME, 0, (LPARAM) selRebar);
		else
			SetWindowTheme(rebar, selRebar, selRebar);
		InvalidateRect(hwnd, NULL, TRUE);
	}
	if (changeToolbar) {
		if (selToolbar != NULL && wcscmp(selToolbar, L"NULL") == 0)
			selToolbar = NULL;
		if (selToolbar != NULL && *selToolbar != L'\0')
			SendMessageW(leftbar, TB_SETWINDOWTHEME, 0, (LPARAM) selToolbar);
		else
			SetWindowTheme(leftbar, selToolbar, selToolbar);
		InvalidateRect(hwnd, NULL, TRUE);
	}
	if (bufRebar != NULL)
		delete[] bufRebar;
	if (bufToolbar != NULL)
		delete[] bufToolbar;
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
	case WM_SIZE:
		repositionRebar(hwnd);
		break;
	case WM_COMMAND:
		handleEvents(hwnd, wParam);
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
