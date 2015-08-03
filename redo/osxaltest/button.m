// 31 july 2015
#import "osxaltest.h"

@implementation tButton {
	NSButton *b;
	id<tControl> parent;
	NSLayoutPriority horzpri, vertpri;
}

- (id)tInitWithText:(NSString *)text
{
	self = [super init];
	if (self) {
		self->b = [[NSButton alloc] initWithFrame:NSZeroRect];
		[self->b setTitle:text];
		[self->b setButtonType:NSMomentaryPushInButton];
		[self->b setBordered:YES];
		[self->b setBezelStyle:NSRoundedBezelStyle];
		[self->b setFont:[NSFont systemFontOfSize:[NSFont systemFontSizeForControlSize:NSRegularControlSize]]];
		[self->b setTranslatesAutoresizingMaskIntoConstraints:NO];

		self->parent = nil;

		self->horzpri = [self->b contentHuggingPriorityForOrientation:NSLayoutConstraintOrientationHorizontal];
		self->vertpri = [self->b contentHuggingPriorityForOrientation:NSLayoutConstraintOrientationVertical];
	}
	return self;
}

- (void)tSetParent:(id<tControl>)p addToView:(NSView *)v relayout:(BOOL)relayout
{
	self->parent = p;
	[v addSubview:self->b];
	if (relayout)
		[self tRelayout];
}

- (void)tFillAutoLayout:(tAutoLayoutParams *)p
{
	// reset the hugging priority
	[self->b setContentHuggingPriority:self->horzpri forOrientation:NSLayoutConstraintOrientationHorizontal];
	[self->b setContentHuggingPriority:self->vertpri forOrientation:NSLayoutConstraintOrientationVertical];
	p->view = self->b;
	p->attachLeft = YES;
	p->attachTop = YES;
	p->attachRight = YES;
	p->attachBottom = YES;
}

- (void)tRelayout
{
	if (self->parent != nil)
		[self->parent tRelayout];
}

@end
