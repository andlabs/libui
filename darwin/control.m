// 16 august 2015
#import "uipriv_darwin.h"

void uiDarwinControlTriggerRelayout(uiDarwinControl *c)
{
	uiControl *p;

	p = toplevelOwning(uiControl(c));
	if (p == NULL)		// not in a window
		return;
	c = uiDarwinControl(p);
	(*(c->Relayout))(uiDarwinControl(c));
}

static void defaultCommitShow(uiControl *c)
{
	NSView *view;

	view = (NSView *) uiControlHandle(c);
	[view setHidden:NO];
}

static void defaultCommitHide(uiControl *c)
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
	if (!isToplevel(c))
		[view setTranslatesAutoresizingMaskIntoConstraints:NO];
	c->CommitShow = defaultCommitShow;
	c->CommitHide = defaultCommitHide;
}

void uiDarwinSetControlFont(NSControl *c, NSControlSize size)
{
	[c setFont:[NSFont systemFontOfSize:[NSFont systemFontSizeForControlSize:size]]];
}

#define uiDarwinControlSignature 0x44617277

uiDarwinControl *uiDarwinNewControl(size_t n, uint32_t typesig, const char *typenamestr)
{
	return uiDarwinControl(newControl(n, uiDarwinControlSignature, typesig, typenamestr));
}
