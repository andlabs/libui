// 14 august 2015
#import "uipriv_darwin.h"

// TODO events

type uiSlider {
	uiDarwinControl c;
	NSSlider *slider;
	void (*onChanged)(uiSlider *, void *);
	void *onChangedData;
};

uiDarwinDefineControl(
	uiSlider,								// type name
	uiSliderType,							// type function
	slider								// handle
)

intmax_t uiSliderValue(uiSlider *s)
{
	return PUT_CODE_HERE;
}

void uiSliderSetValue(uiSlider *s, intmax_t value)
{
	// TODO
}

void uiSliderOnChanged(uiSlider *s, void (*f)(uiSlider *, void *), void *data)
{
	s->onChanged = f;
	s->onChangedData = data;
}

static void defaultOnChanged(uiSlider *s, void *data)
{
	// do nothing
}

uiSlider *uiNewSlider(intmax_t min, intmax_t max)
{
	uiSlider *s;
	NSSliderCell *cell;

	s = (uiSlider *) uiNewControl(uiSliderType());

	s->slider = [[NSSlider alloc] initWithFrame:NSZeroRect];
	// TODO vertical is defined by wider than tall
	[s->slider setMinValue:min];
	[s->slider setMaxValue:max];
	[s->slider setAllowsTickMarkValuesOnly:NO];
	[s->slider setNumberOfTickMarks:0];
	[s->slider setTickMarkPosition:NSTickMarkAbove];

	cell = (NSSliderCell *) [s->slider cell];
	[cell setSliderType:NSLinearSlider];

	uiSliderOnChanged(s, defaultOnChanged, NULL);

	uiDarwinFinishNewControl(s, uiSlider);

	return s;
}
