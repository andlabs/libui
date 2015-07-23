// 11 june 2015
#include "uipriv_darwin.h"

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

uiDefineControlType(uiEntry, uiTypeEntry, struct entry)

static uintptr_t entryHandle(uiControl *c)
{
	struct entry *e = (struct entry *) c;

	return (uintptr_t) (e->textfield);
}

static void defaultOnChanged(uiEntry *e, void *data)
{
	// do nothing
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
	// don't queue the control for resize; entry sizes are independent of their contents
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
void finishNewTextField(uiControl *tt, NSTextField *t, BOOL isEntry)
{
	uiDarwinMakeSingleViewControl(tt, t, YES);

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

	e = (struct entry *) uiNewControl(uiTypeEntry());

	e->textfield = [[NSTextField alloc] initWithFrame:NSZeroRect];

	[e->textfield setSelectable:YES];		// otherwise the setting is masked by the editable default of YES
	finishNewTextField(uiControl(e), e->textfield, YES);

	e->delegate = [entryDelegate new];
	[e->textfield setDelegate:e->delegate];
	[e->delegate setEntry:uiEntry(e)];
	[e->delegate setOnChanged:defaultOnChanged data:NULL];

	e->onChanged = defaultOnChanged;

	uiControl(e)->Handle = entryHandle;

	uiEntry(e)->Text = entryText;
	uiEntry(e)->SetText = entrySetText;
	uiEntry(e)->OnChanged = entryOnChanged;
	uiEntry(e)->ReadOnly = entryReadOnly;
	uiEntry(e)->SetReadOnly = entrySetReadOnly;

	return uiEntry(e);
}
