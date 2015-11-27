// 18 november 2015
#include "uipriv_winforms.hpp"

struct uiSlider {
	uiWindowsControl c;
	DUMMY dummy;
	void (*onChanged)(uiSlider *, void *);
	void *onChangedData;
};

uiWindowsDefineControl(
	uiSlider,								// type name
	uiSliderType,							// type function
	dummy								// handle
)

static void defaultOnChanged(uiSlider *s, void *data)
{
	// do nothing
}

intmax_t uiSliderValue(uiSlider *s)
{
	// TODO
	return -1;
}

void uiSliderSetValue(uiSlider *s, intmax_t value)
{
	// TODO
}

void uiSliderOnChanged(uiSlider *s, void (*f)(uiSlider *s, void *data), void *data)
{
	s->onChanged = f;
	s->onChangedData = data;
}

uiSlider *uiNewSlider(intmax_t min, intmax_t max)
{
	uiSlider *s;

	s = (uiSlider *) uiNewControl(uiSliderType());

	s->dummy = mkdummy(L"uiSlider");

	uiSliderOnChanged(s, defaultOnChanged, NULL);

	uiWindowsFinishNewControl(s, uiSlider, dummy);

	return s;
}
