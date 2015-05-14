// 26 april 2015
#include "uipriv_windows.h"

// This contains code used by all OS controls that contain other controls. It also contains the container drawing code.

static HBRUSH parentBrush = NULL;

static HWND realParent(HWND hwnd)
{
	HWND parent;
	int class;

	parent = hwnd;
	for (;;) {
		parent = GetAncestor(parent, GA_PARENT);
		// skip groupboxes; they're (supposed to be) transparent
		// skip uiContainers; they don't draw anything
		class = windowClassOf(parent, L"button", containerClass, NULL);
		if (class != 0 && class != 1)
			break;
	}
	return parent;
}

struct parentDraw {
	HDC cdc;
	HBITMAP bitmap;
	HBITMAP prevbitmap;
};

static void parentDraw(HDC dc, HWND parent, struct parentDraw *pd)
{
	RECT r;

	if (GetClientRect(parent, &r) == 0)
		logLastError("error getting parent's client rect in parentDraw()");
	pd->cdc = CreateCompatibleDC(dc);
	if (pd->cdc == NULL)
		logLastError("error creating compatible DC in parentDraw()");
	pd->bitmap = CreateCompatibleBitmap(dc, r.right - r.left, r.bottom - r.top);
	if (pd->bitmap == NULL)
		logLastError("error creating compatible bitmap in parentDraw()");
	pd->prevbitmap = SelectObject(pd->cdc, pd->bitmap);
	if (pd->prevbitmap == NULL)
		logLastError("error selecting bitmap into compatible DC in parentDraw()");
	SendMessageW(parent, WM_PRINTCLIENT, (WPARAM) (pd->cdc), PRF_CLIENT);
}

static void endParentDraw(struct parentDraw *pd)
{
	if (SelectObject(pd->cdc, pd->prevbitmap) != pd->bitmap)
		logLastError("error selecting previous bitmap back into compatible DC in endParentDraw()");
	if (DeleteObject(pd->bitmap) == 0)
		logLastError("error deleting compatible bitmap in endParentDraw()");
	if (DeleteDC(pd->cdc) == 0)
		logLastError("error deleting compatible DC in endParentDraw()");
}

// see http://www.codeproject.com/Articles/5978/Correctly-drawn-themed-dialogs-in-WinXP
static HBRUSH getControlBackgroundBrush(HWND hwnd, HDC dc)
{
	HWND parent;
	RECT hwndScreenRect;
	struct parentDraw pd;
	HBRUSH brush;

	parent = realParent(hwnd);

	parentDraw(dc, parent, &pd);
	brush = CreatePatternBrush(pd.bitmap);
	if (brush == NULL)
		logLastError("error creating pattern brush in getControlBackgroundBrush()");
	endParentDraw(&pd);

	// now figure out where the control is relative to the parent so we can align the brush properly
	if (GetWindowRect(hwnd, &hwndScreenRect) == 0)
		logLastError("error getting control window rect in getControlBackgroundBrush()");
	// this will be in screen coordinates; convert to parent coordinates
	mapWindowRect(NULL, parent, &hwndScreenRect);
	if (SetBrushOrgEx(dc, -hwndScreenRect.left, -hwndScreenRect.top, NULL) == 0)
		logLastError("error setting brush origin in getControlBackgroundBrush()");

	return brush;
}

// TODO
static void paintContainerBackground(HWND hwnd, HDC dc, RECT *paintRect)
{
	HWND parent;
	RECT paintRectParent;
	struct parentDraw pd;

	parent = realParent(hwnd);
	parentDraw(dc, parent, &pd);

	paintRectParent = *paintRect;
	mapWindowRect(hwnd, parent, &paintRectParent);
	if (BitBlt(dc, paintRect->left, paintRect->top, paintRect->right - paintRect->left, paintRect->bottom - paintRect->top,
		pd.cdc, paintRectParent.left, paintRectParent.top,
		SRCCOPY) == 0)
		logLastError("error drawing parent background over uiContainer in paintContainerBackground()");

	endParentDraw(&pd);
}

// why have this to begin with? http://blogs.msdn.com/b/oldnewthing/archive/2010/03/16/9979112.aspx
BOOL handleParentMessages(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT *lResult)
{
	HWND control;
	NMHDR *nm = (NMHDR *) lParam;
	RECT r;

	switch (uMsg) {
	case WM_COMMAND:
		// bounce back to the control in question
		// except if to the initial parent, in which case act as if the message was ignored
		control = (HWND) lParam;
		if (control != NULL && IsChild(initialParent, control) == 0) {
			*lResult = SendMessageW(control, msgCOMMAND, wParam, lParam);
			return TRUE;
		}
		break;
	case WM_NOTIFY:
		// same as WM_COMMAND
		control = nm->hwndFrom;
		if (control != NULL && IsChild(initialParent, control) == 0) {
			*lResult = SendMessageW(control, msgNOTIFY, wParam, lParam);
			return TRUE;
		}
		break;
	case WM_CTLCOLORSTATIC:
	case WM_CTLCOLORBTN:
		if (parentBrush != NULL)
			if (DeleteObject(c->brush) == 0)
				logLastError("error deleting old background brush in containerWndProc()");
		if (SetBkMode((HDC) wParam, TRANSPARENT) == 0)
			logLastError("error setting transparent background mode to controls in containerWndProc()");
		parentBrush = getControlBackgroundBrush((HWND) lParam, (HDC) wParam);
		*lResult = (LRESULT) parentBrush;
		return TRUE;
	}
	return FALSE;
}
