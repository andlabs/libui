// 14 august 2015
#import "uipriv_darwin.h"

// Text fields for entering text have no intrinsic width; we'll use the default Interface Builder width for them.
#define textfieldWidth 96

@interface libui_intrinsicWidthNSTextField : NSTextField
@end

@implementation libui_intrinsicWidthNSTextField

- (NSSize)intrinsicContentSize
{
	NSSize s;

	s = [super intrinsicContentSize];
	s.width = textfieldWidth;
	return s;
}

@end

struct uiEntry {
	uiDarwinControl c;
	NSTextField *textfield;
	void (*onChanged)(uiEntry *, void *);
	void *onChangedData;
};

@interface entryDelegateClass : NSObject<NSTextFieldDelegate> {
	struct mapTable *entries;
}
- (void)controlTextDidChange:(NSNotification *)note;
- (void)registerEntry:(uiEntry *)e;
- (void)unregisterEntry:(uiEntry *)e;
@end

@implementation entryDelegateClass

- (id)init
{
	self = [super init];
	if (self)
		self->entries = newMap();
	return self;
}

- (void)dealloc
{
	mapDestroy(self->entries);
	[super dealloc];
}

- (void)controlTextDidChange:(NSNotification *)note
{
	uiEntry *e;

	e = (uiEntry *) mapGet(self->entries, [note object]);
	(*(e->onChanged))(e, e->onChangedData);
}

- (void)registerEntry:(uiEntry *)e
{
	mapSet(self->entries, e->textfield, e);
	[e->textfield setDelegate:self];
}

- (void)unregisterEntry:(uiEntry *)e
{
	[e->textfield setDelegate:nil];
	mapDelete(self->entries, e->textfield);
}

@end

static entryDelegateClass *entryDelegate = nil;

uiDarwinControlAllDefaultsExceptDestroy(uiEntry, textfield)

static void uiEntryDestroy(uiControl *c)
{
	uiEntry *e = uiEntry(c);

	[entryDelegate unregisterEntry:e];
	[e->textfield release];
	uiFreeControl(uiControl(e));
}

char *uiEntryText(uiEntry *e)
{
	return uiDarwinNSStringToText([e->textfield stringValue]);
}

void uiEntrySetText(uiEntry *e, const char *text)
{
	[e->textfield setStringValue:toNSString(text)];
	// don't queue the control for resize; entry sizes are independent of their contents
}

void uiEntryOnChanged(uiEntry *e, void (*f)(uiEntry *, void *), void *data)
{
	e->onChanged = f;
	e->onChangedData = data;
}

int uiEntryReadOnly(uiEntry *e)
{
	return [e->textfield isEditable] == NO;
}

void uiEntrySetReadOnly(uiEntry *e, int readonly)
{
	BOOL editable;

	editable = YES;
	if (readonly)
		editable = NO;
	[e->textfield setEditable:editable];
}

static void defaultOnChanged(uiEntry *e, void *data)
{
	// do nothing
}

// these are based on interface builder defaults; my comments in the old code weren't very good so I don't really know what talked about what, sorry :/
void finishNewTextField(NSTextField *t, BOOL isEntry)
{
	uiDarwinSetControlFont(t, NSRegularControlSize);

	// THE ORDER OF THESE CALLS IS IMPORTANT; CHANGE IT AND THE BORDERS WILL DISAPPEAR
	[t setBordered:NO];
	[t setBezelStyle:NSTextFieldSquareBezel];
	[t setBezeled:isEntry];

	// we don't need to worry about substitutions/autocorrect here; see window_darwin.m for details

	[[t cell] setLineBreakMode:NSLineBreakByClipping];
	[[t cell] setScrollable:YES];
}

NSTextField *newEditableTextField(void)
{
	NSTextField *tf;

	tf = [[libui_intrinsicWidthNSTextField alloc] initWithFrame:NSZeroRect];
	[tf setSelectable:YES];		// otherwise the setting is masked by the editable default of YES
	finishNewTextField(tf, YES);
	return tf;
}

uiEntry *uiNewEntry(void)
{
	uiEntry *e;

	uiDarwinNewControl(uiEntry, e);

	e->textfield = newEditableTextField();

	if (entryDelegate == nil) {
		entryDelegate = [[entryDelegateClass new] autorelease];
		[delegates addObject:entryDelegate];
	}
	[entryDelegate registerEntry:e];
	uiEntryOnChanged(e, defaultOnChanged, NULL);

	return e;
}
