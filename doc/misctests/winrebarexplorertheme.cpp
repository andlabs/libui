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
#include <windowsx.h>
#include <stdio.h>
#include <stdlib.h>

// cl winrebarexplorertheme.cpp -MT -link user32.lib kernel32.lib gdi32.lib comctl32.lib uxtheme.lib msimg32.lib windows.res

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

HICON shieldIcon;
HICON applicationIcon;
HICON helpIcon;
HIMAGELIST rightList;

#define toolbarStyles (WS_CHILD | CCS_NODIVIDER | CCS_NOPARENTALIGN | CCS_NORESIZE | TBSTYLE_FLAT)

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
void drawExplorerBackground(HTHEME theme, HDC dc, RECT *rcWindow, RECT *rcPaint)
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

	vertices[0].x = rcPaint->left;
	vertices[0].y = 0;
	vertices[1].x = rcPaint->right;
	vertices[1].y = (rcWindow->bottom - rcWindow->top) / 2;
	vertices[2].x = rcPaint->left;
	vertices[2].y = (rcWindow->bottom - rcWindow->top) / 2;
	vertices[3].x = rcPaint->right;
	vertices[3].y = rcWindow->bottom - rcWindow->top;
	GradientFill(dc, vertices, 4, (PVOID) gr, 2, GRADIENT_FILL_RECT_V);
	DrawThemeBackground(theme, dc,
		1, 0,
		rcWindow, rcPaint);
}

// TODO check errors
void drawExplorerChevron(HTHEME theme, HDC dc, HWND rebar, WPARAM band, RECT *rcPaint)
{
	REBARBANDINFOW rbi;
	RECT r;
	int state;

	ZeroMemory(&rbi, sizeof (REBARBANDINFOW));
	rbi.cbSize = sizeof (REBARBANDINFOW);
	rbi.fMask = RBBIM_CHILD | RBBIM_CHEVRONLOCATION | RBBIM_CHEVRONSTATE;
	SendMessageW(rebar, RB_GETBANDINFOW, band, (LPARAM) (&rbi));
	if ((rbi.uChevronState & STATE_SYSTEM_INVISIBLE) != 0)
		return;
	state = 1;
	// TODO check if this is correct
	if ((rbi.uChevronState & STATE_SYSTEM_FOCUSED) != 0)
		state = 4;
	if ((rbi.uChevronState & STATE_SYSTEM_HOTTRACKED) != 0)
		state = 2;
	if ((rbi.uChevronState & STATE_SYSTEM_PRESSED) != 0)
		state = 3;
	r = rbi.rcChevronLocation;
	// TODO commctrl.h says this should be correct for the chevron rect, but it's not?
//	MapWindowRect(rbi.hwndChild, rebar, &r);
	DrawThemeBackground(theme, dc,
		3, state,
		&r, rcPaint);
	DrawThemeBackground(theme, dc,
		7, 1,
		&r, rcPaint);
}

// TODO check errors
LRESULT customDrawExplorerRebar(NMCUSTOMDRAW *nm)
{
	HTHEME theme;
	RECT r;

	if (nm->dwDrawStage != CDDS_PREPAINT)
		return CDRF_DODEFAULT;
	theme = OpenThemeData(nm->hdr.hwndFrom, L"CommandModule");
	GetClientRect(nm->hdr.hwndFrom, &r);
	drawExplorerBackground(theme, nm->hdc, &r, &(nm->rc));
	// TODO dwItemSpec is often invalid?!
	drawExplorerChevron(theme, nm->hdc, nm->hdr.hwndFrom, 0, &(nm->rc));
	CloseThemeData(theme);
	return CDRF_SKIPDEFAULT;
}

// TODO check errors
LRESULT customDrawExplorerToolbar(NMTBCUSTOMDRAW *nm)
{
	HWND toolbar, rebar;
	WPARAM itemIndex;
	TBBUTTON tbb;
	HTHEME theme;
	RECT r;
	int part, state;

	toolbar = nm->nmcd.hdr.hwndFrom;
	switch (nm->nmcd.dwDrawStage) {
	case CDDS_PREPAINT:
		theme = OpenThemeData(toolbar, L"CommandModule");
		rebar = GetParent(toolbar);
		GetWindowRect(rebar, &r);
		MapWindowRect(NULL, toolbar, &r);
		drawExplorerBackground(theme, nm->nmcd.hdc, &r, &(nm->nmcd.rc));
		CloseThemeData(theme);
		return CDRF_NOTIFYITEMDRAW;
	case CDDS_ITEMPREPAINT:
		itemIndex = (WPARAM) SendMessageW(toolbar, TB_COMMANDTOINDEX, nm->nmcd.dwItemSpec, 0);
		ZeroMemory(&tbb, sizeof (TBBUTTON));
		SendMessageW(toolbar, TB_GETBUTTON, itemIndex, (LPARAM) (&tbb));
		theme = OpenThemeData(toolbar, L"CommandModule");
		part = 3;
		if ((tbb.fsStyle & BTNS_DROPDOWN) != 0)
			part = 4;
		state = 1;
		// TODO this doesn't work; both keyboard and mouse are listed as HOT
		if ((nm->nmcd.uItemState & CDIS_FOCUS) != 0)
			state = 4;
		if ((nm->nmcd.uItemState & CDIS_HOT) != 0)
			state = 2;
		if ((nm->nmcd.uItemState & CDIS_SELECTED) != 0)
			state = 3;
		SendMessageW(toolbar, TB_GETITEMRECT, itemIndex, (LPARAM) (&r));
		DrawThemeBackground(theme, nm->nmcd.hdc,
			3, state,
			&r, &(nm->nmcd.rc));
		CloseThemeData(theme);
		return TBCDRF_NOBACKGROUND;
	}
	return CDRF_DODEFAULT;
}

