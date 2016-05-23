// 14 august 2015
#import "uipriv_darwin.h"

// NSComboBoxes have no intrinsic width; we'll use the default Interface Builder width for them.
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

struct uiEditableCombobox {
	uiDarwinControl c;
	NSComboBox *cb;
	void (*onChanged)(uiEditableCombobox *, void *);
	void *onChangedData;
};

@interface editableComboboxDelegateClass : NSObject<NSComboBoxDelegate> {
	struct mapTable *comboboxes;
}
- (void)controlTextDidChange:(NSNotification *)note;
- (void)comboBoxSelectionDidChange:(NSNotification *)note;
- (void)registerCombobox:(uiEditableCombobox *)c;
- (void)unregisterCombobox:(uiEditableCombobox *)c;
@end

@implementation editableComboboxDelegateClass

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

- (void)controlTextDidChange:(NSNotification *)note
{
	uiEditableCombobox *c;

	c = uiEditableCombobox(mapGet(self->comboboxes, [note object]));
	(*(c->onChanged))(c, c->onChangedData);
}

// the above doesn't handle when an item is selected; this will
- (void)comboBoxSelectionDidChange:(NSNotification *)note
{
	// except this is sent BEFORE the entry is changed, and that doesn't send the above, so
	// this is via http://stackoverflow.com/a/21059819/3408572 - it avoids the need to manage selected items
	// this still isn't perfect — I get residual changes to the same value while navigating the list — but it's good enough
	[self performSelector:@selector(controlTextDidChange:)
		withObject:note
		afterDelay:0];
}

- (void)registerCombobox:(uiEditableCombobox *)c
{
	mapSet(self->comboboxes, c->cb, c);
	[c->cb setDelegate:self];
}

- (void)unregisterCombobox:(uiEditableCombobox *)c
{
	[c->cb setDelegate:nil];
	mapDelete(self->comboboxes, c->cb);
}

@end

static editableComboboxDelegateClass *comboboxDelegate = nil;

uiDarwinControlAllDefaultsExceptDestroy(uiEditableCombobox, cb)

static void uiEditableComboboxDestroy(uiControl *cc)
{
	uiEditableCombobox *c = uiEditableCombobox(cc);

	[comboboxDelegate unregisterCombobox:c];
	[c->cb release];
	uiFreeControl(uiControl(c));
}

void uiEditableComboboxAppend(uiEditableCombobox *c, const char *text)
{
	[c->cb addItemWithObjectValue:toNSString(text)];
}

char *uiEditableComboboxText(uiEditableCombobox *c)
{
	return uiDarwinNSStringToText([c->cb stringValue]);
}

void uiEditableComboboxSetText(uiEditableCombobox *c, const char *text)
{
	NSString *t;

	t = toNSString(text);
	[c->cb setStringValue:t];
	// do this just to keep synchronicity with the behavior of the real control
	// for what it's worth, this automatic behavior when typing is the reason why this is separate from uiCombobox
	[c->cb selectItemWithObjectValue:t];
}

#if 0
// TODO
void uiEditableComboboxSetSelected(uiEditableCombobox *c, intmax_t n)
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
#endif

void uiEditableComboboxOnChanged(uiEditableCombobox *c, void (*f)(uiEditableCombobox *c, void *data), void *data)
{
	c->onChanged = f;
	c->onChangedData = data;
}

static void defaultOnChanged(uiEditableCombobox *c, void *data)
{
	// do nothing
}

uiEditableCombobox *uiNewEditableCombobox(void)
{
	uiEditableCombobox *c;

	uiDarwinNewControl(uiEditableCombobox, c);

	c->cb = [[libui_intrinsicWidthNSComboBox alloc] initWithFrame:NSZeroRect];
	[c->cb setUsesDataSource:NO];
	[c->cb setButtonBordered:YES];
	[c->cb setCompletes:NO];
	uiDarwinSetControlFont(c->cb, NSRegularControlSize);

	if (comboboxDelegate == nil) {
		comboboxDelegate = [editableComboboxDelegateClass new];
		[delegates addObject:comboboxDelegate];
	}
	[comboboxDelegate registerCombobox:c];
	uiEditableComboboxOnChanged(c, defaultOnChanged, NULL);

	return c;
}
