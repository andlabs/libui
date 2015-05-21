// 20 may 2015
#include "uipriv_windows.h"

// TODO switch to uthash

struct commandHandler {
	HWND hwnd;
	BOOL (*handler)(uiControl *, WORD, LRESULT *);
	uiControl *c;
};

struct notifyHandler {
	HWND hwnd;
	BOOL (*handler)(uiControl *, NMHDR *, LRESULT *);
	uiControl *c;
};

struct hscrollHandler {
	HWND hwnd;
	BOOL (*handler)(uiControl *, WORD, LRESULT *);
	uiControl *c;
};

struct ptrArray *commandHandlers = NULL;
struct ptrArray *notifyHandlers = NULL;
struct ptrArray *hscrollHandlers = NULL;

void uiWindowsRegisterWM_COMMANDHandler(HWND hwnd, BOOL (*handler)(uiControl *, WORD, LRESULT *), uiControl *c)
{
	struct commandHandler *ch;

	ch = uiNew(struct commandHandler);
	ch->hwnd = hwnd;
	ch->handler = handler;
	ch->c = c;
	if (commandHandlers == NULL)
		commandHandlers = newPtrArray();
	ptrArrayAppend(commandHandlers, ch);
}

void uiWindowsUnregisterWM_COMMANDHandler(HWND hwnd)
{
	struct commandHandler *ch;
	uintmax_t i;

	for (i = 0; i < commandHandlers->len; i++) {
		ch = ptrArrayIndex(commandHandlers, struct commandHandler *, i);
		if (ch->hwnd == hwnd) {
			ptrArrayDelete(commandHandlers, i);
			uiFree(ch);
			if (commandHandlers->len == 0) {
				ptrArrayDestroy(commandHandlers);
				commandHandlers = NULL;
			}
			return;
		}
	}
	complain("window handle %p not registered with a WM_COMMAND handler in uiWindowsUnregisterWM_COMMANDHandler()", hwnd);
}

BOOL runWM_COMMAND(WPARAM wParam, LPARAM lParam, LRESULT *lResult)
{
	HWND control;
	struct commandHandler *ch;
	uintmax_t i;

	// bounce back to the control in question
	// don't bounce back if to the utility window, in which case act as if the message was ignored
	control = (HWND) lParam;
	if (control != NULL && IsChild(utilWindow, control) == 0) {
		for (i = 0; i < commandHandlers->len; i++) {
			ch = ptrArrayIndex(commandHandlers, struct commandHandler *, i);
			if (ch->hwnd == control)
				return (*(ch->handler))(ch->c, HIWORD(wParam), lResult);
		}
		// not registered; fall out to return FALSE
	}
	return FALSE;
}
