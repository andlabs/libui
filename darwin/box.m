// 15 august 2015
#import "uipriv_darwin.h"

// TODOs:
// - page 2 isn't growable - I think this is because the button in the tab isn't stretchy
// - tab on page 2 is glitched
// - separators on page 4 have variable padding after them
// - 10.8: if we switch to page 4, then switch back to page 1, check Spaced, and go back to page 4, some controls (progress bar, popup button) are clipped on the sides
// - wrong padding between controls in the queuemaintest and the control gallery...

struct uiBox {
	uiDarwinControl c;
	NSView *view;
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

static int isStretchy(uiBox *b, uintmax_t n)
{
	NSNumber *num;

	num = (NSNumber *) [b->stretchy objectAtIndex:n];
	return [num intValue];
}

static NSView *boxView(uiBox *b, uintmax_t n)
{
	NSValue *val;
	uiControl *c;

	val = (NSValue *) [b->children objectAtIndex:n];
	c = (uiControl *) [val pointerValue];
	return (NSView *) uiControlHandle(c);
}

static void addRemoveNoStretchyView(uiBox *b, BOOL hasStretchy)
{
	if (!hasStretchy) {
		if ([b->noStretchyView superview] == nil)
			[b->view addSubview:b->noStretchyView];
	} else {
		if ([b->noStretchyView superview] != nil)
			[b->noStretchyView removeFromSuperview];
	}
}

// TODO do we still need to set hugging? I think we do for stretchy controls...
// TODO try unsetting spinbox intrinsics and seeing what happens
static void relayout(uiBox *b)
{
	uintmax_t i, n;
	BOOL hasStretchy;
	NSView *firstStretchy = nil;
	CGFloat padding;
	NSView *prev, *next;

	n = [b->children count];
	if (n == 0)
		return;
	padding = 0;
	if (b->padded)
		padding = 8.0;		// TODO named constant

	[b->view removeConstraints:[b->view constraints]];

	// first, attach the first view to the leading
	prev = boxView(b, 0);
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
		next = boxView(b, i);
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
	addRemoveNoStretchyView(b, hasStretchy);
	if (hasStretchy) {
		[b->view addConstraint:mkConstraint(b->noStretchyView, b->primaryStart,
			NSLayoutRelationEqual,
			prev, b->primaryEnd,
			1, 0,				// don't space between the last control and the no-stretchy view
			@"uiBox no-stretchy primary constraint")];
		prev = b->noStretchyView;
	}

	// and finally end the primary direction
	[b->view addConstraint:mkConstraint(prev, b->primaryEnd,
		NSLayoutRelationEqual,
		b->view, b->primaryEnd,
		1, 0,
		@"uiBox last primary constraint")];

	// next: assemble the views in the secondary direction
	// each of them will span the secondary direction
	for (i = 0; i < n; i++) {
		[b->view addConstraint:mkConstraint(boxView(b, i), b->secondaryStart,
			NSLayoutRelationEqual,
			b->view, b->secondaryStart,
			1, 0,
			@"uiBox start secondary constraint")];
		[b->view addConstraint:mkConstraint(boxView(b, i), b->secondaryEnd,
			NSLayoutRelationEqual,
			b->view, b->secondaryEnd,
			1, 0,
			@"uiBox start secondary constraint")];
	}
	if (!hasStretchy) {			// and again to the no-stretchy view
		[b->view addConstraint:mkConstraint(b->noStretchyView, b->secondaryStart,
			NSLayoutRelationEqual,
			b->view, b->secondaryStart,
			1, 0,
			@"uiBox no-stretchy view start secondary constraint")];
		[b->view addConstraint:mkConstraint(b->noStretchyView, b->secondaryEnd,
			NSLayoutRelationEqual,
			b->view, b->secondaryEnd,
			1, 0,
			@"uiBox no-stretchy view start secondary constraint")];
	}

	// finally, set sizes for stretchy controls
	if (hasStretchy)
		for (i = 0; i < n; i++) {
			if (!isStretchy(b, i))
				continue;
			prev = boxView(b, i);
			if (prev == firstStretchy)
				continue;
			[b->view addConstraint:mkConstraint(prev, b->primarySize,
				NSLayoutRelationEqual,
				firstStretchy, b->primarySize,
				1, 0,
				@"uiBox stretchy sizing")];
		}
}

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

	relayout(b);
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
	relayout(b);
}

int uiBoxPadded(uiBox *b)
{
	return b->padded;
}

void uiBoxSetPadded(uiBox *b, int padded)
{
	b->padded = padded;
	relayout(b);
}

static uiBox *finishNewBox(BOOL vertical)
{
	uiBox *b;

	uiDarwinNewControl(uiBox, b);

	b->view = [[NSView alloc] initWithFrame:NSZeroRect];

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

	b->noStretchyView = [[NSView alloc] initWithFrame:NSZeroRect];
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
