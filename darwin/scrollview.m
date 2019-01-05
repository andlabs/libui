// 27 may 2016
#include "uipriv_darwin.h"

// see http://stackoverflow.com/questions/37979445/how-do-i-properly-set-up-a-scrolling-nstableview-using-auto-layout-what-ive-tr for why we don't use auto layout
// TODO do the same with uiGroup and uiTab?

struct uiprivScrollViewData {
	BOOL hscroll;
	BOOL vscroll;
};

NSScrollView *uiprivMkScrollView(uiprivScrollViewCreateParams *p, uiprivScrollViewData **dout)
{
	NSScrollView *sv;
	NSBorderType border;
	uiprivScrollViewData *d;

	sv = [[NSScrollView alloc] initWithFrame:NSZeroRect];
	if (p->BackgroundColor != nil)
		[sv setBackgroundColor:p->BackgroundColor];
	[sv setDrawsBackground:p->DrawsBackground];
	border = NSNoBorder;
	if (p->Bordered)
		border = NSBezelBorder;
	// document view seems to set the cursor properly
	[sv setBorderType:border];
	[sv setAutohidesScrollers:YES];
	[sv setHasHorizontalRuler:NO];
	[sv setHasVerticalRuler:NO];
	[sv setRulersVisible:NO];
	[sv setScrollerKnobStyle:NSScrollerKnobStyleDefault];
	// the scroller style is documented as being set by default for us
	// LONGTERM verify line and page for programmatically created NSTableView
	[sv setScrollsDynamically:YES];
	[sv setFindBarPosition:NSScrollViewFindBarPositionAboveContent];
	[sv setUsesPredominantAxisScrolling:NO];
	[sv setHorizontalScrollElasticity:NSScrollElasticityAutomatic];
	[sv setVerticalScrollElasticity:NSScrollElasticityAutomatic];
	[sv setAllowsMagnification:NO];

	[sv setDocumentView:p->DocumentView];
	d = uiprivNew(uiprivScrollViewData);
	uiprivScrollViewSetScrolling(sv, d, p->HScroll, p->VScroll);

	*dout = d;
	return sv;
}

// based on http://blog.bjhomer.com/2014/08/nsscrollview-and-autolayout.html because (as pointed out there) Apple's official guide is really only for iOS
void uiprivScrollViewSetScrolling(NSScrollView *sv, uiprivScrollViewData *d, BOOL hscroll, BOOL vscroll)
{
	d->hscroll = hscroll;
	[sv setHasHorizontalScroller:d->hscroll];
	d->vscroll = vscroll;
	[sv setHasVerticalScroller:d->vscroll];
}

void uiprivScrollViewFreeData(NSScrollView *sv, uiprivScrollViewData *d)
{
	uiprivFree(d);
}
