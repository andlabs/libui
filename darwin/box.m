// 15 august 2015
#import "uipriv_darwin.h"

// TODOs:
// - tab on page 2 is glitched initially
// - 10.8: if we switch to page 4, then switch back to page 1, check Spaced, and go back to page 4, some controls (progress bar, popup button) are clipped on the sides
// - calling layoutSubtreeIfNeeded on a superview of the box will cause the following intrinsic content size thing to not work until the window is resized in the primary direction; this is bad if we ever add a Splitter...
// - moving around randomly through the tabs does this too

// the default is to have no intrinsic content size; this wreaks havoc with nested no-stretchy boxes fighting over which box gets the remaining space
// let's use a 0x0 intrinsic size instead; that seems to fix things
@interface libuiNoStretchyView : NSView
@end

@implementation libuiNoStretchyView

- (NSSize)intrinsicContentSize
{
	return NSMakeSize(0, 0);
}

@end

@interface boxView : NSView
@property uiBox *b;
@property NSLayoutConstraint *last;
@property BOOL willRelayout;
@property BOOL stretchy;
@end

struct uiBox {
	uiDarwinControl c;
	boxView *view;
	BOOL vertical;
	int padded;
	NSMutableArray *children;		// []NSValue<uiControl *>
	NSMutableArray *stretchy;		// []NSNumber
	// this view is made stretchy if there are no stretchy views
	NSView *noStretchyView;
	NSLayoutAttribute primaryStart;
	NSLayoutAttribute primaryEnd;
	NSLayoutAttribute secondaryStart;
	NSLayoutAttribute secondaryEnd;
	NSLayoutAttribute primarySize;
	NSLayoutConstraintOrientation primaryOrientation;
	NSLayoutConstraintOrientation secondaryOrientation;
};

static uiControl *childAt(uiBox *b, uintmax_t n)
{
	NSValue *v;

	v = (NSValue *) [b->children objectAtIndex:n];
	return (uiControl *) [v pointerValue];
}

static void uiBoxDestroy(uiControl *c)
{
	uiBox *b = uiBox(c);
	uintmax_t i;
	uiControl *child;
	NSView *childView;

	for (i = 0; i < [b->children count]; i++) {
		child = childAt(b, i);
		childView = (NSView *) uiControlHandle(child);
		[childView removeFromSuperview];
		uiControlSetParent(child, NULL);
		uiControlDestroy(child);
	}
	if ([b->noStretchyView superview] != nil)
		[b->noStretchyView removeFromSuperview];
	[b->noStretchyView release];
	[b->children release];
	[b->stretchy release];
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
	NSUInteger i;

	if (uiDarwinShouldStopSyncEnableState(uiDarwinControl(b), enabled))
		return;
	for (i = 0; i < [b->children count]; i++) {
		NSValue *v;
		uiControl *child;

		v = (NSValue *) [b->children objectAtIndex:i];
		// TODO change all these other instances of casts to conversions
		child = uiControl([v pointerValue]);
		uiDarwinControlSyncEnableState(uiDarwinControl(child), enabled);
	}
}

uiDarwinControlDefaultSetSuperview(uiBox, view)
uiDarwinControlDefaultChildrenShouldAllowSpaceAtTrailingEdge(uiBox, view)
uiDarwinControlDefaultChildrenShouldAllowSpaceAtBottom(uiBox, view)

static int isStretchy(uiBox *b, uintmax_t n)
{
	NSNumber *num;

	num = (NSNumber *) [b->stretchy objectAtIndex:n];
	return [num intValue];
}

static NSView *boxChildView(uiBox *b, uintmax_t n)
{
	NSValue *val;
	uiControl *c;

	val = (NSValue *) [b->children objectAtIndex:n];
	c = (uiControl *) [val pointerValue];
	return (NSView *) uiControlHandle(c);
}

static BOOL addRemoveNoStretchyView(uiBox *b, BOOL hasStretchy)
{
	if (!hasStretchy) {
		if ([b->noStretchyView superview] == nil)
			[b->view addSubview:b->noStretchyView];
		return YES;
	}
	if ([b->noStretchyView superview] != nil)
		[b->noStretchyView removeFromSuperview];
	return NO;
}

