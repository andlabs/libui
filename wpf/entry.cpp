// 26 november 2015
#include "uipriv_wpf.hpp"

struct uiEntry {
	uiWindowsControl c;
	DUMMY dummy;
	void (*onChanged)(uiEntry *, void *);
	void *onChangedData;
};

uiWindowsDefineControl(
	uiEntry,								// type name
	uiEntryType,							// type function
	dummy								// handle
)

static void defaultOnChanged(uiEntry *e, void *data)
{
	// do nothing
}

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

	e->dummy = mkdummy("uiEntry");

	uiEntryOnChanged(e, defaultOnChanged, NULL);

	uiWindowsFinishNewControl(e, uiEntry, dummy);

	return e;
}
