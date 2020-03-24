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

// TODO does this have one on its own?
@interface libui_intrinsicWidthNSSecureTextField : NSSecureTextField
@end

@implementation libui_intrinsicWidthNSSecureTextField

- (NSSize)intrinsicContentSize
{
	NSSize s;

	s = [super intrinsicContentSize];
	s.width = textfieldWidth;
	return s;
}

@end

// TODO does this have one on its own?
@interface libui_intrinsicWidthNSSearchField : NSSearchField
@end

@implementation libui_intrinsicWidthNSSearchField

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
	int (*onKeyEvent)(uiEntry *, uiAreaKeyEvent *);
};

static BOOL isSearchField(NSTextField *tf)
{
	return [tf isKindOfClass:[NSSearchField class]];
}

static void triggerOnKeyEvent(void *key, void *e, void *data)
{
	uiEntry *entry = (uiEntry *)e;
	void *firstResponder = [entry->textfield window].firstResponder;
	BOOL sameObj = (entry->textfield == firstResponder);
	BOOL currentEditor = (entry->textfield.currentEditor && entry->textfield.currentEditor == firstResponder);
	if (sameObj || currentEditor)
		entry->onKeyEvent(entry, data);
}

@interface entryDelegateClass : NSObject<NSTextFieldDelegate> {
	uiprivMap *entries;
	id eventMonitor;
}
- (void)controlTextDidChange:(NSNotification *)note;
- (IBAction)onSearch:(id)sender;
- (void)registerEntry:(uiEntry *)e;
- (void)unregisterEntry:(uiEntry *)e;
@end

@implementation entryDelegateClass

- (id)init
{
	self = [super init];
	if (self)
		self->entries = uiprivNewMap();

	NSEvent* (^eventHandler)(NSEvent*) = ^(NSEvent *theEvent) {
		uiAreaKeyEvent ke;
		ke.Key = 0;
		ke.ExtKey = 0;
		ke.Modifier = 0;
		ke.Modifiers = parseModifiers(theEvent);
		ke.Up = ([theEvent type] == NSKeyUp ? 1 : 0);

		if (uiprivFromKeycode([theEvent keyCode], &ke))
			uiprivMapWalkWithData(self->entries, &ke, triggerOnKeyEvent);

		return theEvent;
	};
	eventMonitor = [NSEvent addLocalMonitorForEventsMatchingMask:(NSKeyDownMask | NSKeyUpMask) handler:eventHandler];

	return self;
}

- (void)dealloc
{
	[NSEvent removeMonitor:eventMonitor];
	uiprivMapDestroy(self->entries);
	[super dealloc];
}

- (void)controlTextDidChange:(NSNotification *)note
{
	[self onSearch:[note object]];
}

- (IBAction)onSearch:(id)sender
{
	uiEntry *e;

	e = (uiEntry *) uiprivMapGet(self->entries, sender);
	(*(e->onChanged))(e, e->onChangedData);
}

- (void)registerEntry:(uiEntry *)e
{
	uiprivMapSet(self->entries, e->textfield, e);
	if (isSearchField(e->textfield)) {
		[e->textfield setTarget:self];
		[e->textfield setAction:@selector(onSearch:)];
	} else
		[e->textfield setDelegate:self];
}

- (void)unregisterEntry:(uiEntry *)e
{
	if (isSearchField(e->textfield))
		[e->textfield setTarget:nil];
	else
		[e->textfield setDelegate:nil];
	uiprivMapDelete(self->entries, e->textfield);
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
	[e->textfield setStringValue:uiprivToNSString(text)];
	// don't queue the control for resize; entry sizes are independent of their contents
}

void uiEntryOnChanged(uiEntry *e, void (*f)(uiEntry *, void *), void *data)
{
	e->onChanged = f;
	e->onChangedData = data;
}

void uiEntryOnKeyEvent(uiEntry *e, int (*f)(uiEntry *, uiAreaKeyEvent *))
{
	e->onKeyEvent = f;
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

static int defaultOnKeyEvent(uiEntry *e, uiAreaKeyEvent *ke)
{
	// do nothing
	return FALSE;
}

// these are based on interface builder defaults; my comments in the old code weren't very good so I don't really know what talked about what, sorry :/
void uiprivFinishNewTextField(NSTextField *t, BOOL isEntry)
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

static NSTextField *realNewEditableTextField(Class class)
{
	NSTextField *tf;

	tf = [[class alloc] initWithFrame:NSZeroRect];
	[tf setSelectable:YES];		// otherwise the setting is masked by the editable default of YES
	uiprivFinishNewTextField(tf, YES);
	return tf;
}

NSTextField *uiprivNewEditableTextField(void)
{
	return realNewEditableTextField([libui_intrinsicWidthNSTextField class]);
}

static uiEntry *finishNewEntry(Class class)
{
	uiEntry *e;

	uiDarwinNewControl(uiEntry, e);

	e->textfield = realNewEditableTextField(class);

	if (entryDelegate == nil) {
		entryDelegate = [[entryDelegateClass new] autorelease];
		[uiprivDelegates addObject:entryDelegate];
	}
	[entryDelegate registerEntry:e];
	uiEntryOnChanged(e, defaultOnChanged, NULL);
	uiEntryOnKeyEvent(e, defaultOnKeyEvent);

	return e;
}

uiEntry *uiNewEntry(void)
{
	return finishNewEntry([libui_intrinsicWidthNSTextField class]);
}

uiEntry *uiNewPasswordEntry(void)
{
	return finishNewEntry([libui_intrinsicWidthNSSecureTextField class]);
}

uiEntry *uiNewSearchEntry(void)
{
	uiEntry *e;
	NSSearchField *s;

	e = finishNewEntry([libui_intrinsicWidthNSSearchField class]);
	s = (NSSearchField *) (e->textfield);
	// TODO these are only on 10.10
//	[s setSendsSearchStringImmediately:NO];
//	[s setSendsWholeSearchString:NO];
	[s setBordered:NO];
	[s setBezelStyle:NSTextFieldRoundedBezel];
	[s setBezeled:YES];
	return e;
}
