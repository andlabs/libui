// 20 may 2015
#include "uipriv_windows.h"

// TODOs
// - investigate overriding WM_ERASEBKGND to simulate TBS_TRANSPARENTBKGND; windows xp tries to do this it seems for tab controls but doesn't really do a good job of it
// - wine does not clamp TBM_SETPOS

struct slider {
	uiSlider s;
	HWND hwnd;
	void (*baseResize)(uiControl *, intmax_t, intmax_t, intmax_t, intmax_t, uiSizing *);
	void (*onChanged)(uiSlider *, void *);
	void *onChangedData;
	void (*baseCommitDestroy)(uiControl *);
};

uiDefineControlType(uiSlider, uiTypeSlider, struct slider)

static BOOL onWM_HSCROLL(uiControl *c, HWND hwnd, WORD code, LRESULT *lResult)
{
	struct slider *s = (struct slider *) c;

	(*(s->onChanged))(uiSlider(s), s->onChangedData);
	*lResult = 0;
	return TRUE;
}

static void sliderCommitDestroy(uiControl *c)
{
	struct slider *s = (struct slider *) c;

	uiWindowsUnregisterWM_HSCROLLHandler(s->hwnd);
	(*(s->baseCommitDestroy))(uiControl(s));
}

static uintptr_t sliderHandle(uiControl *c)
{
	struct slider *s = (struct slider *) c;

	return (uintptr_t) (s->hwnd);
}

// from http://msdn.microsoft.com/en-us/library/windows/desktop/dn742486.aspx#sizingandspacing
#define sliderWidth 107 /* this is actually the shorter progress bar width, but Microsoft doesn't indicate a width */
#define sliderHeight 15

static void sliderPreferredSize(uiControl *c, uiSizing *d, intmax_t *width, intmax_t *height)
{
	*width = uiWindowsDlgUnitsToX(sliderWidth, d->Sys->BaseX);
	*height = uiWindowsDlgUnitsToY(sliderHeight, d->Sys->BaseY);
}

// TODO does it go here relative of other things?
static void defaultOnChanged(uiSlider *s, void *data)
{
	// do nothing
}

static intmax_t sliderValue(uiSlider *ss)
{
	struct slider *s = (struct slider *) ss;

	return (intmax_t) SendMessageW(s->hwnd, TBM_GETPOS, 0, 0);
}

static void sliderSetValue(uiSlider *ss, intmax_t value)
{
	struct slider *s = (struct slider *) ss;

	// don't use TBM_SETPOSNOTIFY; that triggers an event
	SendMessageW(s->hwnd, TBM_SETPOS, (WPARAM) TRUE, (LPARAM) value);
}

static void sliderOnChanged(uiSlider *ss, void (*f)(uiSlider *, void *), void *data)
{
	struct slider *s = (struct slider *) ss;

	s->onChanged = f;
	s->onChangedData = data;
}

uiSlider *uiNewSlider(intmax_t min, intmax_t max)
{
	struct slider *s;

	s = (struct slider *) uiWindowsNewSingleHWNDControl(uiTypeSlider());

	s->hwnd = uiWindowsNewSingleHWNDControl(0,
		TRACKBAR_CLASSW, L"",
		// TODO TBS_TRANSPARENTBKGND when making Vista-only
		TBS_HORZ | TBS_TOOLTIPS | WS_TABSTOP,
		hInstance, NULL,
		TRUE);

	uiWindowsRegisterWM_HSCROLLHandler(s->hwnd, onWM_HSCROLL, uiControl(s));

	SendMessageW(s->hwnd, TBM_SETRANGEMIN, (WPARAM) TRUE, (LPARAM) min);
	SendMessageW(s->hwnd, TBM_SETRANGEMAX, (WPARAM) TRUE, (LPARAM) max);
	SendMessageW(s->hwnd, TBM_SETPOS, (WPARAM) TRUE, (LPARAM) min);

	s->onChanged = defaultOnChanged;

	uiControl(s)->Handle = sliderHandle;
	uiControl(s)->PreferredSize = sliderPreferredSize;
	s->baseCommitDestroy = uiControl(s)->CommitDestroy;
	uiControl(s)->CommitDestroy = sliderCommitDestroy;

	uiSlider(s)->Value = sliderValue;
	uiSlider(s)->SetValue = sliderSetValue;
	uiSlider(s)->OnChanged = sliderOnChanged;

	return uiSlider(s);
}
