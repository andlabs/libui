// 20 may 2015
#include "uipriv_windows.h"

// references:
// - http://stackoverflow.com/questions/2892703/how-do-i-draw-separators
// - https://msdn.microsoft.com/en-us/library/windows/desktop/dn742405%28v=vs.85%29.aspx

struct separator {
	uiSeparator s;
	HWND hwnd;
};

static BOOL onWM_HSCROLL(uiControl *c, WORD code, LRESULT *lResult)
{
	return FALSE;
}

static void onDestroy(void *data)
{
	struct separator *s = (struct separator *) data;

	uiFree(s);
}

// via http://msdn.microsoft.com/en-us/library/windows/desktop/dn742486.aspx#sizingandspacing
// TODO
// TODO DPI independence?

static void separatorPreferredSize(uiControl *c, uiSizing *d, intmax_t *width, intmax_t *height)
{
	*width = 1;		// TODO
	*height = 2;		// what Spy++ says Windows XP's Explorer uses
}

uiSeparator *uiNewHorizontalSeparator(void)
{
	struct separator *s;
	uiWindowsMakeControlParams p;

	s = uiNew(struct separator);
	uiTyped(s)->Type = uiTypeSeparator();

	p.dwExStyle = 0;
	p.lpClassName = L"static";
	p.lpWindowName = L"";
	p.dwStyle = SS_ETCHEDHORZ;
	p.hInstance = hInstance;
	p.lpParam = NULL;
	p.useStandardControlFont = TRUE;
	p.onWM_HSCROLL = onWM_HSCROLL;
	p.onDestroy = onDestroy;
	p.onDestroyData = s;
	uiWindowsMakeControl(uiControl(s), &p);

	s->hwnd = (HWND) uiControlHandle(uiControl(s));

	uiControl(s)->PreferredSize = separatorPreferredSize;

	return uiSeparator(s);
}
