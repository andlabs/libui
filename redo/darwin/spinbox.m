// 11 june 2015
#include "uipriv_darwin.h"

struct spinbox {
	uiSpinbox s;
	NSTextField *dummy;
	void (*onChanged)(uiSpinbox *, void *);
	void *onChangedData;
};

uiDefineControlType(uiSpinbox, uiTypeSpinbox, struct spinbox)

static uintptr_t spinboxHandle(uiControl *c)
{
	struct spinbox *s = (struct spinbox *) c;

	return (uintptr_t) (s->dummy);
}

static void defaultOnChanged(uiSpinbox *s, void *data)
{
	// do nothing
}

static intmax_t spinboxValue(uiSpinbox *ss)
{
	struct spinbox *s = (struct spinbox *) ss;

	return PUT_CODE_HERE;
}

static void spinboxSetValue(uiSpinbox *ss, intmax_t value)
{
	struct spinbox *s = (struct spinbox *) ss;

	PUT_CODE_HERE;
}

static void spinboxOnChanged(uiSpinbox *ss, void (*f)(uiSpinbox *, void *), void *data)
{
	struct spinbox *s = (struct spinbox *) ss;

	s->onChanged = f;
	s->onChangedData = data;
}

uiSpinbox *uiNewSpinbox(intmax_t min, intmax_t max)
{
	struct spinbox *s;

	if (min >= max)
		complain("error: min >= max in uiNewSpinbox()");

	s = (struct spinbox *) uiNewControl(uiTypeSpinbox());

	s->dummy = [[NSTextField alloc] initWithFrame:NSZeroRect];
	[s->dummy setStringValue:@"TODO uiSpinbox not implemented"];
	uiDarwinMakeSingleViewControl(uiControl(s), s->dummy, YES);

	s->onChanged = defaultOnChanged;

	uiControl(s)->Handle = spinboxHandle;

	uiSpinbox(s)->Value = spinboxValue;
	uiSpinbox(s)->SetValue = spinboxSetValue;
	uiSpinbox(s)->OnChanged = spinboxOnChanged;

	return uiSpinbox(s);
}
