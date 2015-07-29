// 28 april 2015
#import "uipriv_darwin.h"

@interface containerView : NSView {
	uiControl *c;
}
- (void)setContainer:(uiControl *)cc;
- (void)containerUpdate;
@end

@implementation containerView

- (void)setContainer:(uiControl *)cc
{
	self->c = cc;
}

// These are based on measurements from Interface Builder.
// These seem to be based on Auto Layout constants, but I don't see an API that exposes these...
// This one is 8 for most pairs of controls that I've tried; the only difference is between two pushbuttons, where it's 12...
#define macXPadding 8
// Likewise, this one appears to be 12 for pairs of push buttons...
#define macYPadding 8

- (void)containerUpdate
{
	uiSizing *d;
	intmax_t x, y, width, height;

	x = [self bounds].origin.x;
	y = [self bounds].origin.y;
	width = [self bounds].size.width;
	height = [self bounds].size.height;
	d = uiDarwinNewSizing();
	uiControlResize(self->c, x, y, width, height, d);
}

- (void)setFrameSize:(NSSize)s
{
	[super setFrameSize:s];
	[self containerUpdate];
}

@end

uintptr_t uiMakeContainer(uiControl *c)
{
	containerView *view;

	view = [[containerView alloc] initWithFrame:NSZeroRect];
	uiDarwinMakeSingleWidgetControl(c, view, NO);
	[view setContainer:c];
	return (uintptr_t) widget;
}
