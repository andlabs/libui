// 15 august 2015
#import "uipriv_darwin.h"

// TODO hiding all stretchy controls still hugs trailing edge

@interface fixedChild : NSObject
@property uiControl *c;
@property int x;
@property int y;
- (NSView *)view;
@end

@interface fixedView : NSView {
	uiFixed *b;
	NSMutableArray *children;
}
- (id)initFixed:(uiFixed *)bb;
- (bool)isFlipped;
- (void)onDestroy;
- (void)syncEnableStates:(int)enabled;
- (void)append:(uiControl *)c x:(int)x y:(int)y;
- (void)move:(uiControl *)c x:(int)x y:(int)y;
- (void)reloadPositions;
@end

struct uiFixed {
	uiDarwinControl c;
	fixedView *view;
};

@implementation fixedChild

- (NSView *)view
{
	return (NSView *) uiControlHandle(self.c);
}

@end

@implementation fixedView

- (id)initFixed:(uiFixed *)bb
{
	self = [super initWithFrame:NSZeroRect];
	self.autoresizingMask = NSViewWidthSizable | NSViewHeightSizable;
	if (self != nil) {
		// the weird names vert and bb are to shut the compiler up about shadowing because implicit this/self is stupid
		self->b = bb;
		self->children = [NSMutableArray new];
	}

	return self;
}

- (bool)isFlipped
{
	return YES;
}

- (void)onDestroy
{
	fixedChild *bc;

	for (bc in self->children) {
		uiControlSetParent(bc.c, NULL);
		uiDarwinControlSetSuperview(uiDarwinControl(bc.c), nil);
		uiControlDestroy(bc.c);
	}
	[self->children release];
}

- (void)syncEnableStates:(int)enabled
{
	fixedChild *bc;

	for (bc in self->children)
		uiDarwinControlSyncEnableState(uiDarwinControl(bc.c), enabled);
}

- (void)append:(uiControl *)c x:(int)x y:(int)y
{
	fixedChild *bc;

	bc = [fixedChild new];
	bc.c = c;
	bc.x = x;
	bc.y = y;

	uiControlSetParent(bc.c, uiControl(self->b));
	uiDarwinControlSetSuperview(uiDarwinControl(bc.c), self);
	uiDarwinControlSyncEnableState(uiDarwinControl(bc.c), uiControlEnabledToUser(uiControl(self->b)));

	[self->children addObject:bc];

	[self reloadPositions];

	[bc release];		// we don't need the initial reference now
}

- (void)move:(uiControl *)c x:(int)x y:(int)y
{
	fixedChild *fc;

	for (fc in self->children) {
		if (fc.c == c) {
			fc.x = x;
			fc.y = y;
			[self reloadPositions];
			return;
		}
	}
}

- (void)reloadPositions
{
	fixedChild *fc;
	CGPoint pos;
	NSView *view;

	for (fc in self->children) {
		if (!uiControlVisible(fc.c))
			continue;
		pos = CGPointMake(fc.x, fc.y);
		view = [fc view];
		[view setFrame:(CGRect){.origin = pos, .size=view.frame.size}];
	}
}

@end

static void uiFixedDestroy(uiControl *c)
{
	uiFixed *b = uiFixed(c);

	[b->view onDestroy];
	[b->view release];
	uiFreeControl(uiControl(b));
}

uiDarwinControlDefaultHandle(uiFixed, view)
uiDarwinControlDefaultParent(uiFixed, view)
uiDarwinControlDefaultSetParent(uiFixed, view)
uiDarwinControlDefaultToplevel(uiFixed, view)
uiDarwinControlDefaultVisible(uiFixed, view)
uiDarwinControlDefaultShow(uiFixed, view)
uiDarwinControlDefaultHide(uiFixed, view)
uiDarwinControlDefaultEnabled(uiFixed, view)
uiDarwinControlDefaultEnable(uiFixed, view)
uiDarwinControlDefaultDisable(uiFixed, view)

static void uiFixedSyncEnableState(uiDarwinControl *c, int enabled)
{
	uiFixed *b = uiFixed(c);

	if (uiDarwinShouldStopSyncEnableState(uiDarwinControl(b), enabled))
		return;
	[b->view syncEnableStates:enabled];
}

uiDarwinControlDefaultSetSuperview(uiFixed, view)

uiDarwinControlDefaultHuggingPriority(uiFixed, view)
uiDarwinControlDefaultSetHuggingPriority(uiFixed, view)
uiDarwinControlDefaultHugsTrailingEdge(uiFixed, view)
uiDarwinControlDefaultHugsBottom(uiFixed, view)
uiDarwinControlDefaultChildEdgeHuggingChanged(uiFixed, view)

static void uiFixedChildVisibilityChanged(uiDarwinControl *c)
{
	uiFixed *b = uiFixed(c);

	[b->view reloadPositions];
}

void uiFixedAppend(uiFixed *b, uiControl *c, int x, int y)
{
	// LONGTERM on other platforms
	// or at leat allow this and implicitly turn it into a spacer
	if (c == NULL)
		userbug("You cannot add NULL to a uiFixed.");
	[b->view append:c x:x y:y];
}

void uiFixedMove(uiFixed *b, uiControl *c, int x, int y)
{
	// LONGTERM on other platforms
	// or at leat allow this and implicitly turn it into a spacer
	if (c == NULL)
		userbug("You cannot move NULL to a uiFixed.");
	[b->view move:c x:x y:y];
}

uiFixed *uiNewFixed(void)
{
	uiFixed *f;
	uiDarwinNewControl(uiFixed, f);

	f->view = [[fixedView alloc] initFixed:f];

	return f;
}