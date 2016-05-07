// 15 august 2015
#import "uipriv_darwin.h"

// TODOs:
// - tab on page 2 is glitched initially
// - 10.8: if we switch to page 4, then switch back to page 1, check Spaced, and go back to page 4, some controls (progress bar, popup button) are clipped on the sides
// - calling layoutSubtreeIfNeeded on a superview of the box will cause the following intrinsic content size thing to not work until the window is resized in the primary direction; this is bad if we ever add a Splitter...
// - moving around randomly through the tabs does this too

@interface boxChild : NSObject
@property uiControl *c;
@property BOOL stretchy;
@property NSLayoutPriority oldHorzHuggingPri;
@property NSLayoutPriority oldVertHuggingPri;
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
	NSMutableArray *otherConstraints;

	NSLayoutAttribute primaryStart;
	NSLayoutAttribute primaryEnd;
	NSLayoutAttribute secondaryStart;
	NSLayoutAttribute secondaryEnd;
	NSLayoutAttribute primarySize;
	NSLayoutConstraintOrientation primaryOrientation;
	NSLayoutConstraintOrientation secondaryOrientation;

	BOOL layingOut;
}
- (id)initWithVertical:(BOOL)vert b:(uiBox *)bb;
- (void)onDestroy;
- (void)removeOurConstraints;
- (void)forAll:(void (^)(uintmax_t i, boxChild *b))closure;
- (boxChild *)child:(uintmax_t)i;
- (BOOL)isVertical;
- (void)recreateConstraints;
- (BOOL)isStretchy;
- (void)append:(uiControl *)c stretchy:(int)stretchy;
- (void)delete:(uintmax_t)n;
- (int)isPadded;
- (void)setPadded:(int)p;
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
	}
	return self;
}

- (void)onDestroy
{
	boxChild *bc;
	uintmax_t i, n;

	[self removeOurConstraints];
	[self->first release];
	[self->inBetweens release];
	[self->last release];
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
	[self removeConstraint:self->first];
	[self removeConstraints:self->inBetweens];
	[self removeConstraint:self->last];
	[self removeConstraints:self->otherConstraints];
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
// TODO should this be in updateConstraints?
- (void)recreateConstraints
{
	uintmax_t i, n;
	BOOL hasStretchy;
	NSView *firstStretchy = nil;
	CGFloat padding;
	NSView *prev, *next;
	NSLayoutConstraint *c;
	NSLayoutRelation relation;

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

	// if there is a stretchy control, add the no-stretchy view
	relation = NSLayoutRelationLessThanOrEqual;
	if (hasStretchy)
		relation = NSLayoutRelationEqual;

	// and finally end the primary direction
	self->last = mkConstraint(prev, self->primaryEnd,
		relation,
		self, self->primaryEnd,
		1, 0,
		@"uiBox last primary constraint");
	[self addConstraint:self->last];
	[self->last retain];

	// next: assemble the views in the secondary direction
	// each of them will span the secondary direction
	for (i = 0; i < n; i++) {
		prev = [[self child:i] view];
		c = mkConstraint(prev, self->secondaryStart,
			NSLayoutRelationEqual,
			self, self->secondaryStart,
			1, 0,
			@"uiBox start secondary constraint");
		[self addConstraint:c];
		[self->otherConstraints addObject:c];
		c = mkConstraint(prev, self->secondaryEnd,
			NSLayoutRelationEqual,
			self, self->secondaryEnd,
			1, 0,
			@"uiBox start secondary constraint");
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

- (BOOL)isStretchy
{
	uintmax_t i, n;

	n = [self->children count];
	for (i = 0; i < n; i++)
		if ([self child:i].stretchy)
			return YES;
	return NO;
}

// to avoid repeatedly calling layout
- (void)updateConstraints
{
	[super updateConstraints];
	self->layingOut = YES;
}

- (void)layout
{
	uiControl *parent;
	BOOL shouldExpand;
	NSView *lastView;

	[super layout];
	if (!self->layingOut)
		return;
	self->layingOut = NO;

	if ([self->children count] == 0)
		return;
	if ([self isStretchy])
		return;
	parent = uiControlParent(uiControl(self->b));
	if (parent == NULL)		// do nothing if no parent
		return;			// TODO what happens if parents change?
	if (self->vertical)
		shouldExpand = uiDarwinControlChildrenShouldAllowSpaceAtBottom(uiDarwinControl(parent));
	else
		shouldExpand = uiDarwinControlChildrenShouldAllowSpaceAtTrailingEdge(uiDarwinControl(parent));
	if (shouldExpand)
		return;

	// okay, so we need to redo self->last to not expand
	[self removeConstraint:self->last];
	lastView = (NSView *) [self->last firstItem];
	[self->last release];
	self->last = mkConstraint(lastView, self->primaryEnd,
		NSLayoutRelationEqual,
		self, self->primaryEnd,
		1, 0,
		@"uiBox last primary constraint");
	[self addConstraint:self->last];
	[self->last retain];
	[self updateConstraintsForSubtreeIfNeeded];
	[super layout];
}

- (void)append:(uiControl *)c stretchy:(int)stretchy
{
	boxChild *bc;
	NSView *childView;

	bc = [boxChild new];
	bc.c = c;
	bc.stretchy = stretchy;
	childView = [bc view];
	bc.oldHorzHuggingPri = horzHuggingPri(childView);
	bc.oldVertHuggingPri = vertHuggingPri(childView);

	uiControlSetParent(bc.c, uiControl(self->b));
	uiDarwinControlSetSuperview(uiDarwinControl(bc.c), self);
	uiDarwinControlSyncEnableState(uiDarwinControl(bc.c), uiControlEnabledToUser(uiControl(self->b)));

	// if a control is stretchy, it should not hug in the primary direction
	// otherwise, it should *forcibly* hug
	if (stretchy)
		setHuggingPri(childView, NSLayoutPriorityDefaultLow, self->primaryOrientation);
	else
		// TODO will default high work?
		setHuggingPri(childView, NSLayoutPriorityRequired, self->primaryOrientation);
	// make sure controls don't hug their secondary direction so they fill the width of the view
	setHuggingPri(childView, NSLayoutPriorityDefaultLow, self->secondaryOrientation);

	[self->children addObject:bc];
	[bc release];		// we don't need the initial reference now

	// TODO if we comment out these next calls to recreateConstraints and have everything in updateConstraints then our -layout stuff doesn't work
	[self recreateConstraints];
	[self setNeedsUpdateConstraints:YES];
}

- (void)delete:(uintmax_t)n
{
	boxChild *bc;
	NSView *removedView;

	bc = [self child:n];
	removedView = [bc view];

	uiControlSetParent(bc.c, NULL);
	uiDarwinControlSetSuperview(uiDarwinControl(bc.c), nil);

	setHorzHuggingPri(removedView, bc.oldHorzHuggingPri);
	setVertHuggingPri(removedView, bc.oldVertHuggingPri);

	[self->children removeObjectAtIndex:n];

	[self recreateConstraints];
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

static BOOL uiBoxChildrenShouldAllowSpaceAtTrailingEdge(uiDarwinControl *c)
{
	uiBox *b = uiBox(c);

	// return NO if this box is horizontal so nested horizontal boxes don't lead to ambiguity
	return [b->view isVertical];
}

static BOOL uiBoxChildrenShouldAllowSpaceAtBottom(uiDarwinControl *c)
{
	uiBox *b = uiBox(c);

	// return NO if this box is vertical so nested vertical boxes don't lead to ambiguity
	return ![b->view isVertical];
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
