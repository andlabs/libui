// 6 april 2015
#include "uipriv_windows.hpp"

static HHOOK filter;

static LRESULT CALLBACK filterProc(int code, WPARAM wParam, LPARAM lParam)
{
	MSG *msg = (MSG *) lParam;

	if (code < 0)
		goto callNext;

	if (areaFilter(msg))		// don't continue to our IsDialogMessage() hack if the area handled it
		goto discard;

	// TODO IsDialogMessage() hack here

	// otherwise keep going
	goto callNext;

discard:
	// we handled it; discard the message so the dialog manager doesn't see it
	return 1;

callNext:
	return CallNextHookEx(filter, code, wParam, lParam);
}

int registerMessageFilter(void)
{
	filter = SetWindowsHookExW(WH_MSGFILTER,
		filterProc,
		hInstance,
		GetCurrentThreadId());
	return filter != NULL;
}

void unregisterMessageFilter(void)
{
	if (UnhookWindowsHookEx(filter) == 0)
		logLastError(L"error unregistering libui message filter");
}

// TODO http://blogs.msdn.com/b/oldnewthing/archive/2005/04/08/406509.aspx when adding accelerators, TranslateAccelerators() before IsDialogMessage()

void uiMain(void)
{
	MSG msg;
	int res;
	HWND active;

	for (;;) {
		res = GetMessageW(&msg, NULL, 0, 0);
		if (res < 0) {
			logLastError(L"error calling GetMessage()");
			break;		// bail out on error
		}
		if (res == 0)		// WM_QUIT
			break;
		// TODO really active? or parentToplevel(msg->hwnd)?
		active = GetActiveWindow();
		if (active != NULL)
			// TODO find documentation that says IsDialogMessage() calls CallMsgFilter() for us, because that's what's happening
			if (IsDialogMessage(active, &msg) != 0)
				continue;
		TranslateMessage(&msg);
		DispatchMessageW(&msg);
	}
}

void uiQuit(void)
{
	PostQuitMessage(0);
}

void uiQueueMain(void (*f)(void *data), void *data)
{
	if (PostMessageW(utilWindow, msgQueued, (WPARAM) f, (LPARAM) data) == 0)
		// TODO this is likely not safe to call across threads (allocates memory)
		logLastError(L"error queueing function to run on main thread");
}
