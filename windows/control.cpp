// 16 august 2015
#include "uipriv_windows.hpp"

void uiWindowsControlSyncEnableState(uiWindowsControl *c, int enabled)
{
	(*(c->SyncEnableState))(c, enabled);
}

void uiWindowsControlSetParentHWND(uiWindowsControl *c, HWND parent)
{
	(*(c->SetParentHWND))(c, parent);
}

void uiWindowsControlMinimumSize(uiWindowsControl *c, intmax_t *width, intmax_t *height)
{
	(*(c->MinimumSize))(c, widdth, height);
}

void uiWindowsControlMinimumSizeChanged(uiWIndowsControl *c)
{
	(*(c->MinimumSizeChanged))(c);
}

void uiWindowsControlLayoutRect(uiWindowsControl *c, RECT *r)
{
	(*(c->LayoutRect))(c, r);
}

void uiWindowsControlAssignControlIDZOrder(uiWindowsControl *c, LONG_PTR *controlID, HWND *insertAfter)
{
	(*(c->AssignControlIDZorder))(c, controlID, insertAfter);
}

HWND uiWindowsEnsureCreateControlHWND(DWORD dwExStyle, LPCWSTR lpClassName, LPCWSTR lpWindowName, DWORD dwStyle, HINSTANCE hInstance, LPVOID lpParam, BOOL useStandardControlFont)
{
	HWND hwnd;

	// don't let using the arrow keys in a uiRadioButtons leave the radio buttons
	if ((dwStyle & WS_TABSTOP) != 0)
		dwStyle |= WS_GROUP;
	hwnd = CreateWindowExW(dwExStyle,
		lpClassName, lpWindowName,
		dwStyle | WS_CHILD | WS_VISIBLE,
		0, 0,
		// use a nonzero initial size just in case some control breaks with a zero initial size
		100, 100,
		utilWindow, NULL, hInstance, lpParam);
	if (hwnd == NULL) {
		logLastError(L"error creating window");
		// TODO return a decoy window
	}
	if (useStandardControlFont)
		SendMessageW(hwnd, WM_SETFONT, (WPARAM) hMessageFont, (LPARAM) TRUE);
	return hwnd;
}

// choose a value distinct from uiWindowSignature
#define uiWindowsControlSignature 0x4D53576E

uiWindowsControl *uiWindowsNewControl(size_t n, uint32_t typesig, const char *typenamestr)
{
	return uiWindowsControl(uiAllocControl(n, uiWindowsControlSignature, typesig, typenamestr));
}

void uiWindowsControlNotifyMinimumSizeChanged(uiWindowsControl *c)
{
	uiControl *parent;

	parent = uiControlParent(uiControl(c));
	if (parent != NULL)
		uiWindowsControlChildMinimumSizeChanged(uiWindowsControl(parent));
}

void uiWindowsControlAssignSoleControlIDZOrder(uiWindowsControl *c)
{
	LONG_PTR controlID;
	HWND insertAfter;

	controlID = 100;
	insertAfter = NULL;
	uiWindowsControlAssignControlIDZorder(c, &controlID, &insertAfter);
}
