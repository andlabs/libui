// 20 may 2015
#include "uipriv_windows.h"

// In each of these structures, hwnd is the hash key.

struct commandHandler {
	HWND hwnd;
	BOOL (*handler)(uiControl *, HWND, WORD, LRESULT *);
	uiControl *c;
	UT_hash_handle hh;
};

struct notifyHandler {
	HWND hwnd;
	BOOL (*handler)(uiControl *, HWND, NMHDR *, LRESULT *);
	uiControl *c;
	UT_hash_handle hh;
};

struct hscrollHandler {
	HWND hwnd;
	BOOL (*handler)(uiControl *, HWND, WORD, LRESULT *);
	uiControl *c;
	UT_hash_handle hh;
};

static struct commandHandler *commandHandlers = NULL;
static struct notifyHandler *notifyHandlers = NULL;
static struct hscrollHandler *hscrollHandlers = NULL;

#define REGFN(WM_MESSAGE, message, params) \
	void uiWindowsRegister ## WM_MESSAGE ## Handler(HWND hwnd, BOOL (*handler)params, uiControl *c) \
	{ \
		struct message ## Handler *ch; \
		HASH_FIND_PTR(message ## Handlers, &hwnd, ch); \
		if (ch != NULL) \
			complain("window handle %p already subscribed with a %s handler in uiWindowsRegister%sHandler()", hwnd, #WM_MESSAGE, #WM_MESSAGE); \
		ch = uiNew(struct message ## Handler); \
		ch->hwnd = hwnd; \
		ch->handler = handler; \
		ch->c = c; \
		HASH_ADD_PTR(message ## Handlers, hwnd, ch); \
	}
REGFN(WM_COMMAND, command, (uiControl *, HWND, WORD, LRESULT *))
REGFN(WM_NOTIFY, notify, (uiControl *, HWND, NMHDR *, LRESULT *))
REGFN(WM_HSCROLL, hscroll, (uiControl *, HWND, WORD, LRESULT *))

#define UNREGFN(WM_MESSAGE, message) \
	void uiWindowsUnregister ## WM_MESSAGE ## Handler(HWND hwnd) \
	{ \
		struct message ## Handler *ch; \
		HASH_FIND_PTR(message ## Handlers, &hwnd, ch); \
		if (ch == NULL) \
			complain("window handle %p not registered with a %s handler in uiWindowsUnregister%sHandler()", hwnd, #WM_MESSAGE, #WM_MESSAGE); \
		HASH_DEL(message ## Handlers, ch); \
		uiFree(ch); \
	}
UNREGFN(WM_COMMAND, command)
UNREGFN(WM_NOTIFY, notify)
UNREGFN(WM_HSCROLL, hscroll)

#define RUNFN(WM_MESSAGE, message, gethwnd, arg3) \
	BOOL run ## WM_MESSAGE(WPARAM wParam, LPARAM lParam, LRESULT *lResult) \
	{ \
		HWND control; \
		struct message ## Handler *ch;\
		/* bounce back to the control in question */ \
		/* don't bounce back if to the utility window, in which case act as if the message was ignored */ \
		control = gethwnd; \
		if (control != NULL && IsChild(utilWindow, control) == 0) { \
			HASH_FIND_PTR(message ## Handlers, &control, ch); \
			if (ch != NULL) \
				return (*(ch->handler))(ch->c, control, arg3, lResult); \
			/* not registered; fall out to return FALSE */ \
		} \
		return FALSE; \
	}
RUNFN(WM_COMMAND, command,
	(HWND) lParam,
	HIWORD(wParam))
RUNFN(WM_NOTIFY, notify,
	((NMHDR *) lParam)->hwndFrom,
	((NMHDR *) lParam))
RUNFN(WM_HSCROLL, hscroll,
	(HWND) lParam,
	LOWORD(wParam))

struct wininichange {
	HWND hwnd;
	UT_hash_handle hh;
};

static struct wininichange *wininichanges = NULL;

void uiWindowsRegisterReceiveWM_WININICHANGE(HWND hwnd)
{
	struct wininichange *ch;

	HASH_FIND_PTR(wininichanges, &hwnd, ch);
	if (ch != NULL)
		complain("window handle %p already subscribed to receive WM_WINICHANGEs in uiWindowsRegisterReceiveWM_WININICHANGE()", hwnd);
	ch = uiNew(struct wininichange);
	ch->hwnd = hwnd;
	HASH_ADD_PTR(wininichanges, hwnd, ch);
}

void uiWindowsUnregisterReceiveWM_WININICHANGE(HWND hwnd)
{
	struct wininichange *ch;

	HASH_FIND_PTR(wininichanges, &hwnd, ch);
	if (ch == NULL)
		complain("window handle %p not registered to receive WM_WININICHANGEs in uiWindowsUnregisterReceiveWM_WININICHANGE()", hwnd);
	HASH_DEL(wininichanges, ch);
	uiFree(ch);
}

void issueWM_WININICHANGE(WPARAM wParam, LPARAM lParam)
{
	struct wininichange *ch;

	for (ch = wininichanges; ch != NULL; ch = ch->hh.next)
		SendMessageW(ch->hwnd, WM_WININICHANGE, wParam, lParam);
}
