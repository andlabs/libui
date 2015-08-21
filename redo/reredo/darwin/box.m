// 15 august 2015
#import "uipriv_darwin.h"

// TODOs:
// - page 2 isn't growable - I think this is because the button in the tab isn't stretchy

struct uiBox {
	uiDarwinControl c;
	NSView *view;
	BOOL vertical;
	int padded;
	NSMutableArray *children;		// []NSValue<uiControl *>
	NSMutableArray *stretchy;		// []NSNumber
	// this view is made stretchy if there are no stretchy views
	NSView *noStretchyView;
	NSString *primaryDirPrefix;
	NSString *secondaryDirPrefix;
	NSLayoutConstraintOrientation primaryOrientation;
	NSLayoutConstraintOrientation secondaryOrientation;
};

static void onDestroy(uiBox *);

uiDarwinDefineControlWithOnDestroy(
	uiBox,								// type name
	uiBoxType,							// type function
	view,								// handle
	onDestroy(this);						// on destroy
)

static uiControl *childAt(uiBox *b, uintmax_t n)
{
	NSValue *v;

	v = (NSValue *) [b->children objectAtIndex:n];
	return (uiControl *) [v pointerValue];
}

static void onDestroy(uiBox *b)
{
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
}

static void boxContainerUpdateState(uiControl *c)
{
	uiBox *b = uiBox(c);
	NSUInteger i;

	for (i = 0; i < [b->children count]; i++) {
		NSValue *v;
		uiControl *child;

		v = (NSValue *) [b->children objectAtIndex:i];
		// TODO change all these other instances of casts to conversions
		child = uiControl([v pointerValue]);
		controlUpdateState(child);
	}
}

static NSString *viewName(uintmax_t n)
{
	return [NSString stringWithFormat:@"view%ju", n];
}

static NSString *widthMetricName(uintmax_t n)
{
	return [NSString stringWithFormat:@"view%juwidth", n];
}

static NSString *heightMetricName(uintmax_t n)
{
	return [NSString stringWithFormat:@"view%juheight", n];
}

static int isStretchy(uiBox *b, uintmax_t n)
{
	NSNumber *num;

	num = (NSNumber *) [b->stretchy objectAtIndex:n];
	return [num intValue];
}

