// 14 august 2015
#include "uipriv_darwin.h"

struct uiCombobox {
	uiDarwinControl c;
	BOOL editable;
	NSPopUpButton *pb;
	NSArrayController *pbac;
	NSComboBox *cb;
	NSObject *handle;				// for uiControlHandle()
};

static void onDestroy(uiCombobox *);

uiDarwinDefineControlWithOnDestroy(
	uiCombobox,							// type name
	uiComboboxType,						// type function
	handle,								// handle
	onDestroy(this);						// on destroy
)

static void onDestroy(uiCombobox *c)
{
	if (!c->editable) {
		[c->pb unbind:@"contentObjects"];
		[c->pb unbind:@"selectedIndex"];
		[c->pbac release];
	}
}

void uiComboboxAppend(uiCombobox *c, const char *text)
{
	if (c->editable)
		[c->cb addItemWithObjectValue:toNSString(text)];
	else
		[c->pbac addObject:toNSString(text)];
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

		// NSPopUpButton doesn't work like a combobox
		// - it automatically selects the first item
		// - it doesn't support duplicates
		// but we can use a NSArrayController and Cocoa bindings to bypass these restrictions
		c->pbac = [NSArrayController new];
		[c->pbac setAvoidsEmptySelection:NO];
		[c->pbac setSelectsInsertedObjects:NO];
		[c->pbac setAutomaticallyRearrangesObjects:NO];
		[c->pb bind:@"contentValues"
			toObject:c->pbac
			withKeyPath:@"arrangedObjects"
			options:nil];
		[c->pb bind:@"selectedIndex"
			toObject:c->pbac
			withKeyPath:@"selectionIndex"
			options:nil];
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
