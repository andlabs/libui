// 6 april 2015
#include "uipriv_windows.h"

// TODO http://blogs.msdn.com/b/oldnewthing/archive/2005/04/08/406509.aspx when adding accelerators, TranslateMessage() before IsDialogMessage()

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
			switch (windowClassOf(focus, areaClass, NULL)) {
			case 0:		// uiArea
				processAreaMessage(active, &msg);
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
