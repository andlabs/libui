// 18 november 2015
#include "uipriv_haiku.hpp"

// TODO any of this on api.haiku-os.org?
// TODO tracking events?

struct uiSlider {
	uiHaikuControl c;
	BSlider *slider;
	void (*onChanged)(uiSlider *, void *);
	void *onChangedData;
};

uiHaikuDefineControl(
	uiSlider,								// type name
	uiSliderType,							// type function
	slider								// handle
)

#define mSliderChanged 0x6060FEFE

static void onChanged(BMessage *msg)
{
	void *ss;
	uiSlider *s;

	// TODO error check
	msg->FindPointer(mControlField, &ss);
	s = uiSlider(ss);
	(*(s->onChanged))(s, s->onChangedData);
}

static void defaultOnChanged(uiSlider *s, void *data)
{
	// do nothing
}

intmax_t uiSliderValue(uiSlider *s)
{
	return s->slider->Value();
}

void uiSliderSetValue(uiSlider *s, intmax_t value)
{
	// TODO does this trigger an event?
	s->slider->SetValue(value);
}

void uiSliderOnChanged(uiSlider *s, void (*f)(uiSlider *s, void *data), void *data)
{
	s->onChanged = f;
	s->onChangedData = data;
}

uiSlider *uiNewSlider(intmax_t min, intmax_t max)
{
	uiSlider *s;
	BMessage *msg;

	s = (uiSlider *) uiNewControl(uiSliderType());

	uiHaikuRegisterEventHandler(mSliderChanged, onChanged);
	msg = new BMessage(mSliderChanged);
	msg->AddPointer(mControlField, s);

	s->slider = new BSlider(NULL, NULL, msg,
		min, max, B_HORIZONTAL);

	uiSliderOnChanged(s, defaultOnChanged, NULL);

	uiHaikuFinishNewControl(s, uiSlider);

	return s;
}
