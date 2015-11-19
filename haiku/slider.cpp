// 18 november 2015
#include "uipriv_haiku.hpp"

struct uiSlider {
	uiHaikuControl c;
	BStringView *dummy;
};

uiHaikuDefineControl(
	uiSlider,								// type name
	uiSliderType,							// type function
	dummy								// handle
)

intmax_t uiSliderValue(uiSlider *s)
{
	// TODO
	return 0;
}

void uiSliderSetValue(uiSlider *s, intmax_t value)
{
	// TODO
}

void uiSliderOnChanged(uiSlider *s, void (*f)(uiSlider *s, void *data), void *data)
{
	// TODO
}

uiSlider *uiNewSlider(intmax_t min, intmax_t max)
{
	uiSlider *s;

	s = (uiSlider *) uiNewControl(uiSliderType());

	s->dummy = new BStringView(BRect(0, 0, 1, 1), NULL,
		"TODO uiSlider not implemented");

	uiHaikuFinishNewControl(s, uiSlider);

	return s;
}
