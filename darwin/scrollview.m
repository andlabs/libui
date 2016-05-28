// 27 may 2016
#include "uipriv_darwin.h"

// TODO scrolled drawing test may have a random size outside the area size

struct scrollViewData {
	NSLayoutConstraint *documentLeading;
	NSLayoutConstraint *documentTop;
	BOOL hscroll;
	NSLayoutConstraint *documentTrailing;
	BOOL vscroll;
	NSLayoutConstraint *documentBottom;
};

NSScrollView *mkScrollView(struct scrollViewCreateParams *p, struct scrollViewData **dout)
{
	NSScrollView *sv;
	NSBorderType border;
	struct scrollViewData *d;

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

	[p->DocumentView setTranslatesAutoresizingMaskIntoConstraints:NO];
	[sv setDocumentView:p->DocumentView];
	d = uiNew(struct scrollViewData);
	scrollViewSetScrolling(sv, d, p->HScroll, p->VScroll);

	*dout = d;
	return sv;
}

static void scrollViewConstraintsRemove(NSScrollView *sv, struct scrollViewData *d)
{
	if (d->documentLeading != nil) {
		[sv removeConstraint:d->documentLeading];
		[d->documentLeading release];
		d->documentLeading = nil;
	}
	if (d->documentTop != nil) {
		[sv removeConstraint:d->documentTop];
		[d->documentTop release];
		d->documentTop = nil;
	}
	if (d->documentTrailing != nil) {
		[sv removeConstraint:d->documentTrailing];
		[d->documentTrailing release];
		d->documentTrailing = nil;
	}
	if (d->documentBottom != nil) {
		[sv removeConstraint:d->documentBottom];
		[d->documentBottom release];
		d->documentBottom = nil;
	}
}

// based on http://blog.bjhomer.com/2014/08/nsscrollview-and-autolayout.html because (as pointed out there) Apple's official guide is really only for iOS
void scrollViewSetScrolling(NSScrollView *sv, struct scrollViewData *d, BOOL hscroll, BOOL vscroll)
{
	NSView *cv, *dv;
	NSLayoutRelation rel;

	scrollViewConstraintsRemove(sv, d);
	cv = [sv contentView];
	dv = [sv documentView];

	d->documentLeading = mkConstraint(dv, NSLayoutAttributeLeading,
		NSLayoutRelationEqual,
		cv, NSLayoutAttributeLeading,
		1, 0,
		@"NSScrollView document leading constraint");
	[sv addConstraint:d->documentLeading];
	[d->documentLeading retain];

	d->documentTop = mkConstraint(dv, NSLayoutAttributeTop,
		NSLayoutRelationEqual,
		cv, NSLayoutAttributeTop,
		1, 0,
		@"NSScrollView document top constraint");
	[sv addConstraint:d->documentTop];
	[d->documentTop retain];

	d->hscroll = hscroll;
	[sv setHasHorizontalScroller:d->hscroll];
	rel = NSLayoutRelationGreaterThanOrEqual;
	if (!d->hscroll)
		rel = NSLayoutRelationEqual;
	d->documentTrailing = mkConstraint(dv, NSLayoutAttributeTrailing,
		rel,
		cv, NSLayoutAttributeTrailing,
		1, 0,
		@"NSScrollView document trailing constraint");
	[sv addConstraint:d->documentTrailing];
	[d->documentTrailing retain];

	d->vscroll = vscroll;
	[sv setHasVerticalScroller:d->vscroll];
	rel = NSLayoutRelationGreaterThanOrEqual;
	if (!d->vscroll)
		rel = NSLayoutRelationEqual;
	d->documentBottom = mkConstraint(dv, NSLayoutAttributeBottom,
		rel,
		cv, NSLayoutAttributeBottom,
		1, 0,
		@"NSScrollView document bottom constraint");
	[sv addConstraint:d->documentBottom];
	[d->documentBottom retain];
}

void scrollViewFreeData(NSScrollView *sv, struct scrollViewData *d)
{
	scrollViewConstraintsRemove(sv, d);
	uiFree(d);
}
