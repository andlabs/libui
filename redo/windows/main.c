// 6 april 2015
#include "uipriv_windows.h"

// #qo LDFLAGS: -luser32 -lkernel32 -lgdi32 -luxtheme -lmsimg32 -lcomdlg32 -lole32 -loleaut32 -loleacc -luuid

static void msgloop_tab(HWND active, HWND focus, MSG *msg)
{
	BOOL hasChildren;
	BOOL idm;

	// THIS BIT IS IMPORTANT: if the current tab has no children, then there will be no children left in the dialog to tab to, and IsDialogMessageW() will loop forever
	hasChildren = SendMessageW(focus, msgHasTabStops, 0, 0);
	if (hasChildren)
		tabEnterTabNavigation(focus);
	idm = IsDialogMessageW(active, msg);
	if (hasChildren)
		tabLeaveTabNavigation(focus);
	if (idm != 0)
		return;
	TranslateMessage(msg);
	DispatchMessageW(msg);
}

static void msgloop_else(MSG *msg)
{
	TranslateMessage(msg);
	DispatchMessageW(msg);
}

void uiMain(void)
{
	MSG msg;
	int res;
	HWND active, focus;

	for (;;) {
		SetLastError(0);
		res = GetMessageW(&msg, NULL, 0, 0);
		if (res < 0)
			logLastError("error calling GetMessage() in uiMain()");
		if (res == 0)		// WM_QUIT
			break;
		active = GetActiveWindow();
		if (active == NULL) {
			msgloop_else(&msg);
			continue;
		}

		// bit of logic involved here:
		// we don't want dialog messages passed into Areas, so we don't call IsDialogMessageW() there
		// as for Tabs, we can't have both WS_TABSTOP and WS_EX_CONTROLPARENT set at the same time, so we hotswap the two styles to get the behavior we want
		focus = GetFocus();
		if (focus != NULL) {
			switch (windowClassOf(focus, L"TODO Area not yet implemented", WC_TABCONTROLW, NULL)) {
			case 0:		// uiArea
//				msgloop_area(active, focus, &msg);
				continue;
			case 1:		// WC_TABCONTROLW
				msgloop_tab(active, focus, &msg);
				continue;
			}
			// else fall through
		}

		if (IsDialogMessage(active, &msg) != 0)
			continue;
		msgloop_else(&msg);
	}
}

void uiQuit(void)
{
	PostQuitMessage(0);
}
