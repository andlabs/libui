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

// cl winrebarexplorertheme.cpp -MD -link user32.lib kernel32.lib gdi32.lib comctl32.lib uxtheme.lib msimg32.lib windows.res

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
HWND toolbarCombo;
HWND toolbarTransparentCheckbox;

HICON helpIcon;
HIMAGELIST helpList;

#define toolbarStyles (WS_CHILD | CCS_NODIVIDER | CCS_NOPARENTALIGN | CCS_NORESIZE | TBSTYLE_FLAT | TBSTYLE_LIST)

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

// TODO check errors
// TODO extract colors from the theme
LRESULT customDrawExplorer(NMCUSTOMDRAW *nm)
{
	static TRIVERTEX vertices[] = {
		{ 0, 0, 4 << 8, 80 << 8, 130 << 8, 255 << 8 },
		{ 0, 0, 17 << 8, 101 << 8, 132 << 8, 255 << 8 },
		{ 0, 0, 17 << 8, 101 << 8, 132 << 8, 255 << 8 },
		{ 0, 0, 29 << 8, 121 << 8, 134 << 8, 255 << 8 },
	};
	static GRADIENT_RECT gr[2] = {
		{ 0, 1 },
		{ 2, 3 },
	};
	RECT r;
	HTHEME theme;

	if (nm->dwDrawStage != CDDS_PREPAINT)
		return CDRF_DODEFAULT | TBCDRF_NOOFFSET;
	if (nm->hdr.hwndFrom == rebar) {
	GetClientRect(nm->hdr.hwndFrom, &r);
	vertices[0].x = nm->rc.left;
	vertices[0].y = 0;
	vertices[1].x = nm->rc.right;
	vertices[1].y = (r.bottom - r.top) / 2;
	vertices[2].x = nm->rc.left;
	vertices[2].y = (r.bottom - r.top) / 2;
	vertices[3].x = nm->rc.right;
	vertices[3].y = r.bottom - r.top;
	GradientFill(nm->hdc, vertices, 4, (PVOID) gr, 2, GRADIENT_FILL_RECT_V);
	theme = OpenThemeData(nm->hdr.hwndFrom, L"CommandModule");
	DrawThemeBackground(theme, nm->hdc,
		1, 0,
		&r, &(nm->rc));
	CloseThemeData(theme);
	}
	return CDRF_NOTIFYITEMDRAW;
}

LRESULT customDraw7(NMCUSTOMDRAW *nm)
{
	return CDRF_DODEFAULT;
}

static struct {
	const WCHAR *text;
	LRESULT (*handle)(NMCUSTOMDRAW *nm);
} drawmodes[] = {
	{ L"SetWindowTheme()", NULL },
	{ L"Custom Draw Explorer", customDrawExplorer },
	{ NULL, NULL },
};

int drawmode = 0;

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

