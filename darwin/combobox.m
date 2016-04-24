// 14 august 2015
#import "uipriv_darwin.h"

// NSComboBoxes have no intrinsic width; we'll use the default Interface Builder width for them.
// NSPopUpButton is fine.
#define comboboxWidth 96

@interface libui_intrinsicWidthNSComboBox : NSComboBox
@end

@implementation libui_intrinsicWidthNSComboBox

- (NSSize)intrinsicContentSize
{
	NSSize s;

	s = [super intrinsicContentSize];
	s.width = comboboxWidth;
	return s;
}

@end

struct uiCombobox {
	uiDarwinControl c;
	BOOL editable;
	NSPopUpButton *pb;
	NSArrayController *pbac;
	NSComboBox *cb;
	NSObject *handle;				// for uiControlHandle()
	void (*onSelected)(uiCombobox *, void *);
	void *onSelectedData;
};

@interface comboboxDelegateClass : NSObject<NSComboBoxDelegate> {
	struct mapTable *comboboxes;
}
- (void)comboBoxSelectionDidChange:(NSNotification *)note;
- (IBAction)onSelected:(id)sender;
- (void)registerCombobox:(uiCombobox *)c;
- (void)unregisterCombobox:(uiCombobox *)c;
@end

@implementation comboboxDelegateClass

- (id)init
{
	self = [super init];
	if (self)
		self->comboboxes = newMap();
	return self;
}

- (void)dealloc
{
	mapDestroy(self->comboboxes);
	[super dealloc];
}

// note: does not trigger when text changed
// TODO not perfect either:
// - triggered when keyboard navigating the open menu
// - does not trigger when the text is changed to a menu item (which normally selects that item; IDK how to inhibit that behavior - TODO)
- (void)comboBoxSelectionDidChange:(NSNotification *)note
{
	[self onSelected:[note object]];
}

- (IBAction)onSelected:(id)sender
{
	uiCombobox *c;

	c = (uiCombobox *) mapGet(self->comboboxes, sender);
	(*(c->onSelected))(c, c->onSelectedData);
}

- (void)registerCombobox:(uiCombobox *)c
{
	mapSet(self->comboboxes, c->handle, c);
	if (c->editable)
		[c->cb setDelegate:self];
	else {
		[c->pb setTarget:self];
		[c->pb setAction:@selector(onSelected:)];
	}
}

- (void)unregisterCombobox:(uiCombobox *)c
{
	if (c->editable)
		[c->cb setDelegate:nil];
	else
		[c->pb setTarget:nil];
	mapDelete(self->comboboxes, c->handle);
}

@end

static comboboxDelegateClass *comboboxDelegate = nil;

static void onDestroy(uiCombobox *);

uiDarwinDefineControlWithOnDestroy(
	uiCombobox,							// type name
	handle,								// handle
	onDestroy(this);						// on destroy
)

static void onDestroy(uiCombobox *c)
{
	[comboboxDelegate unregisterCombobox:c];
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

intmax_t uiComboboxSelected(uiCombobox *c)
{
	if (c->editable)
		return [c->cb indexOfSelectedItem];
	return [c->pb indexOfSelectedItem];
}

void uiComboboxSetSelected(uiCombobox *c, intmax_t n)
{
	if (c->editable) {
		// see https://developer.apple.com/library/mac/documentation/Cocoa/Conceptual/ComboBox/Tasks/SettingComboBoxValue.html#//apple_ref/doc/uid/20000256
		id delegate;

		// this triggers the delegate; turn it off for now
		delegate = [c->cb delegate];
		[c->cb setDelegate:nil];

		// this seems to work fine for -1 too
		[c->cb selectItemAtIndex:n];
		if (n == -1)
			[c->cb setObjectValue:@""];
		else
			[c->cb setObjectValue:[c->cb objectValueOfSelectedItem]];

		[c->cb setDelegate:delegate];
		return;
	}
	[c->pb selectItemAtIndex:n];
}

void uiComboboxOnSelected(uiCombobox *c, void (*f)(uiCombobox *c, void *data), void *data)
{
	c->onSelected = f;
	c->onSelectedData = data;
}

static void defaultOnSelected(uiCombobox *c, void *data)
{
	// do nothing
}

static uiCombobox *finishNewCombobox(BOOL editable)
{
	uiCombobox *c;

	c = (uiCombobox *) uiNewControl(uiCombobox);

	c->editable = editable;
	if (c->editable) {
		c->cb = [[libui_intrinsicWidthNSComboBox alloc] initWithFrame:NSZeroRect];
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

	if (comboboxDelegate == nil) {
		comboboxDelegate = [comboboxDelegateClass new];
		[delegates addObject:comboboxDelegate];
	}
	[comboboxDelegate registerCombobox:c];
	uiComboboxOnSelected(c, defaultOnSelected, NULL);

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
