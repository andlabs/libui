// 18 november 2015
#include "uipriv_haiku.hpp"

// TODOs
// - checkbox text isn't aligned with the checkbox

struct uiCheckbox {
	uiHaikuControl c;
	BCheckBox *checkbox;
	void (*onToggled)(uiCheckbox *, void *);
	void *onToggledData;
};

uiHaikuDefineControl(
	uiCheckbox,							// type name
	uiCheckboxType,						// type function
	checkbox								// handle
)

#define mCheckboxToggled 0x4E714E71

static void defaultOnToggled(uiCheckbox *c, void *data)
{
	// do nothing
}

char *uiCheckboxText(uiCheckbox *c)
{
	// TODO not on api.haiku-os.org? or is this not right?
	return uiHaikuStrdupText(c->checkbox->Label());
}

void uiCheckboxSetText(uiCheckbox *c, const char *text)
{
	// TODO not on api.haiku-os.org? or is this not right?
	c->checkbox->SetLabel(text);
}

void uiCheckboxOnToggled(uiCheckbox *c, void (*f)(uiCheckbox *c, void *data), void *data)
{
	c->onToggled = f;
	c->onToggledData = data;
}

int uiCheckboxChecked(uiCheckbox *c)
{
	return c->checkbox->Value() != B_CONTROL_OFF;
}

void uiCheckboxSetChecked(uiCheckbox *c, int checked)
{
	int32 value;

	value = B_CONTROL_OFF;
	if (checked)
		value = B_CONTROL_ON;
	// TODO does this trigger an event?
	c->checkbox->SetValue(value);
}

uiCheckbox *uiNewCheckbox(const char *text)
{
	uiCheckbox *c;

	c = (uiCheckbox *) uiNewControl(uiCheckboxType());

	c->checkbox = new BCheckBox(text, new BMessage(mCheckboxToggled));

	uiCheckboxOnToggled(c, defaultOnToggled, NULL);

	uiHaikuFinishNewControl(c, uiCheckbox);

	return c;
}
