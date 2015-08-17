// 14 august 2015
#import "uipriv_darwin.h"

struct uiGroup {
	uiDarwinControl c;
	NSBox *box;
	uiControl *child;
	int margined;
};

uiDarwinDefineControlWithOnDestroy(
	uiGroup,								// type name
	uiGroupType,							// type function
	box,									// handle
	/* TODO */;							// on destroy
)

// TODO group container update

char *uiGroupTitle(uiGroup *g)
{
	return PUT_CODE_HERE;
}

void uiGroupSetTitle(uiGroup *g, const char *text)
{
	// TODO
	// changing the text might necessitate a change in the groupbox's size
//TODO	uiControlQueueResize(uiControl(g));
}

void uiGroupSetChild(uiGroup *g, uiControl *child)
{
/* TODO
	if (g->child != NULL)
		uiControlSetParent(g->child, NULL);
	g->child = child;
	if (g->child != NULL) {
		uiControlSetParent(g->child, uiControl(g));
//TODO		uiControlQueueResize(g->child);
	}
*/
}

int uiGroupMargined(uiGroup *g)
{
	return g->margined;
}

void uiGroupSetMargined(uiGroup *g, int margined)
{
	g->margined = margined;
	// TODO
//TODO	uiControlQueueResize(uiControl(g));
}

uiGroup *uiNewGroup(const char *title)
{
	uiGroup *g;

	g = (uiGroup *) uiNewControl(uiGroupType());

	g->box = [[NSBox alloc] initWithFrame:NSZeroRect];
	// TODO title
	[g->box setBoxType:NSBoxPrimary];
//TODO	[g->box setBorderType:TODO];
	[g->box setTransparent:NO];
	[g->box setTitlePosition:NSAtTop];
	uiDarwinSetControlFont(g->box, NSSmallControlSize);

	uiDarwinFinishNewControl(g, uiGroup);

	return g;
}
