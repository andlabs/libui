// 11 june 2015
#include "uipriv_darwin.h"

struct combobox {
	uiCombobox c;
	BOOL editable;
	NSPopUpButton *pb;
	NSComboBox *cb;
};

uiDefineControlType(uiCombobox, uiTypeCombobox, struct combobox)

static uintptr_t comboboxHandle(uiControl *cc)
{
	struct combobox *c = (struct combobox *) cc;

	if (c->editable)
		return (uintptr_t) (c->cb);
	return (uintptr_t) (c->pb);
}

static void comboboxAppend(uiCombobox *cc, const char *text)
{
	struct combobox *c = (struct combobox *) cc;

	PUT_CODE_HERE;
}

static uiCombobox *finishNewCombobox(BOOL editable)
{
	struct combobox *c;

	c = (struct combobox *) uiNewControl(uiTypeCombobox());

	c->editable = editable;
	if (c->editable) {
		c->cb = [[NSComboBox alloc] initWithFrame:NSZeroRect];
		[c->cb setUsesDataSource:NO];
		[c->cb setButtonBordered:YES];
NSLog(@"TEST intercellSpacing %@", NSStringFromSize([c->cb intercellSpacing]);
		[c->cb setCompletes:NO];
		uiDarwinMakeSingleViewControl(uiControl(c), c->cb, YES);
NSLog(@"TEST intercellSpacing %@", NSStringFromSize([c->cb intercellSpacing]);
	} else {
		c->pb = [[NSPopUpButton alloc] initWithFrame:NSZeroRect pullsDown:NO];
		// TODO preferred edge
		// TODO arrow position
		// TODO font
		uiDarwinMakeSingleViewControl(uiControl(c), c->cb, YES);
	}

	uiControl(c)->Handle = comboboxHandle;

	uiCombobox(c)->Append = comboboxAppend;

	return uiCombobox(c);
}

uiCombobox *uiNewCombobox(void)
{
	return finishNewCombobox(NO);
}

uiCombobox *uiNewEditableCombobox(void)
{
	return finishNewCombobox(YES);
}
