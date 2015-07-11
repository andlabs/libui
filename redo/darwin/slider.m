// 11 june 2015
#include "uipriv_OSHERE.h"

struct slider {
	uiSlider s;
	OSTYPE OSHANDLE;
	void (*onChanged)(uiSlider *, void *);
	void *onChangedData;
};

uiDefineControlType(uiSlider, uiTypeSlider, struct slider)

static uintptr_t sliderHandle(uiControl *c)
{
	struct slider *s = (struct slider *) c;

	return (uintptr_t) (s->OSHANDLE);
}

static void defaultOnChanged(uiSlider *s, void *data)
{
	// do nothing
}

static intmax_t sliderValue(uiSlider *ss)
{
	struct slider *s = (struct slider *) ss;

	return PUT_CODE_HERE;
}

static void sliderSetValue(uiSlider *ss, intmax_t value)
{
	struct slider *s = (struct slider *) ss;

	PUT_CODE_HERE;
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

	s = (struct slider *) MAKE_CONTROL_INSTANCE(uiTypeSlider());

	PUT_CODE_HERE;

	s->onChanged = defaultOnChanged;

	uiControl(s)->Handle = sliderHandle;

	uiSlider(s)->Value = sliderValue;
	uiSlider(s)->SetValue = sliderSetValue;
	uiSlider(s)->OnChanged = sliderOnChanged;

	return uiSlider(s);
}
