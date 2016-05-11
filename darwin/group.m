// 14 august 2015
#import "uipriv_darwin.h"

// TODO test empty groups

struct uiGroup {
	uiDarwinControl c;
	libuiIntrinsicBox *box;
	uiControl *child;
	int margined;
	struct singleChildConstraints constraints;
};

static void removeConstraints(uiGroup *g)
{
	singleChildConstraintsRemove(&(g->constraints), g->box);
}

static void uiGroupDestroy(uiControl *c)
{
	uiGroup *g = uiGroup(c);

	removeConstraints(g);
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

static void groupRelayout(uiGroup *g)
{
	NSView *childView;

	removeConstraints(g);
	if (g->child == NULL)
		return;
	childView = (NSView *) uiControlHandle(g->child);
	singleChildConstraintsEstablish(&(g->constraints),
		g->box, childView,
		uiDarwinControlHugsTrailingEdge(uiDarwinControl(g->child)),
		uiDarwinControlHugsBottom(uiDarwinControl(g->child)),
		g->margined,
		@"uiGroup");
}

uiDarwinControlDefaultHugsTrailingEdge(uiGroup, box)
uiDarwinControlDefaultHugsBottom(uiGroup, box)

static void uiWindowChildEdgeHuggingChanged(uiDarwinControl *c)
{
	uiGroup *g = uiGroup(c);

	groupRelayout(g);
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
		removeConstraints(g);
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
	singleChildConstraintsSetMargined(&(w->constraints), w->margined);
	// TODO issue a relayout command?
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
