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

// cl winbuttonexplorertheme.cpp -MT -link user32.lib kernel32.lib gdi32.lib comctl32.lib uxtheme.lib msimg32.lib windows.res

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
HWND leftButtons[5];
HWND rightButtons[3];

HICON shieldIcon;
HICON applicationIcon;
HICON helpIcon;
HIMAGELIST rightList;

HTHEME theme = NULL;
HTHEME textstyleTheme = NULL;
HIMAGELIST dropdownArrowList = NULL;
HFONT buttonFont = NULL;

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
LRESULT drawExplorerButton(NMCUSTOMDRAW *nm)
{
	HWND button;
	RECT r;
	int part, state;
	LRESULT n;
	WCHAR *buf;

	if (nm->dwDrawStage != CDDS_PREPAINT)
		return CDRF_DODEFAULT;
	button = nm->hdr.hwndFrom;
	GetClientRect(button, &r);
	part = 3;
//TODO	if ((tbb.fsStyle & BTNS_DROPDOWN) != 0)
//TODO		part = 4;
	state = 1;
	// TODO this doesn't work; both keyboard and mouse are listed as HOT
	if ((nm->uItemState & CDIS_FOCUS) != 0)
		state = 4;
	if ((nm->uItemState & CDIS_HOT) != 0)
		state = 2;
	if ((nm->uItemState & CDIS_SELECTED) != 0)
		state = 3;
	DrawThemeParentBackground(button, nm->hdc, &(nm->rc));
	DrawThemeBackground(theme, nm->hdc,
		part, state,
		&r, &(nm->rc));
	n = SendMessageW(button, WM_GETTEXTLENGTH, 0, 0);
	buf = new WCHAR[n + 1];
	GetWindowTextW(button, buf, n + 1);
	SetBkMode(nm->hdc, TRANSPARENT);
	DrawThemeText(textstyleTheme, nm->hdc,
		4, 0,
		buf, n, DT_CENTER | DT_VCENTER | DT_SINGLELINE,
		0, &r);
	delete[] buf;
	return CDRF_SKIPDEFAULT;
}

void onWM_CREATE(HWND hwnd)
{
	int buttonx, buttony;
	int i;

	buttonx = 5;
	buttony = 5;
	for (i = 0; i < 5; i++) {
		// TODO split buttons don't support arrow navigation?
		leftButtons[i] = CreateWindowExW(0,
			L"BUTTON", leftbarButtons[i].text,
			WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_PUSHBUTTON,
			buttonx, buttony,
			150, 30,
			hwnd, (HMENU) (100 + i), hInstance, NULL);
		if (leftButtons[i] == NULL)
			diele("CreateWindowExW(L\"BUTTON\")");
		buttonx += 150;
	}
}

// TODO check errors
void updateTheme(HWND hwnd)
{
	BUTTON_IMAGELIST bim;
	HDC dc;
	SIZE size;
	HBITMAP hb;
	HTHEME buttonTheme;
	LOGFONTW lf;
	MARGINS marginOffsets;
	RECT margins;
	int i;

	if (buttonFont != NULL) {
		for (i = 0; i < 5; i++)
			SendMessageW(leftButtons[i], WM_SETFONT, (WPARAM) NULL, TRUE);
		DeleteObject(buttonFont);
		buttonFont = NULL;
	}
	if (dropdownArrowList != NULL) {
		ZeroMemory(&bim, sizeof (BUTTON_IMAGELIST));
		bim.himl = BCCL_NOGLYPH;
		for (i = 0; i < 3; i++)
			SendMessageW(leftButtons[i], BCM_SETIMAGELIST, 0, (LPARAM) (&bim));
		ImageList_Destroy(dropdownArrowList);
		dropdownArrowList = NULL;
	}
	if (textstyleTheme != NULL) {
		CloseThemeData(textstyleTheme);
		textstyleTheme = NULL;
	}
	if (theme != NULL) {
		CloseThemeData(theme);
		theme = NULL;
	}

	theme = OpenThemeData(hwnd, L"CommandModule");
	textstyleTheme = OpenThemeData(hwnd, L"TEXTSTYLE");
	dc = GetDC(hwnd);
	// TS_MIN returns 1x1 and TS_DRAW returns 0x0, so...
	GetThemePartSize(theme, dc,
		6, 0,
		NULL, TS_TRUE, &size);
	ReleaseDC(hwnd, dc);
	// TODO draw a bitmap properly
	GetThemeBitmap(theme,
		6, 0,
		0, GBF_COPY, &hb);
	dropdownArrowList = ImageList_Create(size.cx, size.cy,
		ILC_COLOR32, 0, 1);
	ImageList_Add(dropdownArrowList, hb, NULL);
	DeleteObject(hb);
	ZeroMemory(&bim, sizeof (BUTTON_IMAGELIST));
	bim.himl = dropdownArrowList;
	// TODO should be DIPs
	bim.margin.left = 1;
	bim.uAlign = BUTTON_IMAGELIST_ALIGN_RIGHT;
	for (i = 0; i < 3; i++)
		SendMessageW(leftButtons[i], BCM_SETIMAGELIST, 0, (LPARAM) (&bim));

	// TODO find the right TMT constant
	GetThemeFont(textstyleTheme, NULL,
		4, 0,
		TMT_FONT, &lf);
	buttonFont = CreateFontIndirectW(&lf);
	for (i = 0; i < 5; i++)
		SendMessageW(leftButtons[i], WM_SETFONT, (WPARAM) buttonFont, TRUE);

	buttonTheme = OpenThemeData(hwnd, L"Button");
	ZeroMemory(&marginOffsets, sizeof (MARGINS));
	GetThemeMargins(buttonTheme, NULL,
		BP_PUSHBUTTON, PBS_NORMAL,
		TMT_CONTENTMARGINS, NULL, &marginOffsets);
	CloseThemeData(buttonTheme);
	// TODO the constants should be DIPs
	margins.left = 13 - marginOffsets.cxLeftWidth;
	margins.top = 5 - marginOffsets.cyTopHeight;
	margins.right = 13 - marginOffsets.cxRightWidth;
	margins.bottom = 5 - marginOffsets.cyBottomHeight;
//	for (i = 0; i < 5; i++)
//		if (SendMessageW(leftButtons[i], BCM_SETTEXTMARGIN, 0, (LPARAM) (&margins)) == FALSE)
//			diele("BCM_SETTEXTMARGIN");
}

