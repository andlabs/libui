// 4 august 2014
#import "uipriv_darwin.h"

// calling -[className] on the content views of NSWindow, NSTabItem, and NSBox all return NSView, so I'm assuming I just need to override these
// fornunately:
// - NSWindow resizing calls -[setFrameSize:] (but not -[setFrame:])
// - NSTabView resizing calls both -[setFrame:] and -[setFrameSIze:] on the current tab
// - NSTabView switching tabs calls both -[setFrame:] and -[setFrameSize:] on the new tab
// so we just override setFrameSize:
// thanks to mikeash and JtRip in irc.freenode.net/#macdev
@interface uipParent : NSView {
// TODO
@public
	uiControl *mainControl;
	intmax_t marginLeft;
	intmax_t marginTop;
	intmax_t marginRight;
	intmax_t marginBottom;
}
- (void)uiUpdateNow;
@end

@implementation uipParent

uiLogObjCClassAllocations

- (void)viewDidMoveToSuperview
{
	// we can't just use nil because NSTabView will set page views to nil when they're tabbed away
	// this means that we have to explicitly move them to the destroyed controls view when we're done with them, and likewise in NSWindow
	if ([self superview] == destroyedControlsView)
		if (self->mainControl != NULL) {
			uiControlDestroy(self->mainControl);
			self->mainControl = NULL;
			[self release];
		}
	[super viewDidMoveToSuperview];
}

- (void)setFrameSize:(NSSize)s
{
	[super setFrameSize:s];
	[self uiUpdateNow];
}

// These are based on measurements from Interface Builder.
// These seem to be based on Auto Layout constants, but I don't see an API that exposes these...
// This one is 8 for most pairs of controls that I've tried; the only difference is between two pushbuttons, where it's 12...
#define macXPadding 8
// Likewise, this one appears to be 12 for pairs of push buttons...
#define macYPadding 8

- (void)uiUpdateNow
{
	uiSizing d;
	intmax_t x, y, width, height;

	if (self->mainControl == NULL)
		return;
	x = [self bounds].origin.x + self->marginLeft;
	y = [self bounds].origin.y + self->marginTop;
	width = [self bounds].size.width - (self->marginLeft + self->marginRight);
	height = [self bounds].size.height - (self->marginTop + self->marginBottom);
	d.xPadding = macXPadding;
	d.yPadding = macYPadding;
	uiControlResize(self->mainControl, x, y, width, height, &d);
}

@end

static uintptr_t parentHandle(uiParent *p)
{
	uipParent *pp = (uipParent *) (p->Internal);

	return (uintptr_t) pp;
}

static void parentSetMainControl(uiParent *pp, uiControl *mainControl)
{
	uipParent *p = (uipParent *) (pp->Internal);

	if (p->mainControl != NULL)
		uiControlSetParent(p->mainControl, NULL);
	p->mainControl = mainControl;
	if (p->mainControl != NULL)
		uiControlSetParent(p->mainControl, pp);
}

static void parentSetMargins(uiParent *p, intmax_t left, intmax_t top, intmax_t right, intmax_t bottom)
{
	uipParent *pp = (uipParent *) (p->Internal);

	pp->marginLeft = left;
	pp->marginTop = top;
	pp->marginRight = right;
	pp->marginBottom = bottom;
}

static void parentUpdate(uiParent *p)
{
	uipParent *pp = (uipParent *) (p->Internal);

	[pp uiUpdateNow];
}

uiParent *uiNewParent(uintptr_t osParent)
{
	uiParent *p;

	p = uiNew(uiParent);
	p->Internal = [[uipParent alloc] initWithFrame:NSZeroRect];
	p->Handle = parentHandle;
	p->SetMainControl = parentSetMainControl;
	p->SetMargins = parentSetMargins;
	p->Update = parentUpdate;
	// don't use osParent; we'll need to call specific selectors to set the parent view
	// and keep the view alive so we can release it properly later
	[((uipParent *) (p->Internal)) retain];
	return p;
}