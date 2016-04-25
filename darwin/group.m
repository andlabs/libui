// 14 august 2015
#import "uipriv_darwin.h"

struct uiGroup {
	uiDarwinControl c;
	NSBox *box;
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

static void uiBoxSyncEnableState(uiControl *c, int enabled)
{
	uiGroup *g = uiGroup(c);

	if (g->child != NULL)
		uiControlSyncEnableState(g->child);
}

uiDarwinControlDefaultSetSuperview(uiGroup, box)

static void groupRelayout(uiDarwinControl *c)
{
	uiGroup *g = uiGroup(c);
	uiDarwinControl *cc;
	NSView *childView;

	if (g->child == NULL)
		return;
	[g->box removeConstraints:[g->box constraints]];
	cc = uiDarwinControl(g->child);
	childView = (NSView *) uiControlHandle(g->child);
	// first relayout the child
	(*(cc->Relayout))(cc);
	// now relayout ourselves
	// see below on using the content view
	layoutSingleView(g->box, childView, g->margined);
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
	if (g->child != NULL) {
		childView = (NSView *) uiControlHandle(g->child);
		uiControlSetParent(g->child, uiControl(g));
		// we have to add controls to the box itself NOT the content view
		// otherwise, things get really glitchy
		// we also need to call -sizeToFit, but we'll do that in the relayout that's triggered below (see above)
		uiControlSetSuperview(c, g->box);
		uiControlSyncEnableState(c, uiControlEnabledToUser(uiControl(g)));
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

	g->box = [[NSBox alloc] initWithFrame:NSZeroRect];
	[g->box setTitle:toNSString(title)];
	[g->box setBoxType:NSBoxPrimary];
	[g->box setBorderType:NSLineBorder];
	[g->box setTransparent:NO];
	[g->box setTitlePosition:NSAtTop];
	// we can't use uiDarwinSetControlFont() because the selector is different
	[g->box setTitleFont:[NSFont systemFontOfSize:[NSFont systemFontSizeForControlSize:NSSmallControlSize]]];

	return g;
}
