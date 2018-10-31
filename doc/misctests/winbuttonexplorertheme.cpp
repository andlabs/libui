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

// TODO if we merge this into libui proper, this will need to be deduplicated
static inline HRESULT lastErrorToHRESULT(DWORD lastError)
{
	if (lastError == 0)
		return E_FAIL;
	return HRESULT_FROM_WIN32(lastError);
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

class commandModuleStyleParams {
public:
	virtual int partID_CMOD_MODULEBACKGROUND(void) const = 0;
	virtual int partID_CMOD_TASKBUTTON(void) const = 0;
	virtual int partID_CMOD_SPLITBUTTONLEFT(void) const = 0;
	virtual int partID_CMOD_SPLITBUTTONRIGHT(void) const = 0;
	virtual int partID_CMOD_MENUGLYPH(void) const = 0;
	virtual int partID_CMOD_OVERFLOWGLYPH(void) const = 0;

	virtual int stateID_CMODS_NORMAL(void) const = 0;
	virtual int stateID_CMODS_HOT(void) const = 0;
	virtual int stateID_CMODS_PRESSED(void) const = 0;
	virtual int stateID_CMODS_KEYFOCUSED(void) const = 0;
	virtual int stateID_CMODS_NEARHOT(void) const = 0;

	virtual HRESULT backgroundGradientColors(HTHEME theme, COLORREF *colors) const = 0;

	virtual int buttonMarginsXDIP(void) const = 0;
	virtual int buttonMarginsYDIP(void) const = 0;
	virtual int buttonTextArrowSeparationXDIP(void) const = 0;
};

class commandModuleStyleParamsVista : public commandModuleStyleParams {
public:
	virtual int partID_CMOD_MODULEBACKGROUND(void) const { return 1; }
	virtual int partID_CMOD_TASKBUTTON(void) const { return 2; }
	virtual int partID_CMOD_SPLITBUTTONLEFT(void) const { return 3; }
	virtual int partID_CMOD_SPLITBUTTONRIGHT(void) const { return 4; }
	virtual int partID_CMOD_MENUGLYPH(void) const { return 5; }
	virtual int partID_CMOD_OVERFLOWGLYPH(void) const { return 6; }

	virtual int stateID_CMODS_NORMAL(void) const { return 1; }
	virtual int stateID_CMODS_HOT(void) const { return 2; }
	virtual int stateID_CMODS_PRESSED(void) const { return 3; }
	virtual int stateID_CMODS_KEYFOCUSED(void) const { return 4; }
	virtual int stateID_CMODS_NEARHOT(void) const { return 5; }

	virtual HRESULT backgroundGradientColors(HTHEME theme, COLORREF *colors) const
	{
		if (colors == NULL)
			return E_POINTER;
#define argb(a, r, g, b) ((((COLORREF) ((BYTE) (a))) << 24) | RGB(r, g, b))
		colors[0] = argb(255, 4, 80, 130);
		colors[1] = argb(255, 17, 101, 132);
		colors[2] = argb(255, 29, 121, 134);
#undef argb
		return S_OK;
	}

	virtual int buttonMarginsXDIP(void) const { return 6; }
	virtual int buttonMarginsYDIP(void) const { return 5; }
	virtual int buttonTextArrowSeparationXDIP(void) const { return 3; }
};

class commandModuleStyleParams7 : public commandModuleStyleParams {
	virtual int partID_CMOD_MODULEBACKGROUND(void) const { return 1; }
	virtual int partID_CMOD_TASKBUTTON(void) const { return 3; }
	virtual int partID_CMOD_SPLITBUTTONLEFT(void) const { return 4; }
	virtual int partID_CMOD_SPLITBUTTONRIGHT(void) const { return 5; }
	virtual int partID_CMOD_MENUGLYPH(void) const { return 6; }
	virtual int partID_CMOD_OVERFLOWGLYPH(void) const { return 7; }

	virtual int stateID_CMODS_NORMAL(void) const { return 1; }
	virtual int stateID_CMODS_HOT(void) const { return 2; }
	virtual int stateID_CMODS_PRESSED(void) const { return 3; }
	virtual int stateID_CMODS_KEYFOCUSED(void) const { return 4; }
	/*TODO verify this*/virtual int stateID_CMODS_NEARHOT(void) const { return 5; }

	virtual HRESULT backgroundGradientColors(HTHEME theme, COLORREF *colors) const
	{
		HRESULT hr;

		if (colors == NULL)
			return E_POINTER;
		hr = GetThemeColor(theme,
			2, 0,
			TMT_GRADIENTCOLOR1. color + 0);
		if (hr != S_OK)
			return hr;
		hr = GetThemeColor(theme,
			2, 0,
			TMT_GRADIENTCOLOR2, color + 1);
		if (hr != S_OK)
			return hr;
		return GetThemeColor(theme,
			2, 0,
			TMT_GRADIENTCOLOR3, color + 2);
	}

	virtual int buttonMarginsXDIP(void) const { return 13; }
	virtual int buttonMarginsYDIP(void) const { return 5; }
	virtual int buttonTextArrowSeparationXDIP(void) const { return 1; }
};

// all coordinates are in client space
struct buttonMetrics {
	SIZE fittingSize;
	int baseX;
	int baseY;
	int dpiX;
	int dpiY;
	BOOL hasText;
	SIZE textSize;
	BOOL hasArrow;
	SIZE arrowSize;
};

class commandModuleStyle {
public:
	virtual HRESULT drawFolderBar(commandModuleStyleParams *p, HDC dc, RECT *rcWindow, RECT *rcPaint) const = 0;
	virtual HRESULT buttonMetrics(commandModuleStyleParams *p, HWND button, HDC dc, struct buttonMetrics *m) const = 0;
};

class commandModuleStyleThemed : public commandModuleStyle {
	HTHEME theme;
	HTHEME textstyleTheme;
public:
	commandModuleStyleThemed(HTHEME theme, HTHEME textstyleTheme)
	{
		this->theme = theme;
		this->textstyleTheme = textstyleTheme;
	}

	virtual HRESULT drawFolderBar(commandModuleStyleParams *p, HDC dc, RECT *rcWindow, RECT *rcPaint) const
	{
		COLORREF colors[3];
		TRIVERTEX vertices[4];
		static GRADIENT_RECT gr[2] = {
			{ 0, 1 },
			{ 2, 3 },
		};
		HRESULT hr;

		hr = p->backgroundGradientColors(this->theme, colors);
		if (hr != S_OK)
			return hr;

		vertices[0].x = rcWindow->left;
		vertices[0].y = rcWindow->top;
		vertices[0].Red = ((COLOR16) GetRValue(colors[0])) << 8;
		vertices[0].Green = ((COLOR16) GetGValue(colors[0])) << 8;
		vertices[0].Blue = ((COLOR16) GetBValue(colors[0])) << 8;
		vertices[0].Alpha = ((COLOR16) LOBYTE(colors[0] >> 24)) << 8;

		vertices[1].x = (rcWindow->right - rcWindow->left) / 2;
		vertices[1].y = rcWindow->bottom;
		vertices[1].Red = ((COLOR16) GetRValue(colors[1])) << 8;
		vertices[1].Green = ((COLOR16) GetGValue(colors[1])) << 8;
		vertices[1].Blue = ((COLOR16) GetBValue(colors[1])) << 8;
		vertices[1].Alpha = ((COLOR16) LOBYTE(colors[1] >> 24)) << 8;

		vertices[2] = vertices[1];
		vertices[2].y = rcWindow->top;

		vertices[3].x = rcWindow->right;
		vertices[3].y = rcWindow->bottom;
		vertices[3].Red = ((COLOR16) GetRValue(colors[2])) << 8;
		vertices[3].Green = ((COLOR16) GetGValue(colors[2])) << 8;
		vertices[3].Blue = ((COLOR16) GetBValue(colors[2])) << 8;
		vertices[3].Alpha = ((COLOR16) LOBYTE(colors[2] >> 24)) << 8;

		if (GradientFill(dc, vertices, 4, (PVOID) gr, 2, GRADIENT_FILL_RECT_H) == FALSE)
			return lastErrorToHRESULT(GetLastError());
		return DrawThemeBackground(this->theme, dc,
			p->partID_CMOD_MODULEBACKGROUND(), 0,
			rcWindow, rcPaint);
	}

#define hasNonsplitArrow(button) ((button) == leftButtons[0] || (button) == leftButtons[1] || (button) == leftButtons[2])

#define dlgUnitsToX(dlg, baseX) MulDiv((dlg), (baseX), 4)
#define dlgUnitsToY(dlg, baseY) MulDiv((dlg), (baseY), 8)
// TODO verify the parameter order
#define dipsToX(dip, dpiX) MulDiv((dip), (dpiX), 96)
#define dipsToY(dip, dpiY) MulDiv((dip), (dpiY), 96)

	// TODO for win7: the sizes are correct (according to UI Automation) but they don't visually match?
	virtual HRESULT buttonMetrics(commandModuleStyleParams *p, HWND button, HDC dc, struct buttonMetrics *m) const
	{
		BOOL releaseDC;
		TEXTMETRICW tm;
		RECT r;
		int minStdButtonHeight;
		HRESULT hr;

		releaseDC = FALSE;
		if (dc == NULL) {
			dc = GetDC(button);
			if (dc == NULL)
				return lastErrorToHRESULT(GetLastError());
			releaseDC = TRUE;
		}

		ZeroMemory(&tm, sizeof (TEXTMETRICW));
		hr = GetThemeTextMetrics(this->textstyleTheme, dc,
			TEXT_BODYTEXT, 0,
			&tm);
		if (hr != S_OK)
			goto fail;
		hr = GetThemeTextExtent(this->textstyleTheme, dc,
			TEXT_BODYTEXT, 0,
			L"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz", 52, 0,
			NULL, &r);
		if (hr != S_OK)
			goto fail;
		m->baseX = (int) (((r.right - r.left) / 26 + 1) / 2);
		m->baseY = (int) tm.tmHeight;
		m->dpiX = GetDeviceCaps(dc, LOGPIXELSX);
		m->dpiY = GetDeviceCaps(dc, LOGPIXELSY);

		m->fittingSize.cx = 0;
		m->fittingSize.cy = 0;

		m->hasText = TRUE;
		if (m->hasText) {
			LRESULT n;
			WCHAR *buf;
			LOGFONTW lf;

			n = SendMessageW(button, WM_GETTEXTLENGTH, 0, 0);
			buf = new WCHAR[n + 1];
			GetWindowTextW(button, buf, n + 1);
			hr = GetThemeTextExtent(this->textstyleTheme, dc,
				TEXT_BODYTEXT, 0,
				buf, n, DT_CENTER,
				NULL, &r);
			delete[] buf;
			if (hr != S_OK)
				goto fail;
			m->textSize.cx = r.right - r.left;
			m->textSize.cy = r.bottom - r.top;
			m->fittingSize.cx += m->textSize.cx;
			m->fittingSize.cy += m->textSize.cy;

			// dui70.dll adds this to the width when "overhang" is enabled, and it seems to be enabled for our cases, but I can't tell what conditions it's enabled for...
			// and yes, it seems to be using the raw lfHeight value here :/
			// TODO find the right TMT constant
			hr = GetThemeFont(this->textstyleTheme, dc,
				4, 0,
				TMT_FONT, &lf);
			if (hr != S_OK)
				goto fail;
			m->fittingSize.cx += 2 * (abs(lf.lfHeight) / 6);
		}

		m->hasArrow = hasNonsplitArrow(button);
		if (m->hasArrow) {
			// TS_MIN returns 1x1 and TS_DRAW returns 0x0, so...
			hr = GetThemePartSize(theme, dc,
				p->partID_CMOD_MENUGLYPH(), 0,
				NULL, TS_TRUE, &(m->arrowSize));
			if (hr != S_OK)
				goto fail;
			m->fittingSize.cx += m->arrowSize.cx;
			// TODO I don't think dui70.dll takes this into consideration...
			if (m->fittingSize.cy < m->arrowSize.cy)
				m->fittingSize.cy = m->arrowSize.cy;
			m->fittingSize.cx += dipsToX(p->buttonTextArrowSeparationXDIP(), m->dpiX);
		}

		m->fittingSize.cx += dipsToX(p->buttonMarginsXDIP(), m->dpiX) * 2;
		m->fittingSize.cy += dipsToY(p->buttonMarginsYDIP(), m->dpiY) * 2;

		// and dui70.dll seems to do a variant of this but for text buttons only...
		minStdButtonHeight = dlgUnitsToY(14, m->baseY);
		if (m->fittingSize.cy < minStdButtonHeight)
			m->fittingSize.cy = minStdButtonHeight;

		hr = S_OK;
	fail:
		if (releaseDC)
			// TODO when migrating this to libui, this will need to be renamed to indicate we are intentionally ignoring errors
			ReleaseDC(button, dc);
		return hr;
	};
};

struct buttonRects {
	RECT clientRect;
	RECT textRect;
	RECT arrowRect;
};

// TODO check errors
void buttonRects(HWND button, struct buttonMetrics *m, struct buttonRects *r)
{
	GetClientRect(button, &(r->clientRect));

	if (m->hasText)
		r->textRect = r->clientRect;

	if (m->hasArrow) {
		r->arrowRect = r->clientRect;
		r->arrowRect.left = r->arrowRect.right;
		r->arrowRect.left -= dipsToX(13, m->dpiX);
		r->arrowRect.right = r->arrowRect.left;
		r->arrowRect.left -= m->arrowSize.cx;
		r->arrowRect.top += ((r->arrowRect.bottom - r->arrowRect.top) - m->arrowSize.cy) / 2;
		r->arrowRect.bottom = r->arrowRect.top + m->arrowSize.cy;

		if (m->hasText) {
			r->textRect.right = r->arrowRect.left - dipsToX(1, m->dpiX);
			r->textRect.right += dipsToX(13, m->dpiX);
		}
	}
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
	struct buttonMetrics m;
	struct buttonRects r;
	int part, state;

	if (nm->dwDrawStage != CDDS_PREPAINT)
		return CDRF_DODEFAULT;
	button = nm->hdr.hwndFrom;
	buttonMetrics(button, nm->hdc, &m);
	buttonRects(button, &m, &r);

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
		&(r.clientRect), &(nm->rc));

	if (m.hasText) {
		int textState;
		COLORREF textColor;
		LRESULT n;
		WCHAR *buf;
		DTTOPTS dttopts;

		// TODO these values are only for part==3
		textState = 1;
		if ((nm->uItemState & CDIS_DISABLED) != 0)
			textState = 6;
		// TODO name the constant for the property ID
		GetThemeColor(theme,
			3, textState,
			3803, &textColor);
		n = SendMessageW(button, WM_GETTEXTLENGTH, 0, 0);
		buf = new WCHAR[n + 1];
		GetWindowTextW(button, buf, n + 1);
		ZeroMemory(&dttopts, sizeof (DTTOPTS));
		dttopts.dwSize = sizeof (DTTOPTS);
		dttopts.dwFlags = DTT_TEXTCOLOR;
		dttopts.crText = textColor;
		DrawThemeTextEx(textstyleTheme, nm->hdc,
			4, 0,
			buf, n, DT_CENTER | DT_VCENTER | DT_SINGLELINE,
			&(r.textRect), &dttopts);
		delete[] buf;
	}

	if (m.hasArrow)
		DrawThemeBackground(theme, nm->hdc,
			6, 0,
			&(r.arrowRect), &(nm->rc));

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

void repositionButtons(HWND hwnd)
{
	HDWP dwp;
	int buttonx, buttony;
	struct buttonMetrics m;
	int i;

	dwp = BeginDeferWindowPos(5);
	if (dwp == NULL)
		diele("BeginDeferWindowPos()");
	buttonx = 5;
	buttony = 5;
	for (i = 0; i < 5; i++) {
		buttonMetrics(leftButtons[i], NULL, &m);
		dwp = DeferWindowPos(dwp, leftButtons[i], NULL,
			buttonx, buttony, m.fittingSize.cx, m.fittingSize.cy,
			SWP_NOACTIVATE | SWP_NOOWNERZORDER | SWP_NOZORDER);
		if (dwp == NULL)
			diele("DeferWindowPos()");
		buttonx += m.fittingSize.cx;
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
	HDC dc;
	PAINTSTRUCT ps;
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
		// TODO check errors
		InvalidateRect(hwnd, NULL, TRUE);
		break;
	case WM_THEMECHANGED:
		updateTheme(hwnd);
		repositionButtons(hwnd);
		break;
	case WM_PAINT:
		// TODO check errors
		dc = BeginPaint(hwnd, &ps);
		{RECT w;
		GetClientRect(hwnd,&w);
		drawExplorerBackground(theme, dc, &w, &(ps.rcPaint));}
		EndPaint(hwnd, &ps);
		return 0;
	case WM_PRINTCLIENT:
		{RECT w;
		GetClientRect(hwnd,&w);
		drawExplorerBackground(theme, (HDC) wParam, &w, &w);}
		return 0;
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
