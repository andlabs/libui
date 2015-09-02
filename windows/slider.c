// 20 may 2015
#include "uipriv_windows.h"

struct uiSlider {
	uiWindowsControl c;
	HWND hwnd;
	void (*onChanged)(uiSlider *, void *);
	void *onChangedData;
};

uiWindowsDefineControlWithOnDestroy(
	uiSlider,								// type name
	uiSliderType,							// type function
	uiWindowsUnregisterWM_HSCROLLHandler(this->hwnd);	// on destroy
)

static BOOL onWM_HSCROLL(uiControl *c, HWND hwnd, WORD code, LRESULT *lResult)
{
	uiSlider *s = uiSlider(c);

	(*(s->onChanged))(s, s->onChangedData);
	*lResult = 0;
	return TRUE;
}

// from http://msdn.microsoft.com/en-us/library/windows/desktop/dn742486.aspx#sizingandspacing
#define sliderWidth 107 /* this is actually the shorter progress bar width, but Microsoft doesn't indicate a width */
#define sliderHeight 15

static void minimumSize(uiWindowsControl *c, uiWindowsSizing *d, intmax_t *width, intmax_t *height)
{
	*width = uiWindowsDlgUnitsToX(sliderWidth, d->BaseX);
	*height = uiWindowsDlgUnitsToY(sliderHeight, d->BaseY);
}

static void defaultOnChanged(uiSlider *s, void *data)
{
	// do nothing
}

intmax_t uiSliderValue(uiSlider *s)
{
	return (intmax_t) SendMessageW(s->hwnd, TBM_GETPOS, 0, 0);
}

void uiSliderSetValue(uiSlider *s, intmax_t value)
{
	// don't use TBM_SETPOSNOTIFY; that triggers an event
	SendMessageW(s->hwnd, TBM_SETPOS, (WPARAM) TRUE, (LPARAM) value);
}

void uiSliderOnChanged(uiSlider *s, void (*f)(uiSlider *, void *), void *data)
{
	s->onChanged = f;
	s->onChangedData = data;
}

uiSlider *uiNewSlider(intmax_t min, intmax_t max)
{
	uiSlider *s;

	s = (uiSlider *) uiNewControl(uiSliderType());

	s->hwnd = uiWindowsEnsureCreateControlHWND(0,
		TRACKBAR_CLASSW, L"",
		TBS_HORZ | TBS_TOOLTIPS | TBS_TRANSPARENTBKGND | WS_TABSTOP,
		hInstance, NULL,
		TRUE);

	uiWindowsRegisterWM_HSCROLLHandler(s->hwnd, onWM_HSCROLL, uiControl(s));
	uiSliderOnChanged(s, defaultOnChanged, NULL);

	SendMessageW(s->hwnd, TBM_SETRANGEMIN, (WPARAM) TRUE, (LPARAM) min);
	SendMessageW(s->hwnd, TBM_SETRANGEMAX, (WPARAM) TRUE, (LPARAM) max);
	SendMessageW(s->hwnd, TBM_SETPOS, (WPARAM) TRUE, (LPARAM) min);

	uiWindowsFinishNewControl(s, uiSlider);

	return s;
}