// TODO do we still need to set hugging? I think we do for stretchy controls...
// TODO try unsetting spinbox intrinsics and seeing what happens
static void relayout(uiBox *b)
{
	NSMutableDictionary *metrics;
	NSMutableDictionary *views;
	uintmax_t i, n;
	BOOL hasStretchy;
	uintmax_t firstStretchy;
	NSMutableString *constraint;

	if ([b->children count] == 0)
		return;

	[b->view removeConstraints:[b->view constraints]];

	// first lay out all children, collect the views and their fitting sizes (for non-stretchy controls)
	// also figure out which is the first stretchy control, if any
	metrics = [NSMutableDictionary new];
	views = [NSMutableDictionary new];
	hasStretchy = NO;
	n = 0;
	while (n < [b->children count]) {
		uiControl *child;
		uiDarwinControl *cc;
		NSView *childView;
		NSSize fittingSize;

		child = childAt(b, n);
		cc = uiDarwinControl(child);
		childView = (NSView *) uiControlHandle(child);
		[views setObject:childView forKey:viewName(n)];
		(*(cc->Relayout))(cc);
		fittingSize = fittingAlignmentSize(childView);
		[metrics setObject:[NSNumber numberWithDouble:fittingSize.width]
			forKey:widthMetricName(n)];
		[metrics setObject:[NSNumber numberWithDouble:fittingSize.height]
			forKey:heightMetricName(n)];
		if (!hasStretchy && isStretchy(b, n)) {
			hasStretchy = YES;
			firstStretchy = n;
		}
		n++;
	}

	// if there are no stretchy controls, we must add the no-stretchy view
	// if there are, we must remove it
	if (!hasStretchy) {
		if ([b->noStretchyView superview] == nil)
			[b->view addSubview:b->noStretchyView];
		[views setObject:b->noStretchyView forKey:@"noStretchyView"];
	} else {
		if ([b->noStretchyView superview] != nil)
			[b->noStretchyView removeFromSuperview];
	}

	// next, assemble the views in the primary direction
	// they all go in a straight line
	constraint = [NSMutableString new];
	[constraint appendString:b->primaryDirPrefix];
	[constraint appendString:@"|"];
	for (i = 0; i < n; i++) {
		if (b->padded && i != 0)
			[constraint appendString:@"-"];
		[constraint appendString:@"["];
		[constraint appendString:viewName(i)];
		// implement multiple stretchiness properly
		if (isStretchy(b, i) && i != firstStretchy) {
			[constraint appendString:@"(=="];
			[constraint appendString:viewName(firstStretchy)];
			[constraint appendString:@")"];
		}
		// if the control is not stretchy, restrict it to the fitting size
		if (!isStretchy(b, i)) {
			[constraint appendString:@"(=="];
			if (b->vertical)
				[constraint appendString:heightMetricName(i)];
			else
				[constraint appendString:widthMetricName(i)];
			[constraint appendString:@")"];
		}
		[constraint appendString:@"]"];
	}
	if (!hasStretchy)
		// don't space between the last control and the no-stretchy view
		[constraint appendString:@"[noStretchyView]"];
	[constraint appendString:@"|"];
	addConstraint(b->view, constraint, metrics, views);
	[constraint release];

	// next: assemble the views in the secondary direction
	// each of them will span the secondary direction
	for (i = 0; i < n; i++) {
		constraint = [NSMutableString new];
		[constraint appendString:b->secondaryDirPrefix];
		[constraint appendString:@"|["];
		[constraint appendString:viewName(i)];
		[constraint appendString:@"]|"];
		addConstraint(b->view, constraint, nil, views);
		[constraint release];
	}
	if (!hasStretchy) {			// and again to the no-stretchy view
		constraint = [NSMutableString new];
		[constraint appendString:b->secondaryDirPrefix];
		[constraint appendString:@"|[noStretchyView]|"];
		addConstraint(b->view, constraint, nil, views);
		[constraint release];
	}

	[metrics release];
	[views release];
}

static void boxRelayout(uiDarwinControl *c)
{
	relayout(uiBox(c));
}

void uiBoxAppend(uiBox *b, uiControl *c, int stretchy)
{
	NSView *childView;

	childView = (NSView *) uiControlHandle(c);
	[b->children addObject:[NSValue valueWithPointer:c]];
	[b->stretchy addObject:[NSNumber numberWithInt:stretchy]];

	uiControlSetParent(c, uiControl(b));
	[b->view addSubview:childView];

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

	b = (uiBox *) uiNewControl(uiBoxType());

	b->view = [[NSView alloc] initWithFrame:NSZeroRect];

	b->children = [NSMutableArray new];
	b->stretchy = [NSMutableArray new];

	b->vertical = vertical;
	if (b->vertical) {
		b->primaryDirPrefix = @"V:";
		b->secondaryDirPrefix = @"H:";
		b->primaryOrientation = NSLayoutConstraintOrientationVertical;
		b->secondaryOrientation = NSLayoutConstraintOrientationHorizontal;
	} else {
		b->primaryDirPrefix = @"H:";
		b->secondaryDirPrefix = @"V:";
		b->primaryOrientation = NSLayoutConstraintOrientationHorizontal;
		b->secondaryOrientation = NSLayoutConstraintOrientationVertical;
	}

	b->noStretchyView = [[NSView alloc] initWithFrame:NSZeroRect];
	[b->noStretchyView setTranslatesAutoresizingMaskIntoConstraints:NO];
	setHuggingPri(b->noStretchyView, NSLayoutPriorityDefaultLow, NSLayoutConstraintOrientationHorizontal);
	setHuggingPri(b->noStretchyView, NSLayoutPriorityDefaultLow, NSLayoutConstraintOrientationVertical);

	uiDarwinFinishNewControl(b, uiBox);
	uiControl(b)->ContainerUpdateState = boxContainerUpdateState;
	uiDarwinControl(b)->Relayout = boxRelayout;

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
