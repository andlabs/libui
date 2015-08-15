// 14 august 2015
#include "uipriv_darwin.h"

struct uiCombobox {
	uiDarwinControl c;
	BOOL editable;
	NSPopUpButton *pb;
	NSComboBox *cb;
	uintptr_t handle;			// for uiControlHandle()
};

uiDarwinDefineControl(
	uiCombobox,							// type name
	uiComboboxType,						// type function
	handle								// handle
)

void uiComboboxAppend(uiCombobox *c, const char *text)
{
	// TODO
}

static uiCombobox *finishNewCombobox(BOOL editable)
{
	uiCombobox *c;

	c = (uiCombobox *) uiNewControl(uiComboboxType());

	c->editable = editable;
	if (c->editable) {
		c->cb = [[NSComboBox alloc] initWithFrame:NSZeroRect];
		[c->cb setUsesDataSource:NO];
		[c->cb setButtonBordered:YES];
		[c->cb setCompletes:NO];
		uiDarwinSetControlFont(c->cb, NSRegularControlSize);
		c->handle = (uintptr_t) (c->cb);
	} else {
		c->pb = [[NSPopUpButton alloc] initWithFrame:NSZeroRect pullsDown:NO];
		// TODO preferred edge
		// TODO arrow position
		// TODO font
		c->handle = (uintptr_t) (c->pb);
	}

	uiDarwinFinishNewControl(c, uiCombobox);

	return c;
}

uiCombobox *uiNewCombobox(void)
{
	return finishNewCombobox(NO);
}

uiCombobox *uiNewEditableCombobox(void)
{
	return finishNewCombobox(YES);
}
