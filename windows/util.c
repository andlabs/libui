// 6 april 2015
#include "uipriv_windows.h"

// this is a helper function that takes the logic of determining window classes and puts it all in one place
// there are a number of places where we need to know what window class an arbitrary handle has
// theoretically we could use the class atom to avoid a _wcsicmp()
// however, raymond chen advises against this - http://blogs.msdn.com/b/oldnewthing/archive/2004/10/11/240744.aspx (and we're not in control of the Tab class, before you say anything)
// usage: windowClassOf(hwnd, L"class 1", L"class 2", ..., NULL)
int windowClassOf(HWND hwnd, ...)
{
// MSDN says 256 is the maximum length of a class name; add a few characters just to be safe (because it doesn't say whether this includes the terminating null character)
#define maxClassName 260
	WCHAR classname[maxClassName + 1];
	va_list ap;
	WCHAR *curname;
	int i;

	if (GetClassNameW(hwnd, classname, maxClassName) == 0)
		logLastError("error getting name of window class in windowClassOf()");
	va_start(ap, hwnd);
	i = 0;
	for (;;) {
		curname = va_arg(ap, WCHAR *);
		if (curname == NULL)
			break;
		if (_wcsicmp(classname, curname) == 0) {
			va_end(ap);
			return i;
		}
		i++;
	}
	// no match
	va_end(ap);
	return -1;
}

void complain(const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	fprintf(stderr, "[libui] ");
	vfprintf(stderr, fmt, ap);
	fprintf(stderr, "\n");
	va_end(ap);
	DebugBreak();
	abort();		// just in case
}

// wrapper around MapWindowRect() that handles the complex error handling
void mapWindowRect(HWND from, HWND to, RECT *r)
{
	DWORD le;

	SetLastError(0);
	if (MapWindowRect(from, to, r) == 0) {
		le = GetLastError();
		SetLastError(le);		// just to be safe
		if (le != 0)
			logLastError("error calling MapWindowRect() in mapWindowRect()");
	}
}

DWORD getStyle(HWND hwnd)
{
	return (DWORD) GetWindowLongPtrW(hwnd, GWL_STYLE);
}

void setStyle(HWND hwnd, DWORD style)
{
	SetWindowLongPtrW(hwnd, GWL_STYLE, (LONG_PTR) style);
}

DWORD getExStyle(HWND hwnd)
{
	return (DWORD) GetWindowLongPtrW(hwnd, GWL_EXSTYLE);
}

void setExStyle(HWND hwnd, DWORD exstyle)
{
	SetWindowLongPtrW(hwnd, GWL_EXSTYLE, (LONG_PTR) exstyle);
}

void uiWindowsEnsureDestroyWindow(HWND hwnd)
{
	if (DestroyWindow(hwnd) == 0)
		logLastError("error destroying window in uiWindowsEnsureDestroyWindow");
}

void uiWindowsEnsureSetParent(HWND hwnd, HWND parent)
{
	if (SetParent(hwnd, parent) == 0)
		logLastError("error setting window parent in uiWindowsEnsureSetParent");
}

void uiWindowsEnsureAssignControlIDZOrder(HWND hwnd, LONG_PTR controlID, HWND insertAfter)
{
	SetWindowLongPtrW(hwnd, GWLP_ID, controlID);
	setWindowInsertAfter(hwnd, insertAfter);
}

// see http://blogs.msdn.com/b/oldnewthing/archive/2003/09/11/54885.aspx and http://blogs.msdn.com/b/oldnewthing/archive/2003/09/13/54917.aspx
void clientSizeToWindowSize(HWND hwnd, intmax_t *width, intmax_t *height, BOOL hasMenubar)
{
	RECT window;

	window.left = 0;
	window.top = 0;
	window.right = *width;
	window.bottom = *height;
	if (AdjustWindowRectEx(&window, getStyle(hwnd), hasMenubar, getExStyle(hwnd)) == 0)
		logLastError("error getting real window coordinates in clientSizeToWindowSize()");
	if (hasMenubar) {
		RECT temp;

		temp = window;
		temp.bottom = 0x7FFF;		// infinite height
		SendMessageW(hwnd, WM_NCCALCSIZE, (WPARAM) FALSE, (LPARAM) (&temp));
		window.bottom += temp.top;
	}
	*width = window.right - window.left;
	*height = window.bottom - window.top;
}
