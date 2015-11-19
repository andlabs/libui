// 18 november 2015
#include "uipriv_haiku.hpp"

struct uiCheckbox {
	uiHaikuControl c;
	BStringView *dummy;
};

uiHaikuDefineControl(
	uiCheckbox,							// type name
	uiCheckboxType,						// type function
	dummy								// handle
)

char *uiCheckboxText(uiCheckbox *c)
{
	// TODO
	return NULL;
}

void uiCheckboxSetText(uiCheckbox *c, const char *text)
{
	// TODO
}

void uiCheckboxOnToggled(uiCheckbox *c, void (*f)(uiCheckbox *c, void *data), void *data)
{
	// TODO
}

int uiCheckboxChecked(uiCheckbox *c)
{
	// TODO
	return 0;
}

void uiCheckboxSetChecked(uiCheckbox *c, int checked)
{
	// TODO
}

uiCheckbox *uiNewCheckbox(const char *text)
{
	uiCheckbox *c;

	c = (uiCheckbox *) uiNewControl(uiCheckboxType());

	c->dummy = new BStringView(BRect(0, 0, 1, 1), NULL,
		"TODO uiCheckbox not implemented");

	uiHaikuFinishNewControl(c, uiCheckbox);

	return c;
}
