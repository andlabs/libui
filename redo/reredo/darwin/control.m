// 16 august 2015
#import "uipriv_darwin.h"

static uintmax_t type_uiDarwinControl = 0;

uintmax_t uiDarwinControlType(void)
{
	if (type_uiDarwinControl == 0)
		type_uiDarwinControl = uiRegisterType("uiDarwinControl", uiControlType(), sizeof (uiDarwinControl));
	return type_uiDarwinControl;
}

void uiDarwinControlRelayoutParent(uiDarwinControl *c)
{
	uiControl *p;

	p = uiControlParent(uiControl(c));
	if (p == NULL)
		return;
	(*(uiDarwinControl(p)->Relayout))(p);
}

void osCommitShow(uiControl *c)
{
	NSView *view;

	view = (NSView *) uiControlHandle(c);
	// TODO
	if ([view isKindOfClass:[NSWindow class]]) {
		[view makeKeyAndOrderFront:view];
		return;
	}
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
	// TODO omit this for uiWindow properly
	if ([view respondsToSelector:@selector(setTranslatesAutoresizingMaskIntoConstraints:)])
		[view setTranslatesAutoresizingMaskIntoConstraints:NO];
}

void uiDarwinSetControlFont(NSControl *c, NSControlSize size)
{
	[c setFont:[NSFont systemFontOfSize:[NSFont systemFontSizeForControlSize:size]]];
}
