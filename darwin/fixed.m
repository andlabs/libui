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
- (id)initFixed:(uiBox *)bb;
- (void)onDestroy;
- (void)syncEnableStates:(int)enabled;
- (void)append:(uiControl *)c x:(int)x y:(int)y;
- (void)delete:(int)n;
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

- (id)initFixed:(uiBox *)bb
{
	self = [super initWithFrame:NSZeroRect];
	if (self != nil) {
		// the weird names vert and bb are to shut the compiler up about shadowing because implicit this/self is stupid
		self->b = bb;
		self->children = [NSMutableArray new];
	}
	return self;
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
	NSLayoutPriority priority;

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

- (void)reloadPositions
{
	fixedChild *fc;
	CGPoint pos;

	for (fc in self->children) {
		if (!uiControlVisible(fc.c))
			continue;
		pos = CGPointMake(fc.x, fc.y);
		[self setFrame:(CGRect){.origin = position, .size=self.frame.size}];
	}
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

uiDarwinControlDefaultHuggingPriority(uiBox, view)
uiDarwinControlDefaultSetHuggingPriority(uiBox, view)

static void uiFixedChildVisibilityChanged(uiDarwinControl *c)
{
	uiBox *b = uiBox(c);

	[b->view establishOurConstraints];
}

void uiFixedAppend(uiFixed *b, uiControl *c, int x, int y)
{
	// LONGTERM on other platforms
	// or at leat allow this and implicitly turn it into a spacer
	if (c == NULL)
		userbug("You cannot add NULL to a uiFixed.");
	[b->view append:c x:x y:y];
}

uiFixed *uiNewFixed(void)
{
	uiFixed *f;
	uiDarwinNewControl(uiFixed, f);

	f->view = [[fixedView alloc] initFixed:f];

	return f;
}