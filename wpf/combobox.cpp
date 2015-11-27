// 26 november 2015
#include "uipriv_wpf.hpp"

struct uiCombobox {
	uiWindowsControl c;
	gcroot<ComboBox ^> *combobox;
	void (*onSelected)(uiCombobox *, void *);
	void *onSelectedData;
};

uiWindowsDefineControl(
	uiCombobox,							// type name
	uiComboboxType,						// type function
	combobox							// handle
)

static void defaultOnSelected(uiCombobox *c, void *data)
{
	// do nothing
}

void uiComboboxAppend(uiCombobox *c, const char *text)
{
	(*(c->combobox))->Items->Add(fromUTF8(text));
}

intmax_t uiComboboxSelected(uiCombobox *c)
{
	// TODO what happens on an editable combobox?
	return (*(c->combobox))->SelectedIndex;
}

void uiComboboxSetSelected(uiCombobox *c, intmax_t n)
{
	(*(c->combobox))->SelectedIndex = n;
}

void uiComboboxOnSelected(uiCombobox *c, void (*f)(uiCombobox *c, void *data), void *data)
{
	c->onSelected = f;
	c->onSelectedData = data;
}

static uiCombobox *finishNewCombobox(bool editable)
{
	uiCombobox *c;

	c = (uiCombobox *) uiNewControl(uiComboboxType());

	c->combobox = new gcroot<ComboBox ^>();
	*(c->combobox) = gcnew ComboBox();
	(*(c->combobox))->IsEditable = editable;

	uiComboboxOnSelected(c, defaultOnSelected, NULL);

	uiWindowsFinishNewControl(c, uiCombobox, combobox);

	return c;
}

uiCombobox *uiNewCombobox(void)
{
	return finishNewCombobox(false);
}

uiCombobox *uiNewEditableCombobox(void)
{
	return finishNewCombobox(true);
}
