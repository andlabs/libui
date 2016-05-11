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
#if 0 /* TODO */
@property NSLayoutPriority oldHorzHuggingPri;
@property NSLayoutPriority oldVertHuggingPri;
#endif
- (NSView *)view;
@end

@interface boxView : NSView {
	uiBox *b;
	NSMutableArray *children;
	BOOL vertical;
	int padded;

#if 0 /* TODO */
	NSLayoutAttribute primaryStart;
	NSLayoutAttribute primaryEnd;
	NSLayoutAttribute secondaryStart;
	NSLayoutAttribute secondaryEnd;
	NSLayoutAttribute primarySize;
	NSLayoutConstraintOrientation primaryOrientation;
	NSLayoutConstraintOrientation secondaryOrientation;
#endif
}
- (id)initWithVertical:(BOOL)vert b:(uiBox *)bb;
- (void)onDestroy;
- (void)removeOurConstraints;
- (void)syncEnableStates:(int)enabled;
- (CGFloat)paddingAmount;
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

#if 0 /* TODO */
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
#endif
	}
	return self;
}

- (void)onDestroy
{
	boxChild *bc;

	[self removeOurConstraints];

	for (bc in self->children) {
		uiControlSetParent(bc.c, NULL);
		uiDarwinControlSetSuperview(uiDarwinControl(bc.c), nil);
		uiControlDestroy(bc.c);
	}
	[self->children release];
}

- (void)removeOurConstraints
{
	// TODO
}

- (void)syncEnableStates:(int)enabled
{
	boxChild *bc;

	for (bc in self->children)
		uiDarwinControlSyncEnableState(uiDarwinControl(bc.c), enabled);
}

- (CGFloat)paddingAmount
{
	if (!self->padded)
		return 0.0;
	return 8.0;		// TODO named constant
}

// TODO something about spinbox hugging
- (void)updateConstraints
{
	[super updateConstraints];
}

- (void)append:(uiControl *)c stretchy:(int)stretchy
{
	boxChild *bc;
	NSView *childView;

	bc = [boxChild new];
	bc.c = c;
	bc.stretchy = stretchy;
	childView = [bc view];
#if 0 /* TODO */
	bc.oldHorzHuggingPri = horzHuggingPri(childView);
	bc.oldVertHuggingPri = vertHuggingPri(childView);
#endif

	uiControlSetParent(bc.c, uiControl(self->b));
	uiDarwinControlSetSuperview(uiDarwinControl(bc.c), self);
	uiDarwinControlSyncEnableState(uiDarwinControl(bc.c), uiControlEnabledToUser(uiControl(self->b)));

#if 0 /*TODO */
	// if a control is stretchy, it should not hug in the primary direction
	// otherwise, it should *forcibly* hug
	if (stretchy)
		setHuggingPri(childView, NSLayoutPriorityDefaultLow, self->primaryOrientation);
	else
		// TODO will default high work?
		setHuggingPri(childView, NSLayoutPriorityRequired, self->primaryOrientation);
	// make sure controls don't hug their secondary direction so they fill the width of the view
	setHuggingPri(childView, NSLayoutPriorityDefaultLow, self->secondaryOrientation);
#endif

	[self->children addObject:bc];
	[bc release];		// we don't need the initial reference now

	[self removeOurConstraints];
	[self setNeedsUpdateConstraints:YES];
}

- (void)delete:(uintmax_t)n
{
	boxChild *bc;
	NSView *removedView;

	// TODO separate into a method?
	bc = (boxChild *) [self->children objectAtIndex:n];
	removedView = [bc view];

	uiControlSetParent(bc.c, NULL);
	uiDarwinControlSetSuperview(uiDarwinControl(bc.c), nil);

#if 0 /* TODO */
	setHorzHuggingPri(removedView, bc.oldHorzHuggingPri);
	setVertHuggingPri(removedView, bc.oldVertHuggingPri);
#endif

	[self->children removeObjectAtIndex:n];

	[self removeOurConstraints];
	[self setNeedsUpdateConstraints:YES];
}

- (int)isPadded
{
	return self->padded;
}

- (void)setPadded:(int)p
{
	CGFloat padding;
	NSLayoutConstraint *c;

	self->padded = p;
	padding = [self paddingAmount];
#if 0 /* TODO */
	for (c in self->inBetweens)
		[c setConstant:padding];
#endif
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
	[b->view syncEnableStates:enabled];
}

uiDarwinControlDefaultSetSuperview(uiBox, view)

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