static WCHAR *toolbarThemes[] = {
	L"NULL",
	L"",
	L"Alternate",
	L"BB",
	L"BBComposited",
	L"Communications",
	L"ExplorerMenu",
	L"Go",
	L"GoComposited",
	L"InactiveBB",
	L"InactiveBBComposited",
	L"InactiveGo",
	L"InactiveGoComposited",
	L"InfoPaneToolbar",
	L"LVPopup",
	L"LVPopupBottom",
	L"MaxBB",
	L"MaxBBComposited",
	L"MaxGo",
	L"MaxGoComposited",
	L"MaxInactiveBB",
	L"MaxInactiveBBComposited",
	L"MaxInactiveGo",
	L"MaxInactiveGoComposited",
	L"Media",
	L"Placesbar",
	L"SearchButton",
	L"SearchButtonComposited",
	L"StartMenu",
	L"TaskBar",
	L"TaskBarComposited",
	L"TaskBarVert",
	L"TaskBarVertComposited",
	L"Toolbar",
	L"ToolbarStyle",
	L"TrayNotify",
	L"TrayNotifyComposited",
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
		toolbarStyles | TBSTYLE_TRANSPARENT,
		0, 0, 0, 0,
		hwnd, (HMENU) 101, hInstance, NULL);
	if (leftbar == NULL)
		diele("CreateWindowExW(TOOLBARCLASSNAMEW) leftbar");
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
//	if (SendMessageW(rebar, RB_INSERTBANDW, (WPARAM) (-1), (LPARAM) (&rbi)) == 0)
//		diele("RB_INSERTBANDW leftbar");

	rightbar = CreateWindowExW(0,
		TOOLBARCLASSNAMEW, NULL,
		(toolbarStyles & ~TBSTYLE_LIST) | TBSTYLE_TRANSPARENT,
		0, 0, 0, 0,
		hwnd, (HMENU) 102, hInstance, NULL);
	if (rightbar == NULL)
		diele("CreateWindowExW(TOOLBARCLASSNAMEW) rightbar");
	SendMessageW(rightbar, TB_BUTTONSTRUCTSIZE, sizeof (TBBUTTON), 0);
	// TODO check error
	SendMessageW(rightbar, TB_SETBITMAPSIZE, 0, MAKELPARAM(GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON)));
	SendMessageW(rightbar, TB_SETIMAGELIST, 0, (LPARAM) helpList);
	ZeroMemory(tbb, 5 * sizeof (TBBUTTON));
	tbb[0].iBitmap = 0;
	tbb[0].idCommand = 0;
	tbb[0].fsState = TBSTATE_ENABLED;
	tbb[0].fsStyle = BTNS_BUTTON;
	if (SendMessageW(rightbar, TB_ADDBUTTONSW, 1, (LPARAM) tbb) == FALSE)
		diele("TB_ADDBUTTONSW");

	tbbtnsize = (DWORD) SendMessageW(rightbar, TB_GETBUTTONSIZE, 0, 0);
	tbsizex = LOWORD(tbbtnsize);
	tbsizey = HIWORD(tbbtnsize);

	ZeroMemory(&rbi, sizeof (REBARBANDINFOW));
	rbi.cbSize = sizeof (REBARBANDINFOW);
	rbi.fMask = RBBIM_CHILD | RBBIM_STYLE | RBBIM_SIZE | RBBIM_CHILDSIZE;
	rbi.fStyle = RBBS_NOGRIPPER | RBBS_HIDETITLE;
	rbi.hwndChild = rightbar;
	rbi.cx = tbsizex;
	rbi.cyChild = tbsizey;
	rbi.cxMinChild = tbsizex;
	rbi.cyMinChild = tbsizey;
	if (SendMessageW(rebar, RB_INSERTBANDW, (WPARAM) (-1), (LPARAM) (&rbi)) == 0)
		diele("RB_INSERTBANDW rightbar");

	buttonx = 10;
	buttony = 40;
