// 31 july 2015
#import "osxaltest.h"

// leave a whole lot of space around the alignment rect, just to be safe
@interface tSpinboxContainer : NSView
@end

@implementation tSpinboxContainer

- (NSEdgeInsets)alignmentRectInsets
{
	return NSEdgeInsetsMake(50, 50, 50, 50);
}

@end

int nspinbox = 0;

@implementation tSpinbox {
	tSpinboxContainer *c;
	NSTextField *t;
	NSStepper *s;
	id<tControl> parent;
	NSLayoutPriority horzpri, vertpri;
}

- (id)init
{
	self = [super init];
	if (self) {
		NSMutableDictionary *views;
		NSArray *constraints;

		self->c = [[tSpinboxContainer alloc] initWithFrame:NSZeroRect];
		[self->c setTranslatesAutoresizingMaskIntoConstraints:NO];

		self->t = [[NSTextField alloc] initWithFrame:NSZeroRect];
[self->t setStringValue:[NSString stringWithFormat:@"%d", nspinbox]];
nspinbox++;
		[self->t setSelectable:YES];
		[self->t setFont:[NSFont systemFontOfSize:[NSFont systemFontSizeForControlSize:NSRegularControlSize]]];
		[self->t setBordered:NO];
		[self->t setBezelStyle:NSTextFieldSquareBezel];
		[self->t setBezeled:YES];
		[[self->t cell] setLineBreakMode:NSLineBreakByClipping];
		[[self->t cell] setScrollable:YES];
		[self->t setTranslatesAutoresizingMaskIntoConstraints:NO];
		[self->c addSubview:self->t];

		self->s = [[NSStepper alloc] initWithFrame:NSZeroRect];
		[self->s setIncrement:1];
		[self->s setValueWraps:NO];
		[self->s setAutorepeat:YES];
		[self->s setTranslatesAutoresizingMaskIntoConstraints:NO];
		[self->c addSubview:self->s];

		views = [NSMutableDictionary new];
		[views setObject:self->t forKey:@"t"];
		[views setObject:self->s forKey:@"s"];

		constraints = [NSLayoutConstraint constraintsWithVisualFormat:@"H:|[t]-[s]|" options:0 metrics:nil views:views];
		[self->c addConstraints:constraints];
		constraints = [NSLayoutConstraint constraintsWithVisualFormat:@"V:|[t]|" options:0 metrics:nil views:views];
		[self->c addConstraints:constraints];
		constraints = [NSLayoutConstraint constraintsWithVisualFormat:@"V:|[s]|" options:0 metrics:nil views:views];
		[self->c addConstraints:constraints];

		[views release];

		self->parent = nil;

		self->horzpri = [self->c contentHuggingPriorityForOrientation:NSLayoutConstraintOrientationHorizontal];
		self->vertpri = [self->c contentHuggingPriorityForOrientation:NSLayoutConstraintOrientationVertical];
	}
	return self;
}

- (void)tSetParent:(id<tControl>)p addToView:(NSView *)v relayout:(BOOL)relayout
{
	self->parent = p;
	[v addSubview:self->c];
	if (relayout)
		[self tRelayout];
}

- (void)tFillAutoLayout:(tAutoLayoutParams *)p
{
	// reset the hugging priority
	[self->c setContentHuggingPriority:self->horzpri forOrientation:NSLayoutConstraintOrientationHorizontal];
	[self->c setContentHuggingPriority:self->vertpri forOrientation:NSLayoutConstraintOrientationVertical];
	p->view = self->c;
	p->attachLeft = YES;
	p->attachTop = YES;
	p->attachRight = YES;
	p->attachBottom = YES;
	p->nonStretchyWidthPredicate = @"(==96)";		// TODO on the text field only
}

- (void)tRelayout
{
	if (self->parent != nil)
		[self->parent tRelayout];
}

@end
