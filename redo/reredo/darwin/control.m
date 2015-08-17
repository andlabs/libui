// 16 august 2015
#import "uipriv_darwin.h"

static uintmax_t type_uiDarwinControl = 0;

uintmax_t uiDarwinControlType(void)
{
	if (type_uiDarwinControl == 0)
		type_uiDarwinControl = uiRegisterType("uiDarwinControl", uiControlType(), sizeof (uiDarwinControl));
	return type_uiDarwinControl;
}

void osCommitShow(uiControl *c)
{
	NSView *view;

	view = (NSView *) uiControlHandle(c);
	[view setHidden:NO];
}

void osCommitHide(uiControl *c)
{
	NSView *view;

	view = (NSView *) uiControlHandle(c);
	[view setHidden:YES];
}

void osCommitEnable(uiControl *c)
{
	NSControl *view;

	view = (NSControl *) uiControlHandle(c);
	if ([view respondsToSelector:@selector(setEnabled:)])
		[view setEnabled:YES];
}

void osCommitDisable(uiControl *c)
{
	NSControl *view;

	view = (NSControl *) uiControlHandle(c);
	if ([view respondsToSelector:@selector(setEnabled:)])
		[view setEnabled:NO];
}

void uiDarwinFinishControl(uiControl *c)
{
	NSView *view;

	view = (NSView *) uiControlHandle(c);
	[view retain];
}

void uiDarwinSetControlFont(NSControl *c, NSControlSize size)
{
	[c setFont:[NSFont systemFontOfSize:[NSFont systemFontSizeForControlSize:size]]];
}