#define buttonwid 200
#define buttonht 25
	for (i = 0; drawmodes[i].text != NULL; i++) {
		button = CreateWindowExW(0,
			L"BUTTON", drawmodes[i].text,
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
	comboy += buttonht + 5;
	toolbarCombo = CreateWindowExW(WS_EX_CLIENTEDGE,
		L"COMBOBOX", L"",
		WS_CHILD | WS_VISIBLE | CBS_DROPDOWN,
		combox, comboy,
		buttonwid, buttonht,
		hwnd, (HMENU) 301, hInstance, NULL);
	if (toolbarCombo == NULL)
		diele("CreateWindowExW(L\"COMBOBOX\")");
	for (i = 0; toolbarThemes[i] != NULL; i++)
		// TODO check error
		SendMessageW(toolbarCombo, CB_ADDSTRING, 0, (LPARAM) (toolbarThemes[i]));
	comboy += buttonht + 5;
	toolbarTransparentCheckbox = CreateWindowExW(0,
		L"BUTTON", L"Transparent toolbar",
		WS_CHILD | WS_VISIBLE | BS_CHECKBOX,
		combox, comboy,
		buttonwid, buttonht,
		hwnd, (HMENU) 302, hInstance, NULL);
	if (toolbarTransparentCheckbox == NULL)
		diele("CreateWindowExW(L\"BUTTON\")");
	SendMessage(toolbarTransparentCheckbox, BM_SETCHECK, BST_CHECKED, 0);
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
	BOOL invalidate = FALSE;
	WPARAM check;

	switch (wParam) {
	case MAKEWPARAM(300, CBN_SELCHANGE):
		n = SendMessageW(rebarCombo, CB_GETCURSEL, 0, 0);
		selRebar = rebarThemes[n];
		changeRebar = TRUE;
		break;
	case MAKEWPARAM(301, CBN_SELCHANGE):
		n = SendMessageW(toolbarCombo, CB_GETCURSEL, 0, 0);
		selToolbar = toolbarThemes[n];
		changeToolbar = TRUE;
		break;
	case MAKEWPARAM(200, BN_CLICKED):
		drawmode = 0;
		n = SendMessageW(rebarCombo, WM_GETTEXTLENGTH, 0, 0);
		bufRebar = new WCHAR[n + 1];
		GetWindowTextW(rebarCombo, bufRebar, n + 1);
		n = SendMessageW(toolbarCombo, WM_GETTEXTLENGTH, 0, 0);
		bufToolbar = new WCHAR[n + 1];
		GetWindowTextW(toolbarCombo, bufToolbar, n + 1);
		selRebar = bufRebar;
		selToolbar = bufToolbar;
		changeRebar = TRUE;
		changeToolbar = TRUE;
		break;
	case MAKEWPARAM(201, BN_CLICKED):
		drawmode = 1;
		invalidate = TRUE;
		break;
	case MAKEWPARAM(302, BN_CLICKED):
		ShowWindow(leftbar, SW_HIDE);
		check = BST_CHECKED;
		if (SendMessage(toolbarTransparentCheckbox, BM_GETCHECK, 0, 0) == BST_CHECKED)
			check = BST_UNCHECKED;
		SendMessage(toolbarTransparentCheckbox, BM_SETCHECK, check, 0);
		if (check == BST_CHECKED)
			SendMessageW(leftbar, TB_SETSTYLE, 0, toolbarStyles | TBSTYLE_TRANSPARENT);
		else
			SendMessageW(leftbar, TB_SETSTYLE, 0, toolbarStyles);
		ShowWindow(leftbar, SW_SHOW);
		break;
	}
	if (changeRebar) {
		if (selRebar != NULL && wcscmp(selRebar, L"NULL") == 0)
			selRebar = NULL;
		if (selRebar != NULL && *selRebar != L'\0')
			SendMessageW(rebar, RB_SETWINDOWTHEME, 0, (LPARAM) selRebar);
		else
			SetWindowTheme(rebar, selRebar, selRebar);
		invalidate = TRUE;
	}
	if (changeToolbar) {
		if (selToolbar != NULL && wcscmp(selToolbar, L"NULL") == 0)
			selToolbar = NULL;
		if (selToolbar != NULL && *selToolbar != L'\0')
			SendMessageW(leftbar, TB_SETWINDOWTHEME, 0, (LPARAM) selToolbar);
		else
			SetWindowTheme(leftbar, selToolbar, selToolbar);
		invalidate = TRUE;
	}
	if (invalidate)
		InvalidateRect(hwnd, NULL, TRUE);
	if (bufRebar != NULL)
		delete[] bufRebar;
	if (bufToolbar != NULL)
		delete[] bufToolbar;
}

LRESULT CALLBACK wndproc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	NMHDR *nm = (NMHDR *) lParam;

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
	case WM_NOTIFY:
		switch (nm->code) {
		case NM_CUSTOMDRAW:
			if (nm->hwndFrom != rebar)
				break;
			if (drawmode == 0)
				break;
			return (*(drawmodes[drawmode].handle))((NMCUSTOMDRAW *) nm);
		}
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

	helpIcon = (HICON) LoadImageW(NULL, IDI_QUESTION, IMAGE_ICON,
		GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), LR_DEFAULTCOLOR | LR_SHARED);
	if (helpIcon == NULL)
		diele("LoadImageW(IDI_QUESTION)");
	helpList = ImageList_Create(GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON),
		ILC_COLOR32, 0, 1);
	if (helpList == NULL)
		diele("ImageList_Create()");
	if (ImageList_ReplaceIcon(helpList, -1, helpIcon) == -1)
		diele("ImageList_ReplaceIcon()");

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