@implementation boxView

- (NSLayoutConstraint *)mkLast:(NSLayoutRelation)relation on:(NSView *)view
{
	return mkConstraint(view, self.b->primaryEnd,
		relation,
		self.b->view, self.b->primaryEnd,
		1, 0,
		@"uiBox last primary constraint");
}

// TODO do we still need to set hugging? I think we do for stretchy controls...
// TODO try unsetting spinbox intrinsics and seeing what happens
- (void)recreateConstraints
{
	uiBox *b = self.b;
	uintmax_t i, n;
	BOOL hasStretchy;
	NSView *firstStretchy = nil;
	CGFloat padding;
	NSView *prev, *next;
	BOOL hasNoStretchyView;

	n = [b->children count];
	if (n == 0)
		return;
	padding = 0;
	if (b->padded)
		padding = 8.0;		// TODO named constant

	[b->view removeConstraints:[b->view constraints]];

	// first, attach the first view to the leading
	prev = boxChildView(b, 0);
	[b->view addConstraint:mkConstraint(prev, b->primaryStart,
		NSLayoutRelationEqual,
		b->view, b->primaryStart,
		1, 0,
		@"uiBox first primary constraint")];

	// next, assemble the views in the primary direction
	// they all go in a straight line
	// also figure out whether we have stretchy controls, and which is the first
	if (isStretchy(b, 0)) {
		hasStretchy = YES;
		firstStretchy = prev;
	} else
		hasStretchy = NO;
	for (i = 1; i < n; i++) {
		next = boxChildView(b, i);
		if (!hasStretchy && isStretchy(b, i)) {
			hasStretchy = YES;
			firstStretchy = next;
		}
		[b->view addConstraint:mkConstraint(next, b->primaryStart,
			NSLayoutRelationEqual,
			prev, b->primaryEnd,
			1, padding,
			@"uiBox later primary constraint")];
		prev = next;
	}

	// if there is a stretchy control, add the no-stretchy view
	self.stretchy = hasStretchy;
	NSLayoutRelation relation = NSLayoutRelationEqual;
	if (!hasStretchy)
		relation = NSLayoutRelationLessThanOrEqual;

	// and finally end the primary direction
	b->view.last = [b->view mkLast:relation on:prev];
	[b->view addConstraint:b->view.last];

	// next: assemble the views in the secondary direction
	// each of them will span the secondary direction
	for (i = 0; i < n; i++) {
		[b->view addConstraint:mkConstraint(boxChildView(b, i), b->secondaryStart,
			NSLayoutRelationEqual,
			b->view, b->secondaryStart,
			1, 0,
			@"uiBox start secondary constraint")];
		[b->view addConstraint:mkConstraint(boxChildView(b, i), b->secondaryEnd,
			NSLayoutRelationEqual,
			b->view, b->secondaryEnd,
			1, 0,
			@"uiBox start secondary constraint")];
	}

	// finally, set sizes for stretchy controls
	if (hasStretchy)
		for (i = 0; i < n; i++) {
			if (!isStretchy(b, i))
				continue;
			prev = boxChildView(b, i);
			if (prev == firstStretchy)
				continue;
			[b->view addConstraint:mkConstraint(prev, b->primarySize,
				NSLayoutRelationEqual,
				firstStretchy, b->primarySize,
				1, 0,
				@"uiBox stretchy sizing")];
		}
}

- (void)updateConstraints
{
	[super updateConstraints];
	self.willRelayout = YES;
}

- (BOOL)inStretchyView
{
	uiControl *c;
	NSView *v;

	c = uiControlParent(uiControl(self.b));
	v = (NSView *) uiControlHandle(c);
	if ([v isKindOfClass:[boxView class]]) {
		boxView *bv = (boxView *) v;
		uintmax_t i, n;

		n = [bv.b->children count];
		if (bv.b->vertical != self.b->vertical)
			return YES;
		for (i = 0; i < n; i++)
			if (isStretchy(bv.b, i))
				return YES;
		return NO;
	}
	return YES;
}

