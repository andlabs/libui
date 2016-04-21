// 20 may 2015
#include "uipriv_windows.hpp"

// TODO get rid of the macro magic
// TODO re-add existence checks

// In each of these structures, hwnd is the hash key.

struct commandHandler {
	BOOL (*handler)(uiControl *, HWND, WORD, LRESULT *);
	uiControl *c;
};

struct notifyHandler {
	BOOL (*handler)(uiControl *, HWND, NMHDR *, LRESULT *);
	uiControl *c;
};

struct hscrollHandler {
	BOOL (*handler)(uiControl *, HWND, WORD, LRESULT *);
	uiControl *c;
};

static std::map<HWND, struct commandHandler> commandHandlers;
static std::map<HWND, struct notifyHandler> notifyHandlers;
static std::map<HWND, struct hscrollHandler> hscrollHandlers;

#define REGFN(WM_MESSAGE, message, params) \
	void uiWindowsRegister ## WM_MESSAGE ## Handler(HWND hwnd, BOOL (*handler)params, uiControl *c) \
	{ \
		if (message ## Handler.find(hwnd) != ch.end()) \
			complain("window handle %p already subscribed with a %s handler in uiWindowsRegister%sHandler()", hwnd, #WM_MESSAGE, #WM_MESSAGE); \
		message ## Handler[hwnd].handler = handler; \
		message ## Handler[hwnd].c = c; \
	}
REGFN(WM_COMMAND, command, (uiControl *, HWND, WORD, LRESULT *))
REGFN(WM_NOTIFY, notify, (uiControl *, HWND, NMHDR *, LRESULT *))
REGFN(WM_HSCROLL, hscroll, (uiControl *, HWND, WORD, LRESULT *))

#define UNREGFN(WM_MESSAGE, message) \
	void uiWindowsUnregister ## WM_MESSAGE ## Handler(HWND hwnd) \
	{ \
		message ## Handler.erase(hwnd); \
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
			if (message ## Handlers.find(control) != message ## Handlers.end()) \
				return (*(message ## Handlers[control].handler))(message ## Handlers[control].c, control, arg3, lResult); \
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

static std::map<HWND, bool> wininichanges;

void uiWindowsRegisterReceiveWM_WININICHANGE(HWND hwnd)
{
	if (wininichanges[hwnd])
		complain("window handle %p already subscribed to receive WM_WINICHANGEs in uiWindowsRegisterReceiveWM_WININICHANGE()", hwnd);
	wininichanges[hwnd] = true;
}

void uiWindowsUnregisterReceiveWM_WININICHANGE(HWND hwnd)
{
	wininichanges[hwnd] = false;
}

void issueWM_WININICHANGE(WPARAM wParam, LPARAM lParam)
{
	struct wininichange *ch;

	for (const auto &iter : wininichanges)
		SendMessageW(iter.first, WM_WININICHANGE, wParam, lParam);
}
