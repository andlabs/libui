// 18 november 2015
#include "uipriv_haiku.hpp"

struct uiEntry {
	uiHaikuControl c;
	BTextControl *tc;
	void (*onChanged)(uiEntry *, void *);
	void *onChangedData;
};

uiHaikuDefineControl(
	uiEntry,								// type name
	uiEntryType,							// type function
	tc									// handle
)

#define mEntryChanged 0x60FE60FE

static void defaultOnChanged(uiEntry *e, void *data)
{
	// do nothing
}

char *uiEntryText(uiEntry *e)
{
	return uiHaikuStrdupText(e->tc->Text());
}

void uiEntrySetText(uiEntry *e, const char *text)
{
	// TODO does this send a message?
	e->tc->SetText(text);
}

void uiEntryOnChanged(uiEntry *e, void (*f)(uiEntry *e, void *data), void *data)
{
	e->onChanged = f;
	e->onChangedData = data;
}

int uiEntryReadOnly(uiEntry *e)
{
	// TODO
	return 0;
}

void uiEntrySetReadOnly(uiEntry *e, int readonly)
{
	// TODO
}

uiEntry *uiNewEntry(void)
{
	uiEntry *e;

	e = (uiEntry *) uiNewControl(uiEntryType());

	e->tc = new BTextControl(NULL, "", new BMessage(mEntryChanged));

	uiEntryOnChanged(e, defaultOnChanged, NULL);

	uiHaikuFinishNewControl(e, uiEntry);

	return e;
}
