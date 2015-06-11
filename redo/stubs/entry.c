// 11 june 2015
#include "uipriv_OSHERE.h"

struct entry {
	uiEntry e;
	OSTYPE OSHANDLE;
	void (*onChanged)(uiEntry *, void *);
	void *onChangedData;
};

uiDefineControlType(uiEntry, uiTypeEntry, struct entry)

static uintptr_t entryHandle(uiControl *c)
{
	struct entry *e = (struct entry *) c;

	return (uintptr_t) (e->OSHANDLE);
}

static void defaultOnChanged(uiEntry *e, void *data)
{
	// do nothing
}

static char *entryText(uiEntry *ee)
{
	struct entry *e = (struct entry *) ee;

	return PUT_CODE_HERE;
}

static void entrySetText(uiEntry *ee, const char *text)
{
	struct entry *e = (struct entry *) ee;

	PUT_CODE_HERE;
	// don't queue the control for resize; entry sizes are independent of their contents
}

static void entryOnChanged(uiEntry *ee, void (*f)(uiEntry *, void *), void *data)
{
	struct entry *e = (struct entry *) ee;

	e->onChanged = f;
	e->onChangedData = data;
}

static int entryReadOnly(uiEntry *ee)
{
	struct entry *e = (struct entry *) ee;

	return PUT_CODE_HERE;
}

static void entrySetReadOnly(uiEntry *ee, int readonly)
{
	struct entry *e = (struct entry *) ee;
	WPARAM ro;

	PUT_CODE_HERE;
}

uiEntry *uiNewEntry(void)
{
	struct entry *e;

	e = (struct entry *) MAKE_CONTROL_INSTANCE(uiTypeEntry());

	PUT_CODE_HERE;

	e->onChanged = defaultOnChanged;

	uiControl(e)->Handle = entryHandle;

	uiEntry(e)->Text = entryText;
	uiEntry(e)->SetText = entrySetText;
	uiEntry(e)->OnChanged = entryOnChanged;
	uiEntry(e)->ReadOnly = entryReadOnly;
	uiEntry(e)->SetReadOnly = entrySetReadOnly;

	return uiEntry(e);
}
