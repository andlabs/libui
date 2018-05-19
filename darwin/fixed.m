// 19 may 2018
#import "uipriv_darwin.h"

@interface uiprivFixedChild : NSObject
@property uiControl *c;
@property int x;
@property int y;
- (NSView *)view;
@end

@interface uiprivFixedView : NSView {
	uiFixed *b;
	NSMutableArray *children;
}
- (id)initFixed:(uiFixed *)bb;
- (void)onDestroy;
- (void)syncEnableStates:(int)enabled;
- (void)append:(uiControl *)c x:(int)x y:(int)y;
- (void)move:(uiControl *)c x:(int)x y:(int)y;
- (void)reloadPositions;
- (void)size:(uiControl *)c width:(int *)width height:(int *)height;
- (void)setSize:(uiControl *)c width:(int)width height:(int)height;
@end

struct uiFixed {
	uiDarwinControl c;
	uiprivFixedView *view;
};

@implementation uiprivFixedChild

- (NSView *)view
{
	return (NSView *) uiControlHandle(self.c);
}

@end

@implementation uiprivFixedView

- (id)initFixed:(uiFixed *)bb
{
	self = [super initWithFrame:NSZeroRect];
	if (self != nil) {
		self->b = bb;
		self->children = [NSMutableArray new];
	}

	return self;
}

- (BOOL)isFlipped
{
	return YES;
}

- (void)onDestroy
{
	uiprivFixedChild *bc;

	for (bc in self->children) {
		uiControlSetParent(bc.c, NULL);
		uiDarwinControlSetSuperview(uiDarwinControl(bc.c), nil);
		uiControlDestroy(bc.c);
	}
	[self->children release];
}

- (void)syncEnableStates:(int)enabled
{
	uiprivFixedChild *bc;

	for (bc in self->children)
		uiDarwinControlSyncEnableState(uiDarwinControl(bc.c), enabled);
}

- (void)append:(uiControl *)c x:(int)x y:(int)y
{
	uiprivFixedChild *bc;

	bc = [uiprivFixedChild new];
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
	uiprivFixedChild *fc;

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
	uiprivFixedChild *fc;
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

- (void)size:(uiControl *)c width:(int *)width height:(int *)height
{
	uiprivFixedChild *fc;
	NSView *view;

	for (fc in self->children) {
		if (!uiControlVisible(fc.c))
			continue;
		if (fc.c == c) {
			view = [fc view];
			*width = view.frame.size.width;
			*height = view.frame.size.height;
		}
	}
}

- (void)setSize:(uiControl *)c width:(int)width height:(int)height
{
	uiprivFixedChild *fc;
	NSView *view;

	for (fc in self->children) {
		if (!uiControlVisible(fc.c))
			continue;
		if (fc.c == c) {
			view = [fc view];
			
			CGRect bounds = view.bounds;
			bounds.size.width = width;
			bounds.size.height = height;
			view.bounds = bounds;

			CGRect frame = view.frame;
			frame.size.width = width;
			frame.size.height = height;
			view.frame = frame;
		}
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
		uiprivUserBug("You cannot add NULL to a uiFixed.");
	[b->view append:c x:x y:y];
}

void uiFixedMove(uiFixed *b, uiControl *c, int x, int y)
{
	// LONGTERM on other platforms
	// or at leat allow this and implicitly turn it into a spacer
	if (c == NULL)
		uiprivUserBug("You cannot move NULL to a uiFixed.");
	[b->view move:c x:x y:y];
}

void uiFixedSize(uiFixed *b, uiControl *c, int *width, int *height)
{
	[b->view size:c width:width height:height];
}

void uiFixedSetSize(uiFixed *b, uiControl *c, int width, int height)
{
	[b->view setSize:c width:width height:height];
}

uiFixed *uiNewFixed(void)
{
	uiFixed *f;
	uiDarwinNewControl(uiFixed, f);

	f->view = [[uiprivFixedView alloc] initFixed:f];

	return f;
}
