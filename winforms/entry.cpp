// 26 november 2015
#include "uipriv_winforms.hpp"

struct uiEntry {
	uiWindowsControl c;
	gcroot<TextBox ^> *textbox;
	void (*onChanged)(uiEntry *, void *);
	void *onChangedData;
};

uiWindowsDefineControl(
	uiEntry,								// type name
	uiEntryType,							// type function
	textbox								// handle
)

static void defaultOnChanged(uiEntry *e, void *data)
{
	// do nothing
}

char *uiEntryText(uiEntry *e)
{
	return uiWindowsCLRStringToText((*(e->textbox))->Text);
}

void uiEntrySetText(uiEntry *e, const char *text)
{
	(*(e->textbox))->Text = fromUTF8(text);
}

void uiEntryOnChanged(uiEntry *e, void (*f)(uiEntry *e, void *data), void *data)
{
	e->onChanged = f;
	e->onChangedData = data;
}

int uiEntryReadOnly(uiEntry *e)
{
	return (*(e->textbox))->IsReadOnly != false;
}

void uiEntrySetReadOnly(uiEntry *e, int readonly)
{
	(*(e->textbox))->IsReadOnly = readonly != 0;
}

uiEntry *uiNewEntry(void)
{
	uiEntry *e;

	e = (uiEntry *) uiNewControl(uiEntryType());

	e->textbox = new gcroot<TextBox ^>();
	*(e->textbox) = gcnew TextBox();

	uiEntryOnChanged(e, defaultOnChanged, NULL);

	uiWindowsFinishNewControl(e, uiEntry, textbox);

	return e;
}
