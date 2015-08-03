// 31 july 2015
#import "osxaltest.h"

// leave a whole lot of space around the alignment rect, just to be safe
// TODO fine tune this
// TODO de-duplicate this from spinbox.m
@interface tBoxContainer : NSView
@end

@implementation tBoxContainer

- (NSEdgeInsets)alignmentRectInsets
{
	return NSEdgeInsetsMake(50, 50, 50, 50);
}

@end

@implementation tBox {
	NSView *v;
	NSMutableArray *children;
	NSMutableArray *stretchy;
	BOOL vertical;
	id<tControl> parent;
	BOOL spaced;
}

// TODO rename to padded
- (id)tInitVertical:(BOOL)vert spaced:(BOOL)sp
{
	self = [super init];
	if (self) {
		self->v = [[NSView alloc] initWithFrame:NSZeroRect];
		[self->v setTranslatesAutoresizingMaskIntoConstraints:NO];
		self->children = [NSMutableArray new];
		self->stretchy = [NSMutableArray new];
		self->vertical = vert;
		self->parent = nil;
		self->spaced = sp;
	}
	return self;
}

- (void)tAddControl:(id<tControl>)c stretchy:(BOOL)s
{
	[c tSetParent:self addToView:self->v relayout:NO];
	[self->children addObject:c];
	[self->stretchy addObject:[NSNumber numberWithBool:s]];
	[self tRelayout];
}

- (void)tSetParent:(id<tControl>)p addToView:(NSView *)sv relayout:(BOOL)relayout
{
	self->parent = p;
	[sv addSubview:self->v];
	if (relayout)
		[self tRelayout];
}

// TODO custom minimum width for non-stretchy controls
- (void)tFillAutoLayout:(tAutoLayoutParams *)p
{
	NSMutableDictionary *views;
	__block uintmax_t i, n;
	__block tAutoLayoutParams pp;
	NSMutableString *constraint;
	BOOL firstStretchy;
	uintmax_t nStretchy;
	NSLayoutConstraintOrientation orientation;

	[self->v removeConstraints:[self->v constraints]];

	orientation = NSLayoutConstraintOrientationHorizontal;
	if (self->vertical)
		orientation = NSLayoutConstraintOrientationVertical;

	views = [NSMutableDictionary new];
	n = 0;
	[self->children enumerateObjectsUsingBlock:^(id obj, NSUInteger index, BOOL *stop) {
		id<tControl> c;
		NSNumber *isStretchy;
		NSLayoutPriority priority;

		c = (id<tControl>) obj;
		isStretchy = (NSNumber *) [self->stretchy objectAtIndex:n];
		// this also resets the hugging priority
		// TODO do this when adding and removing controls instead
		[c tFillAutoLayout:&pp];
		priority = NSLayoutPriorityDefaultHigh;			// forcibly hug; avoid stretching out
		if ([isStretchy boolValue])
			priority = NSLayoutPriorityDefaultLow;		// do not forcibly hug; freely stretch out
		[pp.view setContentHuggingPriority:priority forOrientation:orientation];
		[views setObject:pp.view forKey:tAutoLayoutKey(n)];
		n++;
	}];

	// first string the views together
	if (self->vertical)
		constraint = [NSMutableString stringWithString:@"V:|"];
	else
		constraint = [NSMutableString stringWithString:@"H:|"];
	firstStretchy = YES;
	for (i = 0; i < n; i++) {
		NSNumber *isStretchy;

		if (self->spaced && i != 0)
			[constraint appendString:@"-"];
		[constraint appendString:@"["];
		[constraint appendString:tAutoLayoutKey(i)];
		isStretchy = (NSNumber *) [self->stretchy objectAtIndex:i];
		if ([isStretchy boolValue])
			if (firstStretchy) {
				firstStretchy = NO;
				nStretchy = i;
			} else {
				[constraint appendString:@"(=="];
				[constraint appendString:tAutoLayoutKey(nStretchy)];
				[constraint appendString:@")"];
			}
		[constraint appendString:@"]"];
	}
	[constraint appendString:@"|"];
	[self->v addConstraints:[NSLayoutConstraint constraintsWithVisualFormat:constraint options:0 metrics:nil views:views]];
	// TODO do not release constraint; it's autoreleased?

	// next make the views span the full other dimension
	for (i = 0; i < n; i++) {
		if (self->vertical)
			constraint = [NSMutableString stringWithString:@"H:|["];
		else
			constraint = [NSMutableString stringWithString:@"V:|["];
		[constraint appendString:tAutoLayoutKey(i)];
		[constraint appendString:@"]|"];
		[self->v addConstraints:[NSLayoutConstraint constraintsWithVisualFormat:constraint options:0 metrics:nil views:views]];
		// TODO do not release constraint; it's autoreleased?
	}

	[views release];

	// and now populate for self
	p->view = self->v;
	p->attachLeft = YES;
	p->attachTop = YES;
	// don't attach to the end if there weren't any stretchy controls
	// firstStretchy is NO if there was at least one stretchy control
	if (self->vertical) {
		p->attachRight = YES;
		p->attachBottom = !firstStretchy;
	} else {
		p->attachRight = !firstStretchy;
		p->attachBottom = YES;
	}
}

- (void)tRelayout
{
	if (self->parent != nil)
		[self->parent tRelayout];
}

@end
