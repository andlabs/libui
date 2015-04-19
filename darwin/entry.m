// 9 april 2015
#import "uipriv_darwin.h"

struct entry {
	uiEntry e;
	NSTextField *textfield;
};

static void destroy(void *data)
{
	struct entry *e = (struct entry *) data;

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

	uiDarwinNewControl(uiControl(e), [NSTextField class], NO, NO, destroy, e);

	e->textfield = (NSTextField *) VIEW(e);

	[e->textfield setSelectable:YES];		// otherwise the setting is masked by the editable default of YES
	finishNewTextField(e->textfield, YES);

	uiEntry(e)->Text = entryText;
	uiEntry(e)->SetText = entrySetText;

	return uiEntry(e);
}
