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
	HWND active;

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

		// TODO find documentation that says IsDialogMessage() calls CallMsgFilter() for us, because that's what's happening
		// TODO rewrite this whole function to compensate

		if (IsDialogMessage(active, &msg) != 0)
			continue;
		msgloop_else(&msg);
	}
}

void uiQuit(void)
{
	PostQuitMessage(0);
}
