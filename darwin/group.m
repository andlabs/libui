// 14 august 2015
#import "uipriv_darwin.h"

struct uiGroup {
	uiDarwinControl c;
	NSBox *box;
	uiControl *child;
	int margined;
};

static void onDestroy(uiGroup *);

uiDarwinDefineControlWithOnDestroy(
	uiGroup,								// type name
	box,									// handle
	onDestroy(this);						// on destroy
)

static void onDestroy(uiGroup *g)
{
	if (g->child != NULL) {
		uiControlSetParent(g->child, NULL);
		uiControlDestroy(g->child);
	}
}

static void groupContainerUpdateState(uiControl *c)
{
	uiGroup *g = uiGroup(c);

	if (g->child != NULL)
		controlUpdateState(g->child);
}

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
		[g->box addSubview:childView];
		uiDarwinControlTriggerRelayout(uiDarwinControl(g));
	}
}

int uiGroupMargined(uiGroup *g)
{
	return g->margined;
}

void uiGroupSetMargined(uiGroup *g, int margined)
{
	g->margined = margined;
	if (g->child != NULL)
		uiDarwinControlTriggerRelayout(uiDarwinControl(g));
}

uiGroup *uiNewGroup(const char *title)
{
	uiGroup *g;

	g = (uiGroup *) uiNewControl(uiGroupType());

	g->box = [[NSBox alloc] initWithFrame:NSZeroRect];
	[g->box setTitle:toNSString(title)];
	[g->box setBoxType:NSBoxPrimary];
	[g->box setBorderType:NSLineBorder];
	[g->box setTransparent:NO];
	[g->box setTitlePosition:NSAtTop];
	// we can't use uiDarwinSetControlFont() because the selector is different
	[g->box setTitleFont:[NSFont systemFontOfSize:[NSFont systemFontSizeForControlSize:NSSmallControlSize]]];

	uiDarwinFinishNewControl(g, uiGroup);
	uiControl(g)->ContainerUpdateState = groupContainerUpdateState;
	uiDarwinControl(g)->Relayout = groupRelayout;

	return g;
}
