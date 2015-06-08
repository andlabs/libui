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

// TODO refine this
void uiLog(const char *fmt, ...)
{
	va_list ap;
	char buf[2048];

	va_start(ap, fmt);
	vsnprintf(buf, 2044, fmt, ap);
	va_end(ap);
	// unfortunately we need to make sure this is callable even before uiInit() executes or after uiUninit() has executed
	// that means we can't use toUTF16(), because that requires our private heap
	// sadly that means we're stuck with OutputDebugStringA() without some serious refinement
	OutputDebugStringA(buf);
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
