// 14 august 2015
#include "uipriv_darwin.h"

struct uiCombobox {
	uiDarwinControl c;
	BOOL editable;
	NSPopUpButton *pb;
	NSComboBox *cb;
	NSObject *handle;				// for uiControlHandle()
};

uiDarwinDefineControl(
	uiCombobox,							// type name
	uiComboboxType,						// type function
	handle								// handle
)

void uiComboboxAppend(uiCombobox *c, const char *text)
{
	if (c->editable)
		[c->cb addItemWithObjectValue:toNSString(text)];
	else
		[c->pb addItemWithTitle:toNSString(text)];
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
		c->handle = c->cb;
	} else {
		NSPopUpButtonCell *pbcell;

		c->pb = [[NSPopUpButton alloc] initWithFrame:NSZeroRect pullsDown:NO];
		[c->pb setPreferredEdge:NSMinYEdge];
		pbcell = (NSPopUpButtonCell *) [c->pb cell];
		[pbcell setArrowPosition:NSPopUpArrowAtBottom];
		// TODO font
		c->handle = c->pb;
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
