// 26 november 2015
#include "uipriv_wpf.hpp"

struct uiCombobox {
	uiWindowsControl c;
	DUMMY dummy;
};

uiWindowsDefineControl(
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
	// return 0 so the area test can work
	return 0;
}

void uiComboboxSetSelected(uiCombobox *c, intmax_t n)
{
	// TODO
}

void uiComboboxOnSelected(uiCombobox *c, void (*f)(uiCombobox *c, void *data), void *data)
{
	// TODO
}

static uiCombobox *finishNewCombobox(bool editable)
{
	uiCombobox *c;

	c = (uiCombobox *) uiNewControl(uiComboboxType());

	c->dummy = mkdummy(L"uiCombobox");

//	(*(c->combobox))->IsReadOnly = editable;

	uiWindowsFinishNewControl(c, uiCombobox, dummy);

	return c;
}

uiCombobox *uiNewCombobox(void)
{
	return finishNewCombobox(true);
}

uiCombobox *uiNewEditableCombobox(void)
{
	return finishNewCombobox(false);
}