static struct {
	const WCHAR *text;
	LRESULT (*handleRebar)(NMCUSTOMDRAW *nm);
	LRESULT (*handleToolbar)(NMTBCUSTOMDRAW *nm);
} drawmodes[] = {
	{ L"SetWindowTheme()", NULL, NULL },
	{ L"Custom Draw Explorer", customDrawExplorerRebar, customDrawExplorerToolbar },
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
		WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | CCS_NODIVIDER | CCS_TOP | RBS_FIXEDORDER,
		0, 0, 0, 0,
		hwnd, (HMENU) 100, hInstance, NULL);
	if (rebar == NULL)
		diele("CreateWindowExW(REBARCLASSNAMEW)");

	leftbar = CreateWindowExW(0,
		TOOLBARCLASSNAMEW, NULL,
		toolbarStyles | TBSTYLE_LIST | TBSTYLE_TRANSPARENT,
		0, 0, 0, 0,
		hwnd, (HMENU) 101, hInstance, NULL);
	if (leftbar == NULL)
		diele("CreateWindowExW(TOOLBARCLASSNAMEW) leftbar");
	SendMessageW(leftbar, TB_BUTTONSTRUCTSIZE, sizeof (TBBUTTON), 0);
	// I_IMAGENONE causes the button text to be left-aligned; don't use it
//	if (SendMessageW(leftbar, TB_SETBITMAPSIZE, 0, 0) == FALSE)
//		diele("TB_SETBITMAPSIZE");
	SendMessageW(leftbar, TB_SETEXTENDEDSTYLE, 0, TBSTYLE_EX_DRAWDDARROWS | TBSTYLE_EX_HIDECLIPPEDBUTTONS | TBSTYLE_EX_MIXEDBUTTONS);
	// TODO this *should* be DIPs...
	// TODO figure out where the *2 is documented
//	SendMessageW(leftbar, TB_SETPADDING, 0, MAKELPARAM(6 * 2, 5 * 2));
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
	rbi.fMask = RBBIM_CHILD | RBBIM_STYLE | RBBIM_SIZE | RBBIM_CHILDSIZE | RBBIM_IDEALSIZE | RBBIM_ID;
	rbi.fStyle = RBBS_NOGRIPPER | RBBS_CHILDEDGE | RBBS_USECHEVRON | RBBS_HIDETITLE;
	rbi.hwndChild = leftbar;
	rbi.cx = tbsizex;
	rbi.cyChild = tbsizey;
	rbi.cxMinChild = 0;
	rbi.cyMinChild = tbsizey;
	rbi.cxIdeal = tbsizex;
	rbi.wID = 0;
	if (SendMessageW(rebar, RB_INSERTBANDW, (WPARAM) (-1), (LPARAM) (&rbi)) == 0)
		diele("RB_INSERTBANDW leftbar");

	rightbar = CreateWindowExW(0,
		TOOLBARCLASSNAMEW, NULL,
		toolbarStyles | TBSTYLE_TRANSPARENT,
		0, 0, 0, 0,
		hwnd, (HMENU) 102, hInstance, NULL);
	if (rightbar == NULL)
		diele("CreateWindowExW(TOOLBARCLASSNAMEW) rightbar");
	SendMessageW(rightbar, TB_BUTTONSTRUCTSIZE, sizeof (TBBUTTON), 0);
	SendMessageW(rightbar, TB_SETIMAGELIST, 0, (LPARAM) rightList);
	SendMessageW(rightbar, TB_SETEXTENDEDSTYLE, 0, TBSTYLE_EX_DRAWDDARROWS);
	// TODO this *should* be DIPs...
	// TODO figure out where the *2 is documented
