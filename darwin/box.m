// 15 august 2015
#import "uipriv_darwin.h"

// TODOs:
// - tab on page 2 is glitched initially and doesn't grow
// - page 3 doesn't work right; probably due to our shouldExpand logic being applied incorrectly

// TODOs to confirm
// - 10.8: if we switch to page 4, then switch back to page 1, check Spaced, and go back to page 4, some controls (progress bar, popup button) are clipped on the sides

@interface boxChild : NSObject
@property uiControl *c;
@property BOOL stretchy;
- (NSView *)view;
@end

@interface boxView : NSView {
	uiBox *b;
	NSMutableArray *children;
	BOOL vertical;
	int padded;

	NSLayoutConstraint *first;
	NSMutableArray *inBetweens;
	NSLayoutConstraint *last;
	NSLayoutConstraint *lastHugging;
	NSMutableArray *otherConstraints;

	NSLayoutAttribute primaryStart;
	NSLayoutAttribute primaryEnd;
	NSLayoutAttribute secondaryStart;
	NSLayoutAttribute secondaryEnd;
	NSLayoutAttribute primarySize;
	NSLayoutConstraintOrientation primaryOrientation;
	NSLayoutConstraintOrientation secondaryOrientation;

	NSLayoutPriority horzHuggingPri;
	NSLayoutPriority vertHuggingPri;
}
- (id)initWithVertical:(BOOL)vert b:(uiBox *)bb;
- (void)onDestroy;
- (void)removeOurConstraints;
- (void)forAll:(void (^)(uintmax_t i, boxChild *b))closure;
- (boxChild *)child:(uintmax_t)i;
- (BOOL)isVertical;
- (void)append:(uiControl *)c stretchy:(int)stretchy;
- (void)delete:(uintmax_t)n;
- (int)isPadded;
- (void)setPadded:(int)p;
- (void)setRealHuggingPriority:(NSLayoutPriority)priority forOrientation:(NSLayoutConstraintOrientation)orientation;
@end

struct uiBox {
	uiDarwinControl c;
	boxView *view;
};

@implementation boxChild

- (NSView *)view
{
	return (NSView *) uiControlHandle(self.c);
}

@end

@implementation boxView

- (id)initWithVertical:(BOOL)vert b:(uiBox *)bb
{
	self = [super initWithFrame:NSZeroRect];
	if (self != nil) {
		// the weird names vert and bb are to shut the compiler up about shadowing because implicit this/self is stupid
		self->b = bb;
		self->vertical = vert;
		self->children = [NSMutableArray new];
		self->inBetweens = [NSMutableArray new];
		self->otherConstraints = [NSMutableArray new];

		if (self->vertical) {
			self->primaryStart = NSLayoutAttributeTop;
			self->primaryEnd = NSLayoutAttributeBottom;
			self->secondaryStart = NSLayoutAttributeLeading;
			self->secondaryEnd = NSLayoutAttributeTrailing;
			self->primarySize = NSLayoutAttributeHeight;
			self->primaryOrientation = NSLayoutConstraintOrientationVertical;
			self->secondaryOrientation = NSLayoutConstraintOrientationHorizontal;
		} else {
			self->primaryStart = NSLayoutAttributeLeading;
			self->primaryEnd = NSLayoutAttributeTrailing;
			self->secondaryStart = NSLayoutAttributeTop;
			self->secondaryEnd = NSLayoutAttributeBottom;
			self->primarySize = NSLayoutAttributeWidth;
			self->primaryOrientation = NSLayoutConstraintOrientationHorizontal;
			self->secondaryOrientation = NSLayoutConstraintOrientationVertical;
		}

		// TODO default high?
		self->horzHuggingPri = NSLayoutPriorityRequired;
		self->vertHuggingPri = NSLayoutPriorityRequired;
	}
	return self;
}

- (void)onDestroy
{
	boxChild *bc;
	uintmax_t i, n;

	// TODO if guard these
	[self removeOurConstraints];
	[self->first release];
	[self->inBetweens release];
	[self->last release];
	[self->lastHugging release];
	[self->otherConstraints release];

	n = [self->children count];
	for (i = 0; i < n; i++) {
		bc = [self child:i];
		uiControlSetParent(bc.c, NULL);
		uiDarwinControlSetSuperview(uiDarwinControl(bc.c), nil);
		uiControlDestroy(bc.c);
	}
	[self->children release];
}

- (void)removeOurConstraints
{
	// TODO if guard these
	[self removeConstraint:self->first];
	[self removeConstraints:self->inBetweens];
	[self->inBetweens removeAllObjects];
	[self removeConstraint:self->last];
	[self removeConstraint:self->lastHugging];
	[self removeConstraints:self->otherConstraints];
	[self->otherConstraints removeAllObjects];
}

