// 16 august 2015
#import "uipriv_darwin.h"

// TODO refine this
void uiDarwinControlTriggerRelayout(uiDarwinControl *c)
{
	NSView *view;
	uiWindow *p;

	view = (NSView *) uiControlHandle(uiControl(c));
	// this can be a NSWindow
	if (![view isKindOfClass:[NSWindow class]]) {
		p = windowFromNSWindow([view window]);
		if (p == NULL)		// not in a window
			return;
	} else
		p = uiWindow(c);
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