- (void)layout
{
	[super layout];
	if (!self.willRelayout) return;
	self.willRelayout = NO;
	if (!self.stretchy && ![self inStretchyView]) {
		NSView *prev;

		prev = [self.last firstItem];
		[self removeConstraint:self.last];
		self.last = [self mkLast:NSLayoutRelationEqual on:prev];
		[self addConstraint:self.last];
		[self updateConstraintsForSubtreeIfNeeded];
		[super layout];
	}
}

@end

void uiBoxAppend(uiBox *b, uiControl *c, int stretchy)
{
	NSView *childView;

	childView = (NSView *) uiControlHandle(c);
	[b->children addObject:[NSValue valueWithPointer:c]];
	[b->stretchy addObject:[NSNumber numberWithInt:stretchy]];

	uiControlSetParent(c, uiControl(b));
	uiDarwinControlSetSuperview(uiDarwinControl(c), b->view);
	uiDarwinControlSyncEnableState(uiDarwinControl(c), uiControlEnabledToUser(uiControl(b)));

	// TODO save the old hugging priorities
	// if a control is stretchy, it should not hug in the primary direction
	// otherwise, it should *forcibly* hug
	if (stretchy)
		setHuggingPri(childView, NSLayoutPriorityDefaultLow, b->primaryOrientation);
	else
		// TODO will default high work?
		setHuggingPri(childView, NSLayoutPriorityRequired, b->primaryOrientation);
	// make sure controls don't hug their secondary direction so they fill the width of the view
	setHuggingPri(childView, NSLayoutPriorityDefaultLow, b->secondaryOrientation);

	[b->view recreateConstraints];
	[b->view setNeedsUpdateConstraints:YES];
}

void uiBoxDelete(uiBox *b, uintmax_t n)
{
	NSValue *v;
	uiControl *removed;
	NSView *removedView;

	v = (NSValue *) [b->children objectAtIndex:n];
	removed = (uiControl *) [v pointerValue];
	removedView = (NSView *) uiControlHandle(removed);
	[removedView removeFromSuperview];
	uiControlSetParent(removed, NULL);
	[b->children removeObjectAtIndex:n];
	[b->stretchy removeObjectAtIndex:n];
	[b->view recreateConstraints];
	[b->view setNeedsUpdateConstraints:YES];
}

int uiBoxPadded(uiBox *b)
{
	return b->padded;
}

void uiBoxSetPadded(uiBox *b, int padded)
{
	b->padded = padded;
	[b->view recreateConstraints];
	[b->view setNeedsUpdateConstraints:YES];
}

static uiBox *finishNewBox(BOOL vertical)
{
	uiBox *b;

	uiDarwinNewControl(uiBox, b);

	b->view = [[boxView alloc] initWithFrame:NSZeroRect];
	b->view.b = b;

	b->children = [NSMutableArray new];
	b->stretchy = [NSMutableArray new];

	b->vertical = vertical;
	if (b->vertical) {
		b->primaryStart = NSLayoutAttributeTop;
		b->primaryEnd = NSLayoutAttributeBottom;
		b->secondaryStart = NSLayoutAttributeLeading;
		b->secondaryEnd = NSLayoutAttributeTrailing;
		b->primarySize = NSLayoutAttributeHeight;
		b->primaryOrientation = NSLayoutConstraintOrientationVertical;
		b->secondaryOrientation = NSLayoutConstraintOrientationHorizontal;
	} else {
		b->primaryStart = NSLayoutAttributeLeading;
		b->primaryEnd = NSLayoutAttributeTrailing;
		b->secondaryStart = NSLayoutAttributeTop;
		b->secondaryEnd = NSLayoutAttributeBottom;
		b->primarySize = NSLayoutAttributeWidth;
		b->primaryOrientation = NSLayoutConstraintOrientationHorizontal;
		b->secondaryOrientation = NSLayoutConstraintOrientationVertical;
	}

	b->noStretchyView = [[libuiNoStretchyView alloc] initWithFrame:NSZeroRect];
	[b->noStretchyView setTranslatesAutoresizingMaskIntoConstraints:NO];
	setHuggingPri(b->noStretchyView, NSLayoutPriorityDefaultLow, NSLayoutConstraintOrientationHorizontal);
	setHuggingPri(b->noStretchyView, NSLayoutPriorityDefaultLow, NSLayoutConstraintOrientationVertical);

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
