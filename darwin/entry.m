// 9 april 2015
#import "uipriv_darwin.h"

@interface entryDelegate : NSObject <NSTextFieldDelegate> {
	uiEntry *e;
	void (*onChanged)(uiEntry *, void *);
	void *onChangedData;
}
- (void)controlTextDidChange:(NSNotification *)note;
- (void)setEntry:(uiEntry *)newe;
- (void)setOnChanged:(void (*)(uiEntry *, void *))f data:(void *)data;
@end

@implementation entryDelegate

- (void)controlTextDidChange:(NSNotification *)note
{
	(*(self->onChanged))(self->e, self->onChangedData);
}

- (void)setEntry:(uiEntry *)newe
{
	self->e = newe;
}

- (void)setOnChanged:(void (*)(uiEntry *, void *))f data:(void *)data
{
	self->onChanged = f;
	self->onChangedData = data;
}

@end

struct entry {
	uiEntry e;
	NSTextField *textfield;
	entryDelegate *delegate;
};

static void defaultOnChanged(uiEntry *e, void *data)
{
	// do nothing
}

static void destroy(void *data)
{
	struct entry *e = (struct entry *) data;

	[e->textfield setDelegate:nil];
	[e->delegate release];
	uiFree(e);
}

static char *entryText(uiEntry *ee)
{
	struct entry *e = (struct entry *) ee;

	return uiDarwinNSStringToText([e->textfield stringValue]);
}

static void entrySetText(uiEntry *ee, const char *text)
{
	struct entry *e = (struct entry *) ee;

	[e->textfield setStringValue:toNSString(text)];
}

static void entryOnChanged(uiEntry *ee, void (*f)(uiEntry *, void *), void *data)
{
	struct entry *e = (struct entry *) ee;

	[e->delegate setOnChanged:f data:data];
}

static int entryReadOnly(uiEntry *ee)
{
	struct entry *e = (struct entry *) ee;

	return [e->textfield isEditable] == NO;
}

static void entrySetReadOnly(uiEntry *ee, int readonly)
{
	struct entry *e = (struct entry *) ee;
	BOOL editable;

	editable = YES;
	if (readonly)
		editable = NO;
	[e->textfield setEditable:editable];
}

// these are based on interface builder defaults; my comments in the old code weren't very good so I don't really know what talked about what, sorry :/
void finishNewTextField(NSTextField *t, BOOL isEntry)
{
	setStandardControlFont(t);

	// THE ORDER OF THESE CALLS IS IMPORTANT; CHANGE IT AND THE BORDERS WILL DISAPPEAR
	[t setBordered:NO];
	[t setBezelStyle:NSTextFieldSquareBezel];
	[t setBezeled:isEntry];

	// we don't need to worry about substitutions/autocorrect here; see window_darwin.m for details

	[[t cell] setLineBreakMode:NSLineBreakByClipping];
	[[t cell] setScrollable:YES];
}

uiEntry *uiNewEntry(void)
{
	struct entry *e;

	e = uiNew(struct entry);

	uiDarwinMakeControl(uiControl(e), [NSTextField class], NO, NO, destroy, e);

	e->textfield = (NSTextField *) uiControlHandle(uiControl(e));

	[e->textfield setSelectable:YES];		// otherwise the setting is masked by the editable default of YES
	finishNewTextField(e->textfield, YES);

	e->delegate = [entryDelegate new];
	[e->textfield setDelegate:e->delegate];
	[e->delegate setEntry:uiEntry(e)];
	[e->delegate setOnChanged:defaultOnChanged data:NULL];

	uiEntry(e)->Text = entryText;
	uiEntry(e)->SetText = entrySetText;
	uiEntry(e)->OnChanged = entryOnChanged;
	uiEntry(e)->ReadOnly = entryReadOnly;
	uiEntry(e)->SetReadOnly = entrySetReadOnly;

	return uiEntry(e);
}
