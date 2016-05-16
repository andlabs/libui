// 16 may 2016
#include "uipriv_windows.hpp"

struct colorDialog {
	HWND hwnd;

	HWND svChooser;
	HWND hSlider;
	HWND opacitySlider;
	HWND editH;
	HWND editS;
	HWND editV;
	HWND editRDouble, editRInt;
	HWND editGDouble, editGInt;
	HWND editBDouble, editBInt;
	HWND editADouble, editAInt;
	HWND editHex;

	double r;
	double g;
	double b;
	double a;
	struct colorDialogRGBA *out;
};

static void endColorDialog(struct colorDialog *c, INT_PTR code)
{
	if (EndDialog(c->hwnd, code) == 0)
		logLastError(L"error ending color dialog");
	uiFree(c);
}

static BOOL tryFinishDialog(struct colorDialog *c, WPARAM wParam)
{
	// cancelling
	if (LOWORD(wParam) != IDOK) {
		endColorDialog(c, 1);
		return TRUE;
	}

	// OK
	c->out->r = c->r;
	c->out->g = c->g;
	c->out->b = c->b;
	c->out->a = c->a;
	endColorDialog(c, 2);
	return TRUE;
}

// a few issues:
// - some controls are positioned wrong; see http://stackoverflow.com/questions/37263267/why-are-some-of-my-controls-positioned-slightly-off-in-a-dialog-template-in-a-re
// - labels are too low; need to adjust them by the font's internal leading
// fixupControlPositions() and the following helper routines fix that for us

static LONG offsetTo(HWND a, HWND b)
{
	RECT ra, rb;

	uiWindowsEnsureGetWindowRect(a, &ra);
	uiWindowsEnsureGetWindowRect(b, &rb);
	return rb.top - ra.bottom;
}

static void moveWindowsUp(struct colorDialog *c, LONG by, ...)
{
	va_list ap;
	HWND cur;
	RECT r;

	va_start(ap, by);
	for (;;) {
		cur = va_arg(ap, HWND);
		if (cur == NULL)
			break;
		uiWindowsEnsureGetWindowRect(cur, &r);
		mapWindowRect(NULL, c->hwnd, &r);
		r.top -= by;
		r.bottom -= by;
		// TODO this isn't technically during a resize
		uiWindowsEnsureMoveWindowDuringResize(cur,
			r.left, r.top,
			r.right - r.left, r.bottom - r.top);
	}
	va_end(ap);
}

static void fixupControlPositions(struct colorDialog *c)
{
	HWND labelH;
	HWND labelS;
	HWND labelV;
	HWND labelR;
	HWND labelG;
	HWND labelB;
	HWND labelA;
	HWND labelHex;
	LONG offset;
	uiWindowsSizing sizing;

	labelH = getDlgItem(c->hwnd, rcHLabel);
	labelS = getDlgItem(c->hwnd, rcSLabel);
	labelV = getDlgItem(c->hwnd, rcVLabel);
	labelR = getDlgItem(c->hwnd, rcRLabel);
	labelG = getDlgItem(c->hwnd, rcGLabel);
	labelB = getDlgItem(c->hwnd, rcBLabel);
	labelA = getDlgItem(c->hwnd, rcALabel);
	labelHex = getDlgItem(c->hwnd, rcHexLabel);

	offset = offsetTo(c->editH, c->editS);
	moveWindowsUp(c, offset,
		labelS, c->editS,
		labelG, c->editGDouble, c->editGInt,
		NULL);
	offset = offsetTo(c->editS, c->editV);
	moveWindowsUp(c, offset,
		labelV, c->editV,
		labelB, c->editBDouble, c->editBInt,
		NULL);
	offset = offsetTo(c->editBDouble, c->editADouble);
	moveWindowsUp(c, offset,
		labelA, c->editADouble, c->editAInt,
		NULL);

	// TODO this uses the message font, not the dialog font
	uiWindowsGetSizing(c->hwnd, &sizing);
	offset = sizing.InternalLeading;
	moveWindowsUp(c, offset,
		labelH, labelS, labelV,
		labelR, labelG, labelB, labelA,
		labelHex,
		NULL);
}

static struct colorDialog *beginColorDialog(HWND hwnd, LPARAM lParam)
{
	struct colorDialog *c;

	c = uiNew(struct colorDialog);
	c->hwnd = hwnd;
	c->out = (struct colorDialogRGBA *) lParam;
	c->r = c->out->r;		// load initial values now
	c->g = c->out->g;
	c->b = c->out->b;
	c->a = c->out->a;

	// TODO set up d2dscratches

	c->editH = getDlgItem(c->hwnd, rcH);
	c->editS = getDlgItem(c->hwnd, rcS);
	c->editV = getDlgItem(c->hwnd, rcV);
	c->editRDouble = getDlgItem(c->hwnd, rcRDouble);
	c->editRInt = getDlgItem(c->hwnd, rcRInt);
	c->editGDouble = getDlgItem(c->hwnd, rcGDouble);
	c->editGInt = getDlgItem(c->hwnd, rcGInt);
	c->editBDouble = getDlgItem(c->hwnd, rcBDouble);
	c->editBInt = getDlgItem(c->hwnd, rcBInt);
	c->editADouble = getDlgItem(c->hwnd, rcADouble);
	c->editAInt = getDlgItem(c->hwnd, rcAInt);
	c->editHex = getDlgItem(c->hwnd, rcHex);

	fixupControlPositions(c);

	return c;
}

static INT_PTR CALLBACK colorDialogDlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	struct colorDialog *c;

	c = (struct colorDialog *) GetWindowLongPtrW(hwnd, DWLP_USER);
	if (c == NULL) {
		if (uMsg == WM_INITDIALOG) {
			c = beginColorDialog(hwnd, lParam);
			SetWindowLongPtrW(hwnd, DWLP_USER, (LONG_PTR) c);
			return TRUE;
		}
		return FALSE;
	}

	switch (uMsg) {
	case WM_COMMAND:
		SetWindowLongPtrW(c->hwnd, DWLP_MSGRESULT, 0);		// just in case
		switch (LOWORD(wParam)) {
		case IDOK:
		case IDCANCEL:
			if (HIWORD(wParam) != BN_CLICKED)
				return FALSE;
			return tryFinishDialog(c, wParam);
		}
		return FALSE;
	}
	return FALSE;
}

BOOL showColorDialog(HWND parent, struct colorDialogRGBA *c)
{
	switch (DialogBoxParamW(hInstance, MAKEINTRESOURCE(rcColorDialog), parent, colorDialogDlgProc, (LPARAM) c)) {
	case 1:			// cancel
		return FALSE;
	case 2:			// ok
		// make the compiler happy by putting the return after the switch
		break;
	default:
		logLastError(L"error running color dialog");
	}
	return TRUE;
}
