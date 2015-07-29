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
	uiDarwinMakeSingleViewControl(c, view, NO);
	[view setContainer:c];
	return (uintptr_t) view;
}