- (void)forAll:(void (^)(uintmax_t i, boxChild *b))closure
{
	uintmax_t i, n;

	n = [self->children count];
	for (i = 0; i < n; i++)
		closure(i, [self child:i]);
}

- (boxChild *)child:(uintmax_t)i
{
	return (boxChild *) [self->children objectAtIndex:i];
}

- (BOOL)isVertical
{
	return self->vertical;
}

// TODO something about spinbox hugging
- (void)updateConstraints
{
	uintmax_t i, n;
	BOOL hasStretchy;
	NSView *firstStretchy = nil;
	CGFloat padding;
	NSView *prev, *next;
	NSLayoutConstraint *c;
	NSLayoutPriority priority;

	[super updateConstraints];
	[self removeOurConstraints];

	n = [self->children count];
	if (n == 0)
		return;
	padding = 0;
	if (self->padded)
		padding = 8.0;		// TODO named constant

	// first, attach the first view to the leading
	prev = [[self child:0] view];
	self->first = mkConstraint(prev, self->primaryStart,
		NSLayoutRelationEqual,
		self, self->primaryStart,
		1, 0,
		@"uiBox first primary constraint");
	[self addConstraint:self->first];
	[self->first retain];

	// next, assemble the views in the primary direction
	// they all go in a straight line
	// also figure out whether we have stretchy controls, and which is the first
	if ([self child:0].stretchy) {
		hasStretchy = YES;
		firstStretchy = prev;
	} else
		hasStretchy = NO;
	for (i = 1; i < n; i++) {
		next = [[self child:i] view];
		if (!hasStretchy && [self child:i].stretchy) {
			hasStretchy = YES;
			firstStretchy = next;
		}
		c = mkConstraint(next, self->primaryStart,
			NSLayoutRelationEqual,
			prev, self->primaryEnd,
			1, padding,
			@"uiBox later primary constraint");
		[self addConstraint:c];
		[self->inBetweens addObject:c];
		prev = next;
	}

	// end the primary direction with the <= constraint
	self->last = mkConstraint(prev, self->primaryEnd,
		NSLayoutRelationLessThanOrEqual,
		self, self->primaryEnd,
		1, 0,
		@"uiBox last primary constraint");
	[self addConstraint:self->last];
	[self->last retain];

	// end the primary direction with the == hugging constraint, with the appropriate priority
	self->lastHugging = mkConstraint(prev, self->primaryEnd,
		NSLayoutRelationEqual,
		self, self->primaryEnd,
		1, 0,
		@"uiBox last2 primary constraint");
	priority = NSLayoutPriorityRequired;
	if (self->vertical)
		[self->lastHugging setPriority:self->vertHuggingPri];
	else
		[self->lastHugging setPriority:self->horzHuggingPri];
	[self addConstraint:self->lastHugging];
	[self->lastHugging retain];

	// next: assemble the views in the secondary direction
	// each of them will span the secondary direction
	for (i = 0; i < n; i++) {
		prev = [[self child:i] view];
		c = mkConstraint(prev, self->secondaryStart,
			NSLayoutRelationEqual,
			self, self->secondaryStart,
			1, 0,
			@"uiBox start secondary constraint");
		if (self->vertical)
			[c setPriority:self->horzHuggingPri];
		else
			[c setPriority:self->vertHuggingPri];
		[self addConstraint:c];
		[self->otherConstraints addObject:c];
		c = mkConstraint(prev, self->secondaryEnd,
			NSLayoutRelationEqual,
			self, self->secondaryEnd,
			1, 0,
			@"uiBox start secondary constraint");
		if (self->vertical)
			[c setPriority:self->horzHuggingPri];
		else
			[c setPriority:self->vertHuggingPri];
		[self addConstraint:c];
		[self->otherConstraints addObject:c];
	}

	// finally, set sizes for stretchy controls
	if (hasStretchy)
		for (i = 0; i < n; i++) {
			if (![self child:i].stretchy)
				continue;
			prev = [[self child:i] view];
			if (prev == firstStretchy)
				continue;
			c = mkConstraint(prev, self->primarySize,
				NSLayoutRelationEqual,
				firstStretchy, self->primarySize,
				1, 0,
				@"uiBox stretchy sizing");
			[self addConstraint:c];
			[self->otherConstraints addObject:c];
		}
}

