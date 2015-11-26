// 26 november 2015
#include "uipriv_wpf.hpp"

struct uiCheckbox {
	uiWindowsControl c;
	gcroot<CheckBox ^> *checkbox;
	void (*onToggled)(uiCheckbox *, void *);
	void *onToggledData;
};

uiWindowsDefineControl(
	uiCheckbox,							// type name
	uiCheckboxType,						// type function
	checkbox								// handle
)

static void defaultOnToggled(uiCheckbox *c, void *data)
{
	// do nothing
}

char *uiCheckboxText(uiCheckbox *c)
{
	String ^text;

	// TOOD bad cast?
	text = (String ^) ((*(c->checkbox))->Content);
	return uiWindowsCLRStringToText(text);
}

void uiCheckboxSetText(uiCheckbox *c, const char *text)
{
	(*(c->checkbox))->Content = fromUTF8(text);
}

void uiCheckboxOnToggled(uiCheckbox *c, void (*f)(uiCheckbox *c, void *data), void *data)
{
	c->onToggled = f;
	c->onToggledData = data;
}

int uiCheckboxChecked(uiCheckbox *c)
{
	return (*(c->checkbox))->IsChecked.Value != false;
}

void uiCheckboxSetChecked(uiCheckbox *c, int checked)
{
	bool value;

	value = checked != 0;
	// TODO does this trigger an event?
	(*(c->checkbox))->IsChecked = value;
}

uiCheckbox *uiNewCheckbox(const char *text)
{
	uiCheckbox *c;

	c = (uiCheckbox *) uiNewControl(uiCheckboxType());

	c->checkbox = new gcroot<CheckBox ^>();
	*(c->checkbox) = gcnew CheckBox();
	(*(c->checkbox))->Content = fromUTF8(text);

	uiCheckboxOnToggled(c, defaultOnToggled, NULL);

	uiWindowsFinishNewControl(c, uiCheckbox, checkbox);

	return c;
}
