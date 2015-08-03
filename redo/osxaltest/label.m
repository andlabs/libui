// 31 july 2015
#import "osxaltest.h"

@implementation tLabel {
	NSTextField *t;
	id<tControl> parent;
	NSLayoutPriority horzpri, vertpri;
}

- (id)init
{
	self = [super init];
	if (self) {
		self->t = [[NSTextField alloc] initWithFrame:NSZeroRect];
		[self->t setStringValue:@"Label"];
		[self->t setEditable:NO];
		[self->t setSelectable:NO];
		[self->t setDrawsBackground:NO];
		[self->t setFont:[NSFont systemFontOfSize:[NSFont systemFontSizeForControlSize:NSRegularControlSize]]];
		[self->t setBordered:NO];
		[self->t setBezelStyle:NSTextFieldSquareBezel];
		[self->t setBezeled:NO];
		[[self->t cell] setLineBreakMode:NSLineBreakByClipping];
		[[self->t cell] setScrollable:YES];
		[self->t setTranslatesAutoresizingMaskIntoConstraints:NO];

		self->parent = nil;

		self->horzpri = [self->t contentHuggingPriorityForOrientation:NSLayoutConstraintOrientationHorizontal];
		self->vertpri = [self->t contentHuggingPriorityForOrientation:NSLayoutConstraintOrientationVertical];
	}
	return self;
}

- (void)tSetParent:(id<tControl>)p addToView:(NSView *)v relayout:(BOOL)relayout
{
	self->parent = p;
	[v addSubview:self->t];
	if (relayout)
		[self tRelayout];
}

- (void)tFillAutoLayout:(tAutoLayoutParams *)p
{
	// reset the hugging priority
	[self->t setContentHuggingPriority:self->horzpri forOrientation:NSLayoutConstraintOrientationHorizontal];
	[self->t setContentHuggingPriority:self->vertpri forOrientation:NSLayoutConstraintOrientationVertical];
	p->view = self->t;
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