- (void)append:(uiControl *)c stretchy:(int)stretchy
{
	boxChild *bc;

	bc = [boxChild new];
	bc.c = c;
	bc.stretchy = stretchy;

	uiControlSetParent(bc.c, uiControl(self->b));
	uiDarwinControlSetSuperview(uiDarwinControl(bc.c), self);
	uiDarwinControlSyncEnableState(uiDarwinControl(bc.c), uiControlEnabledToUser(uiControl(self->b)));

	// if a control is stretchy, it should not hug in the primary direction
	// otherwise, it should *forcibly* hug
	if (stretchy)
		uiDarwinControlSetRealHuggingPriority(uiDarwinControl(bc.c), NSLayoutPriorityDefaultLow, self->primaryOrientation);
	else
		// TODO will default high work?
		uiDarwinControlSetRealHuggingPriority(uiDarwinControl(bc.c), NSLayoutPriorityRequired, self->primaryOrientation);
	// make sure controls don't hug their secondary direction so they fill the width of the view
	uiDarwinControlSetRealHuggingPriority(uiDarwinControl(bc.c), NSLayoutPriorityDefaultLow, self->secondaryOrientation);

	[self->children addObject:bc];
	[bc release];		// we don't need the initial reference now

	[self setNeedsUpdateConstraints:YES];
}

- (void)delete:(uintmax_t)n
{
	boxChild *bc;

	bc = [self child:n];

	uiControlSetParent(bc.c, NULL);
	uiDarwinControlSetSuperview(uiDarwinControl(bc.c), nil);

	[self->children removeObjectAtIndex:n];

	[self setNeedsUpdateConstraints:YES];
}

- (int)isPadded
{
	return self->padded;
}

- (void)setPadded:(int)p
{
	CGFloat padding;
	uintmax_t i, n;
	NSLayoutConstraint *c;

	self->padded = p;

	// TODO split into method (using above code)
	padding = 0;
	if (self->padded)
		padding = 8.0;
	n = [self->inBetweens count];
	for (i = 0; i < n; i++) {
		c = (NSLayoutConstraint *) [self->inBetweens objectAtIndex:i];
		[c setConstant:padding];
	}
	// TODO call anything?
}

- (void)setRealHuggingPriority:(NSLayoutPriority)priority forOrientation:(NSLayoutConstraintOrientation)orientation
{
	if (orientation == NSLayoutConstraintOrientationVertical)
		self->vertHuggingPri = priority;
	else
		self->horzHuggingPri = priority;
	[self setNeedsUpdateConstraints:YES];
}

@end

static void uiBoxDestroy(uiControl *c)
{
	uiBox *b = uiBox(c);

	[b->view onDestroy];
	[b->view release];
	uiFreeControl(uiControl(b));
}

uiDarwinControlDefaultHandle(uiBox, view)
uiDarwinControlDefaultParent(uiBox, view)
uiDarwinControlDefaultSetParent(uiBox, view)
uiDarwinControlDefaultToplevel(uiBox, view)
uiDarwinControlDefaultVisible(uiBox, view)
uiDarwinControlDefaultShow(uiBox, view)
uiDarwinControlDefaultHide(uiBox, view)
uiDarwinControlDefaultEnabled(uiBox, view)
uiDarwinControlDefaultEnable(uiBox, view)
uiDarwinControlDefaultDisable(uiBox, view)

static void uiBoxSyncEnableState(uiDarwinControl *c, int enabled)
{
	uiBox *b = uiBox(c);

	if (uiDarwinShouldStopSyncEnableState(uiDarwinControl(b), enabled))
		return;
	[b->view forAll:^(uintmax_t i, boxChild *bc) {
		uiDarwinControlSyncEnableState(uiDarwinControl(bc.c), enabled);
	}];
}

uiDarwinControlDefaultSetSuperview(uiBox, view)

static void uiBoxSetRealHuggingPriority(uiDarwinControl *c, NSLayoutPriority priority, NSLayoutConstraintOrientation orientation)
{
	uiBox *b = uiBox(c);

	[b->view setRealHuggingPriority:priority forOrientation:orientation];
}

void uiBoxAppend(uiBox *b, uiControl *c, int stretchy)
{
	[b->view append:c stretchy:stretchy];
}

void uiBoxDelete(uiBox *b, uintmax_t n)
{
	[b->view delete:n];
}

int uiBoxPadded(uiBox *b)
{
	return [b->view isPadded];
}

void uiBoxSetPadded(uiBox *b, int padded)
{
	[b->view setPadded:padded];
}

static uiBox *finishNewBox(BOOL vertical)
{
	uiBox *b;

	uiDarwinNewControl(uiBox, b);

	b->view = [[boxView alloc] initWithVertical:vertical b:b];

	return b;
}

uiBox *uiNewHorizontalBox(void)
{
	return finishNewBox(NO);
}

uiBox *uiNewVerticalBox(void)
{
	return finishNewBox(YES);
}
