// 11 june 2015
#include "uipriv_darwin.h"

// TODO events

struct slider {
	uiSlider s;
	NSSlider *slider;
	void (*onChanged)(uiSlider *, void *);
	void *onChangedData;
};

uiDefineControlType(uiSlider, uiTypeSlider, struct slider)

static uintptr_t sliderHandle(uiControl *c)
{
	struct slider *s = (struct slider *) c;

	return (uintptr_t) (s->slider);
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
	NSSliderCell *cell;

	s = (struct slider *) uiNewControl(uiTypeSlider());

	s->slider = [[NSSlider alloc] initWithFrame:NSZeroRect];
	// TODO vertical is defined by wider than tall
	[s->slider setMinValue:min];
	[s->slider setMaxValue:max];
	[s->slider setAllowsTickMarkValuesOnly:NO];
	[s->slider setNumberOfTickMarks:0];
	[s->slider setTickMarkPosition:NSTickMarkAbove];

	cell = (NSSliderCell *) [s->slider cell];
	[cell setSliderType:NSLinearSlider];

	uiDarwinMakeSingleViewControl(uiControl(s), s->slider, NO);

	s->onChanged = defaultOnChanged;

	uiControl(s)->Handle = sliderHandle;

	uiSlider(s)->Value = sliderValue;
	uiSlider(s)->SetValue = sliderSetValue;
	uiSlider(s)->OnChanged = sliderOnChanged;

	return uiSlider(s);
}
