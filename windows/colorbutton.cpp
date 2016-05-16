// 16 may 2016
#include "uipriv_windows.hpp"

struct uiColorButton {
	uiWindowsControl c;
	HWND hwnd;
	double r;
	double g;
	double b;
	double a;
	void (*onChanged)(uiColorButton *, void *);
	void *onChangedData;
};

static void uiColorButtonDestroy(uiControl *c)
{
	uiColorButton *b = uiColorButton(c);

	uiWindowsUnregisterWM_COMMANDHandler(b->hwnd);
	uiWindowsUnregisterWM_NOTIFYHandler(b->hwnd);
	uiWindowsEnsureDestroyWindow(b->hwnd);
	uiFreeControl(uiControl(b));
}

static INT_PTR TODO(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam){return uMsg == WM_INITDIALOG;}

static BOOL onWM_COMMAND(uiControl *c, HWND hwnd, WORD code, LRESULT *lResult)
{
	uiColorButton *b = uiColorButton(c);
	HWND parent;

	if (code != BN_CLICKED)
		return FALSE;

/* TODO
	parent = GetAncestor(b->hwnd, GA_ROOT);		// TODO didn't we have a function for this
	if (showColorDialog(parent, &(b->params))) {
		updateColorButtonLabel(b);
		(*(b->onChanged))(b, b->onChangedData);
	}
*/

	DialogBox(hInstance, MAKEINTRESOURCE(rcColorDialog), GetAncestor(b->hwnd, GA_ROOT), TODO);

	*lResult = 0;
	return TRUE;
}

static BOOL onWM_NOTIFY(uiControl *c, HWND hwnd, NMHDR *nmhdr, LRESULT *lResult)
{
	uiColorButton *b = uiColorButton(c);
	NMCUSTOMDRAW *nm = (NMCUSTOMDRAW *) nmhdr;
	RECT r;
	uiWindowsSizing sizing;
	int x, y;

	if (nmhdr->code != NM_CUSTOMDRAW)
		return FALSE;
	// and allow the button to draw its background
	if (nm->dwDrawStage != CDDS_PREPAINT)
		return FALSE;

	// TODO use Direct2D? either way, draw alpha
	uiWindowsEnsureGetClientRect(b->hwnd, &r);
	uiWindowsGetSizing(b->hwnd, &sizing);
	x = 3;		// should be enough
	y = 3;
	uiWindowsSizingDlgUnitsToPixels(&sizing, &x, &y);
	r.left += x;
	r.top += y;
	r.right -= x;
	r.bottom -= y;
	// TODO error check
#define comp(x) ((BYTE) (x * 255))
	// TODO free brush
	FillRect(nm->hdc, &r, CreateSolidBrush(RGB(comp(b->r), comp(b->g), comp(b->b))));
#undef comp

	// skip default processing (don't draw text)
	*lResult = CDRF_SKIPDEFAULT;
	return TRUE;
}

uiWindowsControlAllDefaultsExceptDestroy(uiColorButton)

// from http://msdn.microsoft.com/en-us/library/windows/desktop/dn742486.aspx#sizingandspacing
#define buttonHeight 14

// TODO check widths
static void uiColorButtonMinimumSize(uiWindowsControl *c, intmax_t *width, intmax_t *height)
{
	uiColorButton *b = uiColorButton(c);
	SIZE size;
	uiWindowsSizing sizing;
	int y;

	// try the comctl32 version 6 way
	size.cx = 0;		// explicitly ask for ideal size
	size.cy = 0;
	if (SendMessageW(b->hwnd, BCM_GETIDEALSIZE, 0, (LPARAM) (&size)) != FALSE) {
		*width = size.cx;
		*height = size.cy;
		return;
	}

	// that didn't work; fall back to using Microsoft's metrics
	// Microsoft says to use a fixed width for all buttons; this isn't good enough
	// use the text width instead, with some edge padding
	*width = uiWindowsWindowTextWidth(b->hwnd) + (2 * GetSystemMetrics(SM_CXEDGE));
	y = buttonHeight;
	uiWindowsGetSizing(b->hwnd, &sizing);
	uiWindowsSizingDlgUnitsToPixels(&sizing, NULL, &y);
	*height = y;
}

static void defaultOnChanged(uiColorButton *b, void *data)
{
	// do nothing
}

void uiColorButtonColor(uiColorButton *b, double *r, double *g, double *bl, double *a)
{
	*r = b->r;
	*g = b->g;
	*bl = b->b;
	*a = b->a;
}

void uiColorButtonSetColor(uiColorButton *b, double r, double g, double bl, double a)
{
	b->r = r;
	b->g = g;
	b->b = bl;
	b->a = a;
	// TODO don't we have a helper function for this?
	InvalidateRect(b->hwnd, NULL, TRUE);
}

void uiColorButtonOnChanged(uiColorButton *b, void (*f)(uiColorButton *, void *), void *data)
{
	b->onChanged = f;
	b->onChangedData = data;
}

uiColorButton *uiNewColorButton(void)
{
	uiColorButton *b;

	uiWindowsNewControl(uiColorButton, b);

	// initial color is black
	b->r = 0.0;
	b->g = 0.0;
	b->b = 0.0;
	b->a = 1.0;

	b->hwnd = uiWindowsEnsureCreateControlHWND(0,
		L"button", L" ",		// TODO; can't use "" TODO
		BS_PUSHBUTTON | WS_TABSTOP,
		hInstance, NULL,
		TRUE);

	uiWindowsRegisterWM_COMMANDHandler(b->hwnd, onWM_COMMAND, uiControl(b));
	uiWindowsRegisterWM_NOTIFYHandler(b->hwnd, onWM_NOTIFY, uiControl(b));
	uiColorButtonOnChanged(b, defaultOnChanged, NULL);

	return b;
}
