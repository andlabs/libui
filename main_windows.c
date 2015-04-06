// 6 april 2015
#include "ui_windows.h"

// #qo LDFLAGS: -luser32 -lkernel32 -lgdi32 -luxtheme -lmsimg32 -lcomdlg32 -lole32 -loleaut32 -loleacc -luuid

static void uimsgloop_else(MSG *msg)
{
	TranslateMessage(msg);
	DispatchMessage(msg);
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
			uimsgloop_else(&msg);
			continue;
		}

		// bit of logic involved here:
		// we don't want dialog messages passed into Areas, so we don't call IsDialogMessageW() there
		// as for Tabs, we can't have both WS_TABSTOP and WS_EX_CONTROLPARENT set at the same time, so we hotswap the two styles to get the behavior we want
		focus = GetFocus();
		if (focus != NULL) {
/*TODO			switch (windowClassOf(focus, areaWindowClass, WC_TABCONTROLW, NULL)) {
			case 0:		// areaWindowClass
				uimsgloop_area(active, focus, &msg);
				continue;
			case 1:		// WC_TABCONTROLW
				uimsgloop_tab(active, focus, &msg);
				continue;
			}
			// else fall through
*/		}

		if (IsDialogMessage(active, &msg) != 0)
			continue;
		uimsgloop_else(&msg);
	}
}

void uiQuit(void)
{
	PostQuitMessage(0);
}