//	SendMessageW(rightbar, TB_SETPADDING, 0, MAKELPARAM(6 * 2, 5 * 2));
	ZeroMemory(tbb, 5 * sizeof (TBBUTTON));
	tbb[0].iBitmap = 0;
	tbb[0].idCommand = 0;
	tbb[0].fsState = TBSTATE_ENABLED;
	tbb[0].fsStyle = BTNS_AUTOSIZE | BTNS_BUTTON | BTNS_DROPDOWN;
	tbb[1].iBitmap = 1;
	tbb[1].idCommand = 1;
	tbb[1].fsState = TBSTATE_ENABLED;
	tbb[1].fsStyle = BTNS_AUTOSIZE | BTNS_BUTTON;
	tbb[2].iBitmap = 2;
	tbb[2].idCommand = 2;
	tbb[2].fsState = TBSTATE_ENABLED;
	tbb[2].fsStyle = BTNS_AUTOSIZE | BTNS_BUTTON;
	if (SendMessageW(rightbar, TB_ADDBUTTONSW, 3, (LPARAM) tbb) == FALSE)
		diele("TB_ADDBUTTONSW");
	// TODO check error
	// TODO figure out why this works here but not elsewhere
//	SendMessageW(rightbar, TB_SETBUTTONSIZE, 0, 0);

	tbsizex = 0;
	for (i = 0; i < 3; i++) {
		if (SendMessageW(rightbar, TB_GETITEMRECT, (WPARAM) i, (LPARAM) (&btnrect)) == FALSE)
			diele("TB_GETITEMRECT");
		tbsizex += btnrect.right - btnrect.left;
	}
	tbbtnsize = (DWORD) SendMessageW(rightbar, TB_GETBUTTONSIZE, 0, 0);
	tbsizey = HIWORD(tbbtnsize);

	ZeroMemory(&rbi, sizeof (REBARBANDINFOW));
	rbi.cbSize = sizeof (REBARBANDINFOW);
	rbi.fMask = RBBIM_CHILD | RBBIM_STYLE | RBBIM_SIZE | RBBIM_CHILDSIZE | RBBIM_ID;
	rbi.fStyle = RBBS_NOGRIPPER | RBBS_HIDETITLE;
	rbi.hwndChild = rightbar;
	rbi.cx = tbsizex;
	rbi.cyChild = tbsizey;
	rbi.cxMinChild = tbsizex;
	rbi.cyMinChild = tbsizey;
	rbi.wID = 1;
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
	button = CreateWindowExW(0,
		L"BUTTON", L"Give Toolbar Focus",
		WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
		buttonx, buttony,
		buttonwid, buttonht,
		hwnd, (HMENU) (200 + i), hInstance, NULL);
	if (button == NULL)
		diele("CreateWIndowExW(L\"BUTTON\")");
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
			SendMessageW(leftbar, TB_SETSTYLE, 0, toolbarStyles | TBSTYLE_LIST | TBSTYLE_TRANSPARENT);
		else
			SendMessageW(leftbar, TB_SETSTYLE, 0, toolbarStyles | TBSTYLE_LIST);
		ShowWindow(leftbar, SW_SHOW);
		break;
	case MAKEWPARAM(202, BN_CLICKED):
		SetFocus(leftbar);
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
		if (selToolbar != NULL && *selToolbar != L'\0') {
			SendMessageW(leftbar, TB_SETWINDOWTHEME, 0, (LPARAM) selToolbar);
			SendMessageW(rightbar, TB_SETWINDOWTHEME, 0, (LPARAM) selToolbar);
		} else {
			SetWindowTheme(leftbar, selToolbar, selToolbar);
			SetWindowTheme(rightbar, selToolbar, selToolbar);
		}
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
			if (drawmode == 0)
				break;
			if (nm->hwndFrom == rebar)
				return (*(drawmodes[drawmode].handleRebar))((NMCUSTOMDRAW *) nm);
			else if (nm->hwndFrom == leftbar || nm->hwndFrom == rightbar)
				return (*(drawmodes[drawmode].handleToolbar))((NMTBCUSTOMDRAW *) nm);
			break;
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
	HRESULT hr;

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

	hr = LoadIconMetric(NULL, IDI_SHIELD, LIM_SMALL, &shieldIcon);
	if (hr != S_OK)
		diehr("LoadIconMetric(IDI_SHIELD)", hr);
	hr = LoadIconMetric(NULL, IDI_APPLICATION, LIM_SMALL, &applicationIcon);
	if (hr != S_OK)
		diehr("LoadIconMetric(IDI_APPLICATION)", hr);
	hr = LoadIconMetric(NULL, IDI_QUESTION, LIM_SMALL, &helpIcon);
	if (hr != S_OK)
		diehr("LoadIconMetric(IDI_QUESTION)", hr);
	rightList = ImageList_Create(GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON),
		ILC_COLOR32, 0, 3);
	if (rightList == NULL)
		diele("ImageList_Create()");
	if (ImageList_ReplaceIcon(rightList, -1, shieldIcon) == -1)
		diele("ImageList_ReplaceIcon(IDI_SHIELD)");
	if (ImageList_ReplaceIcon(rightList, -1, applicationIcon) == -1)
		diele("ImageList_ReplaceIcon(IDI_APPLICATION)");
	if (ImageList_ReplaceIcon(rightList, -1, helpIcon) == -1)
		diele("ImageList_ReplaceIcon(IDI_QUESTION)");

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
