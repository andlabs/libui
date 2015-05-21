// 20 may 2015
#include "uipriv_windows.h"

// In each of these structures, hwnd is the hash key.

struct commandHandler {
	HWND hwnd;
	BOOL (*handler)(uiControl *, WORD, LRESULT *);
	uiControl *c;
	UT_hash_handle hh;
};

struct notifyHandler {
	HWND hwnd;
	BOOL (*handler)(uiControl *, NMHDR *, LRESULT *);
	uiControl *c;
	UT_hash_handle hh;
};

struct hscrollHandler {
	HWND hwnd;
	BOOL (*handler)(uiControl *, WORD, LRESULT *);
	uiControl *c;
	UT_hash_handle hh;
};

struct commandHandler *commandHandlers = NULL;
struct notifyHandler *notifyHandlers = NULL;
struct hscrollHandler *hscrollHandlers = NULL;

void uiWindowsRegisterWM_COMMANDHandler(HWND hwnd, BOOL (*handler)(uiControl *, WORD, LRESULT *), uiControl *c)
{
	struct commandHandler *ch;

	HASH_FIND_PTR(commandHandlers, &hwnd, ch);
	if (ch != NULL)
		complain("window handle %p already subscribed to receive WM_COMMANDs in uiWindowsRegisterWM_COMMANDHandler()", hwnd);
	ch = uiNew(struct commandHandler);
	ch->hwnd = hwnd;
	ch->handler = handler;
	ch->c = c;
	HASH_ADD_PTR(commandHandlers, hwnd, ch);
}

void uiWindowsUnregisterWM_COMMANDHandler(HWND hwnd)
{
	struct commandHandler *ch;

	HASH_FIND_PTR(commandHandlers, &hwnd, ch);
	if (ch == NULL)
		complain("window handle %p not registered with a WM_COMMAND handler in uiWindowsUnregisterWM_COMMANDHandler()", hwnd);
	HASH_DEL(commandHandlers, ch);
	uiFree(ch);
}

BOOL runWM_COMMAND(WPARAM wParam, LPARAM lParam, LRESULT *lResult)
{
	HWND control;
	struct commandHandler *ch;

	// bounce back to the control in question
	// don't bounce back if to the utility window, in which case act as if the message was ignored
	control = (HWND) lParam;
	if (control != NULL && IsChild(utilWindow, control) == 0) {
		HASH_FIND_PTR(commandHandlers, &control, ch);
		if (ch != NULL)
			return (*(ch->handler))(ch->c, HIWORD(wParam), lResult);
		// not registered; fall out to return FALSE
	}
	return FALSE;
}
