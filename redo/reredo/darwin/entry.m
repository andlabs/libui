// 14 august 2015
#import "uipriv_darwin.h"

// TODO set a text field minimum width

struct uiEntry {
	uiDarwinControl c;
	NSTextField *textfield;
	void (*onChanged)(uiEntry *, void *);
	void *onChangedData;
};

@interface entryDelegateClass : NSObject<NSTextFieldDelegate> {
	NSMapTable *entries;
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

// TODO have this called
- (void)dealloc
{
	if ([self->entries count] != 0)
		complain("attempt to destroy shared entry delegate but entries are still registered to it");
	[self->entries release];
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
	[self->entries removeObjectForKey:e->textfield];
}

@end

static entryDelegateClass *entryDelegate = nil;

uiDarwinDefineControlWithOnDestroy(
	uiEntry,								// type name
	uiEntryType,							// type function
	textfield,								// handle
	[entryDelegate unregisterEntry:this];			// on destroy
)

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

static void defaultOnChanged(uiEntry *e, void *data)
{
	// do nothing
}

uiEntry *uiNewEntry(void)
{
	uiEntry *e;

	e = (uiEntry *) uiNewControl(uiEntryType());

	e->textfield = [[NSTextField alloc] initWithFrame:NSZeroRect];
	[e->textfield setSelectable:YES];		// otherwise the setting is masked by the editable default of YES
	finishNewTextField(e->textfield, YES);

	if (entryDelegate == nil)
		entryDelegate = [entryDelegateClass new];
	[entryDelegate registerEntry:e];
	uiEntryOnChanged(e, defaultOnChanged, NULL);

	uiDarwinFinishNewControl(e, uiEntry);

	return e;
}
