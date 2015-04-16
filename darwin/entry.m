// 9 april 2015
#import "uipriv_darwin.h"

@interface uiNSTextField : NSTextField
@property uiEntry *uiE;
@end

@implementation uiNSTextField

- (void)viewDidMoveToSuperview
{
	if (uiDarwinControlFreeWhenAppropriate(uiControl(self.uiE), [self superview])) {
		[self setTarget:nil];
		self.uiE = NULL;
	}
	[super viewDidMoveToSuperview];
}

@end

static char *entryText(uiEntry *e)
{
	uiNSTextField *t;

	t = (uiNSTextField *) uiControlHandle(uiControl(e));
	return uiDarwinNSStringToText([t stringValue]);
}

static void entrySetText(uiEntry *e, const char *text)
{
	uiNSTextField *t;

	t = (uiNSTextField *) uiControlHandle(uiControl(e));
	[t setStringValue:toNSString(text)];
}

// TOOD move elsewhere
// these are based on interface builder defaults; my comments in the old code weren't very good so I don't really know what talked about what, sorry :/
void finishNewTextField(NSTextField *t, BOOL isEntry)
{
	setStandardControlFont((id) t);

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
	uiEntry *e;
	uiNSTextField *t;

	e = uiNew(uiEntry);

	uiDarwinNewControl(uiControl(e), [uiNSTextField class], NO, NO);
	t = (uiNSTextField *) uiControlHandle(uiControl(e));

	[t setSelectable:YES];		// otherwise the setting is masked by the editable default of YES
	finishNewTextField((NSTextField *) t, YES);

	uiEntry(e)->Text = entryText;
	uiEntry(e)->SetText = entrySetText;

	t.uiE = e;

	return t.uiE;
}
