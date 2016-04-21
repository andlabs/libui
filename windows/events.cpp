// 20 may 2015
#include "uipriv_windows.hpp"

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

template<typename thirdArg, std::map<HWND, typename> &handlers>
static void registerHandler(HWND hwnd, BOOL (*handler)(uiControl *, HWND, thirdArg, LRESULT *), uiControl *c, const char *mname, const char *fname)
{
	if (handlers.find(hwnd) != handlers.end())
		complain("window handle %p already subscribed with a %s handler in %s", hwnd, mname, fname);
	handlers[hwnd].handler = handler;
	handlers[hwnd].c = c;
}

void uiWindowsRegisterWM_COMMANDHandler(HWND hwnd, BOOL (*handler)(uiControl *, HWND, WORD, LRESULT *), uiControl *c)
{
	registerHandler<WORD, commandHandlers>(hwnd, handler, c, "WM_COMMAND", "uiWindowsRegisterWM_COMMANDHandler()");
}

void uiWindowsRegisterWM_NOTIFYHandler(HWND hwnd, BOOL (*handler)(uiControl *, HWND, NMHDR *, LRESULT *), uiControl *c)
{
	registerHandler<NMHDR *, notifyHandlers>(hwnd, handler, c, "WM_NOTIFY", "uiWindowsRegisterWM_NOTIFYHandler()");
}

void uiWindowsRegisterWM_HSCROLLHandler(HWND hwnd, BOOL (*handler)(uiControl *, HWND, WORD, LRESULT *), uiControl *c)
{
	registerHandler<WORD, hscrollHandlers>(hwnd, handler, c, "WM_HSCROLL", "uiWindowsRegisterWM_HSCROLLHandler");
}

template<std::map<HWND, typename> &handlers>
static void unregisterHandler(HWND hwnd, const char *mname, const char *fname)
{
	if (handlers.find(hwnd) == handlers.end())
		complain("window handle %p not registered with a %s handler in %s", hwnd, mname, fname);
	handlers.erase(hwnd);
}

void uiWindowsUnregisterWM_COMMANDHandler(HWND hwnd)
{
	unregisterHandler<commandHandlers>(hwnd, "WM_COMMAND", "uiWindowsUnregisterWM_COMMANDHandler()");
}

void uiWindowsUnregisterWM_NOTIFYHandler(HWND hwnd)
{
	unregisterHandler<notifyHandlers>(hwnd, "WM_NOTIFY", "uiWindowsUnregisterWM_NOTIFYHandler()");
}

void uiWindowsUnregisterWM_HSCROLLHandler(HWND hwnd)
{
	unregisterHandler<hscrollHandlers>(hwnd, "WM_HSCROLL", "uiWindowsUnregisterWM_HSCROLLHandler()");
}

template<typename thirdArg, std::map<HWND, typename> &handlers, HWND (*getHWND)(WPARAM, LPARAM), thirdArg (*getThirdArg)(WPARAM, LPARAM)>
static BOOL runHandler(WPARAM wParam, LPARAM lParam, LRESULT *lResult)
{
	HWND control;

	// bounce back to the control in question
	// dn't bounce back if to the utility window, in which case act as if the message was ignored
	control = (*getHWND)(wParam, lParam);
	if (control != NULL && IsChild(utilWindow, control) == 0) {
		if (handlers.find(control) != handlers.end())
			return (*(handlers[control].handler))(handlers[control].c, control, (*getThirdArg)(wParam, lParam), lResult);
		// not registered; fall out to return FALSE
	}
	return FALSE;
}

BOOL runWM_COMMAND(WPARAM wParam, LPARAM lParam, LRESULT *lResult)
{
	return runHandler<WORD, commandHandlers,
		[](WPARAM wParam, LPARAM lParam) {
			return (HWND) lParam;
		},
		[](WPARAM wParam, LPARAM lParam) {
			return HIWORD(wParam);
		}>(wParam, lParam, lResult);
}

BOOL runWM_NOTIFY(WPARAM wParam, LPARAM lParam, LRESULT *lResult)
{
	return runHandler<NMHDR *, notifyHandlers,
		[](WPARAM wParam, LPARAM lParam) {
			return ((NMHDR *) lParam)->hwndFrom;
		},
		[](WPARAM wParam, LPARAM lParam) {
			return (NMHDR *) lParam;
		}>(wParam, lParam, lResult);
}

BOOL runWM_HSCROLL(WPARAM wParam, LPARAM lParam, LRESULT *lResult)
{
	return runHandler<WORD, hscrollHandlers,
		[](WPARAM wParam, LPARAM lParam) {
			return (HWND) lParam;
		},
		[](WPARAM wParam, LPARAM lParam) {
			return LOWORD(wParam);
		}>(wParam, lParam, lResult);
}

static std::map<HWND, bool> wininichanges;

void uiWindowsRegisterReceiveWM_WININICHANGE(HWND hwnd)
{
	if (wininichanges[hwnd])
		complain("window handle %p already subscribed to receive WM_WINICHANGEs in uiWindowsRegisterReceiveWM_WININICHANGE()", hwnd);
	wininichanges[hwnd] = true;
}

void uiWindowsUnregisterReceiveWM_WININICHANGE(HWND hwnd)
{
	if (!wininichanges[hwnd])
		complain("window handle %p not registered to receive WM_WININICHANGEs in uiWindowsUnregisterReceiveWM_WININICHANGE
()", hwnd);
	wininichanges[hwnd] = false;
}

void issueWM_WININICHANGE(WPARAM wParam, LPARAM lParam)
{
	struct wininichange *ch;

	for (const auto &iter : wininichanges)
		SendMessageW(iter.first, WM_WININICHANGE, wParam, lParam);
}
