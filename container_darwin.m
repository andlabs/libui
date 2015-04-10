// 4 august 2014
#import "uipriv_darwin.h"

// calling -[className] on the content views of NSWindow, NSTabItem, and NSBox all return NSView, so I'm assuming I just need to override these
// fornunately:
// - NSWindow resizing calls -[setFrameSize:] (but not -[setFrame:])
// - NSTab resizing calls both -[setFrame:] and -[setFrameSIze:] on the current tab
// - NSTab switching tabs calls both -[setFrame:] and -[setFrameSize:] on the new tab
// so we just override setFrameSize:
// thanks to mikeash and JtRip in irc.freenode.net/#macdev
@implementation uiContainer {
	BOOL uimargined;
}

uiLogObjCClassAllocations

- (void)viewDidMoveToSuperview
{
	if ([self superview] == nil)
		if (self.uiChild != NULL) {
			uiControlDestroy(self.uiChild);
			self.uiChild = NULL;
		}
	[super viewDidMoveToSuperview];
}

- (void)setFrameSize:(NSSize)s
{
	[super setFrameSize:s];
	[self uiUpdateNow];
}

// These are based on measurements from Interface Builder.
// TODO reverify these against /layout rects/, not /frame rects/
#define macXMargin 20
#define macYMargin 20
#define macXPadding 8
#define macYPadding 8

- (void)uiUpdateNow
{
	uiSizing d;
	intmax_t x, y, width, height;

	if (self.uiChild == NULL)
		return;
	x = [self bounds].origin.x;
	y = [self bounds].origin.y;
	width = [self bounds].size.width;
	height = [self bounds].size.height;
	if (self->uimargined) {
		x += macXMargin;
		y += macYMargin;
		width -= 2 * macXMargin;
		height -= 2 * macYMargin;
	}
	d.xPadding = macXPadding;
	d.yPadding = macYPadding;
	uiControlResize(self.uiChild, x, y, width, height, &d);
}

- (BOOL)uiMargined
{
	return self->uimargined;
}

- (void)uiSetMargined:(BOOL)margined
{
	self->uimargined = margined;
	[self uiUpdateNow];
}

@end

void updateParent(uintptr_t parent)
{
	uiContainer *c;

	if (parent == 0)
		return;
	c = (uiContainer *) parent;
	[c uiUpdateNow];
}