// TODO check errors
// TODO the width is always off by 4 pixels somehow (according to UI Automation)
// TODO the height is correct (according to UI Automation) but they don't visually match?
SIZE buttonSize(HWND button)
{
	HDC dc;
	LRESULT n;
	WCHAR *buf;
	RECT textRect;
	RECT contentRect;
	SIZE minSize;
	SIZE ret;

	dc = GetDC(button);

printf("%08I32X ", GetThemePartSize(theme, dc, 3, 1, NULL, TS_TRUE, &ret));
printf("%d %d\n", ret.cx, ret.cy);

	n = SendMessageW(button, WM_GETTEXTLENGTH, 0, 0);
	buf = new WCHAR[n + 1];
	GetWindowTextW(button, buf, n + 1);
	GetThemeTextExtent(textstyleTheme, dc,
		4, 0,
		buf, n, DT_CENTER,
		NULL, &textRect);
	contentRect.left = 0;
	contentRect.top = 0;
	contentRect.right = textRect.right - textRect.left;
	contentRect.bottom = textRect.bottom - textRect.top;
	delete[] buf;
printf("%d %d\n", contentRect.right, contentRect.bottom);

	if (button == leftButtons[0] || button == leftButtons[1] || button == leftButtons[2]) {
		SIZE arrowSize;

		// TODO this should be in DIPs
		contentRect.right += 1;
		// TS_MIN returns 1x1 and TS_DRAW returns 0x0, so...
		GetThemePartSize(theme, dc,
			6, 0,
			NULL, TS_TRUE, &arrowSize);
		contentRect.right += arrowSize.cx;
		if (contentRect.bottom < arrowSize.cy)
			contentRect.bottom = arrowSize.cy;
	}

	// TODO these should be DIPs
	contentRect.right += 13 * 2;
	contentRect.bottom += 5 * 2;

	// dui70.dll seems to do this, including the TS_TRUE part...
	GetThemePartSize(theme, dc,
		3, 1,
		NULL, TS_TRUE, &minSize);
	if (contentRect.right < minSize.cx)
		contentRect.right = minSize.cx;
	if (contentRect.bottom < minSize.cy)
		contentRect.bottom = minSize.cy;

	ReleaseDC(button, dc);
	ret.cx = contentRect.right - contentRect.left;
	ret.cy = contentRect.bottom - contentRect.top;
printf("FINAL %d %d\n", ret.cx, ret.cy);
	return ret;
}

void repositionButtons(HWND hwnd)
{
	HDWP dwp;
	int buttonx, buttony;
	SIZE size;
	int i;

	dwp = BeginDeferWindowPos(5);
	if (dwp == NULL)
		diele("BeginDeferWindowPos()");
	buttonx = 5;
	buttony = 5;
	for (i = 0; i < 5; i++) {
		size = buttonSize(leftButtons[i]);
		dwp = DeferWindowPos(dwp, leftButtons[i], NULL,
			buttonx, buttony, size.cx, size.cy,
			SWP_NOACTIVATE | SWP_NOOWNERZORDER | SWP_NOZORDER);
		if (dwp == NULL)
			diele("DeferWindowPos()");
		buttonx += size.cx;
	}
	if (EndDeferWindowPos(dwp) == 0)
		diele("EndDeferWindowPos()");
}

#if 0
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
#endif

LRESULT CALLBACK wndproc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	NMHDR *nm = (NMHDR *) lParam;
	int i;

	switch (uMsg) {
	case WM_CREATE:
		onWM_CREATE(hwnd);
		updateTheme(hwnd);
		repositionButtons(hwnd);
		break;
	case WM_CLOSE:
		PostQuitMessage(0);
		break;
	case WM_SIZE:
		repositionButtons(hwnd);
		break;
	case WM_THEMECHANGED:
		updateTheme(hwnd);
		repositionButtons(hwnd);
		break;
#if 0
	case WM_COMMAND:
		handleEvents(hwnd, wParam);
		break;
#endif
	case WM_NOTIFY:
		switch (nm->code) {
		case NM_CUSTOMDRAW:
			for (i = 0; i < 5; i++)
				if (nm->hwndFrom == leftButtons[i])
					return drawExplorerButton((NMCUSTOMDRAW *) nm);
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
		if (IsDialogMessageW(mainwin, &msg) == 0) {
			TranslateMessage(&msg);
			DispatchMessageW(&msg);
		}
	}
	return 0;
}
