// 18 november 2015
#include "uipriv_haiku.hpp"

struct uiCombobox {
	uiHaikuControl c;
	BStringView *dummy;
};

uiHaikuDefineControl(
	uiCombobox,							// type name
	uiComboboxType,						// type function
	dummy								// handle
)

void uiComboboxAppend(uiCombobox *c, const char *text)
{
	// TODO
}

intmax_t uiComboboxSelected(uiCombobox *c)
{
	// TODO
	return -1;
}

void uiComboboxSetSelected(uiCombobox *c, intmax_t n)
{
	// TODO
}

void uiComboboxOnSelected(uiCombobox *c, void (*f)(uiCombobox *c, void *data), void *data)
{
	// TODO
}

static uiCombobox *finishNewCombobox(void)
{
	uiCombobox *c;

	c = (uiCombobox *) uiNewControl(uiComboboxType());

	c->dummy = new BStringView(BRect(0, 0, 1, 1), NULL,
		"TODO uiCombobox not implemented");

	uiHaikuFinishNewControl(c, uiCombobox);

	return c;
}

uiCombobox *uiNewCombobox(void)
{
	return finishNewCombobox();
}

uiCombobox *uiNewEditableCombobox(void)
{
	return finishNewCombobox();
}
