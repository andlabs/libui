// 18 november 2015
#include "uipriv_haiku.hpp"

struct uiEntry {
	uiHaikuControl c;
	BStringView *dummy;
};

uiHaikuDefineControl(
	uiEntry,								// type name
	uiEntryType,							// type function
	dummy								// handle
)

char *uiEntryText(uiEntry *e)
{
	// TODO
	return NULL;
}

void uiEntrySetText(uiEntry *e, const char *text)
{
	// TODO
}

void uiEntryOnChanged(uiEntry *e, void (*f)(uiEntry *e, void *data), void *data)
{
	// TODO
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

	e->dummy = new BStringView(BRect(0, 0, 1, 1), NULL,
		"TODO uiEntry not implemented");

	uiHaikuFinishNewControl(e, uiEntry);

	return e;
}
