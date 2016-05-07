// 14 august 2015
#import "uipriv_darwin.h"

// TODO test empty groups

// if there is an intrinsic content size on the NSBox, Auto Layout won't bother checking the box's children and force the box to be the intrinsic size if its hugging priority is Required; we don't want that
// TODO don't truncate the label?
@interface libuiIntrinsicBox : NSBox {
	BOOL libui_hasChild;
}
- (void)libui_setHasChild:(BOOL)h;
@end

@implementation libuiIntrinsicBox

- (void)libui_setHasChild:(BOOL)h
{
	self->libui_hasChild = h;
	[self invalidateIntrinsicContentSize];
}

- (NSSize)intrinsicContentSize
{
	if (!self->libui_hasChild)
		return [super intrinsicContentSize];
	return NSMakeSize(NSViewNoIntrinsicMetric, NSViewNoIntrinsicMetric);
}

@end

struct uiGroup {
	uiDarwinControl c;
	libuiIntrinsicBox *box;
	uiControl *child;
	int margined;
};

static void uiGroupDestroy(uiControl *c)
{
	uiGroup *g = uiGroup(c);

	if (g->child != NULL) {
		uiControlSetParent(g->child, NULL);
		uiControlDestroy(g->child);
	}
	[g->box release];
	uiFreeControl(uiControl(g));
}

uiDarwinControlDefaultHandle(uiGroup, box)
uiDarwinControlDefaultParent(uiGroup, box)
uiDarwinControlDefaultSetParent(uiGroup, box)
uiDarwinControlDefaultToplevel(uiGroup, box)
uiDarwinControlDefaultVisible(uiGroup, box)
uiDarwinControlDefaultShow(uiGroup, box)
uiDarwinControlDefaultHide(uiGroup, box)
uiDarwinControlDefaultEnabled(uiGroup, box)
uiDarwinControlDefaultEnable(uiGroup, box)
uiDarwinControlDefaultDisable(uiGroup, box)

static void uiGroupSyncEnableState(uiDarwinControl *c, int enabled)
{
	uiGroup *g = uiGroup(c);

	if (uiDarwinShouldStopSyncEnableState(uiDarwinControl(g), enabled))
		return;
	if (g->child != NULL)
		uiDarwinControlSyncEnableState(uiDarwinControl(g->child), enabled);
}

uiDarwinControlDefaultSetSuperview(uiGroup, box)

static BOOL uiGroupChildrenShouldAllowSpaceAtTrailingEdge(uiDarwinControl *c)
{
	// always allow growth
	// TODO actually these should probably only allow growth if the parent does...
	return YES;
}

static BOOL uiGroupChildrenShouldAllowSpaceAtBottom(uiDarwinControl *c)
{
	// always allow growth
	return YES;
}

static void groupRelayout(uiGroup *g)
{
	uiDarwinControl *cc;
	NSView *childView;

	if (g->child == NULL)
		return;
	cc = uiDarwinControl(g->child);
	childView = (NSView *) uiControlHandle(g->child);
	// first relayout the child
//TODO	(*(cc->Relayout))(cc);
	layoutSingleView([g->box contentView], childView, g->margined, @"uiGroup");
	// we need to explicitly tell the NSBox to recompute its own size based on the new content layout
	[g->box sizeToFit];
}

char *uiGroupTitle(uiGroup *g)
{
	return uiDarwinNSStringToText([g->box title]);
}

void uiGroupSetTitle(uiGroup *g, const char *title)
{
	[g->box setTitle:toNSString(title)];
	// changing the text might necessitate a change in the groupbox's size
	uiDarwinControlTriggerRelayout(uiDarwinControl(g));
}

void uiGroupSetChild(uiGroup *g, uiControl *child)
{
	NSView *childView;

	if (g->child != NULL) {
		childView = (NSView *) uiControlHandle(g->child);
		[childView removeFromSuperview];
		uiControlSetParent(g->child, NULL);
	}
	g->child = child;
	[g->box libui_setHasChild:(g->child != NULL)];
	if (g->child != NULL) {
		childView = (NSView *) uiControlHandle(g->child);
		uiControlSetParent(g->child, uiControl(g));
		uiDarwinControlSetSuperview(uiDarwinControl(g->child), [g->box contentView]);
		uiDarwinControlSyncEnableState(uiDarwinControl(g->child), uiControlEnabledToUser(uiControl(g)));
	}
	groupRelayout(g);
}

int uiGroupMargined(uiGroup *g)
{
	return g->margined;
}

void uiGroupSetMargined(uiGroup *g, int margined)
{
	g->margined = margined;
	groupRelayout(g);
}

uiGroup *uiNewGroup(const char *title)
{
	uiGroup *g;

	uiDarwinNewControl(uiGroup, g);

	g->box = [[libuiIntrinsicBox alloc] initWithFrame:NSZeroRect];
	[g->box setTitle:toNSString(title)];
	[g->box setBoxType:NSBoxPrimary];
	[g->box setBorderType:NSLineBorder];
	[g->box setTransparent:NO];
	[g->box setTitlePosition:NSAtTop];
	// we can't use uiDarwinSetControlFont() because the selector is different
	[g->box setTitleFont:[NSFont systemFontOfSize:[NSFont systemFontSizeForControlSize:NSSmallControlSize]]];
	[g->box libui_setHasChild:NO];

	return g;
}
