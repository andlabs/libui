// 11 june 2015
#include "uipriv_darwin.h"

#define VIEW(c) ((NSView *) uiControlHandle((c)))

static void singleViewCommitDestroy(uiControl *c)
{
	[VIEW(c) release];
}

static void singleViewCommitSetParent(uiControl *c, uiControl *parent)
{
	if (parent == NULL) {
		[VIEW(c) removeFromSuperview];
		return;
	}
	[VIEW(parent) addSubview:VIEW(c)];
}

static void singleViewPreferredSize(uiControl *c, uiSizing *d, intmax_t *width, intmax_t *height)
{
	NSView *v;
	NSRect r;

	v = VIEW(c);
	if (![v respondsToSelector:@selector(sizeToFit)])
		complain("uiControl %p does not respond to -sizeToFit; its type needs to provide an implementation of uiControlPreferredSize()", c);
	[v sizeToFit];
	// use alignmentRect here instead of frame because we'll be resizing based on that
	r = [v alignmentRectForFrame:[v frame]];
	*width = (intmax_t) r.size.width;
	*height = (intmax_t) r.size.height;
}

static void singleViewResize(uiControl *c, intmax_t x, intmax_t y, intmax_t width, intmax_t height, uiSizing *d)
{
	NSRect r;

	r.origin.x = x;
	// mac os x coordinate system has (0,0) in the lower-left
	r.origin.y = ([[VIEW(c) superview] bounds].size.height - height) - y;
	r.size.width = width;
	r.size.height = height;
	// this is the size of the alignment rect; the frame can be bigger (see NSButton)
	r = [VIEW(c) frameForAlignmentRect:r];
	[VIEW(c) setFrame:r];
}

static uiSizing *singleViewSizing(uiControl *c)
{
	return uiDarwinNewSizing();
}

static void singleViewCommitShow(uiControl *c)
{
	[VIEW(c) setHidden:NO];
}

static void singleViewCommitHide(uiControl *c)
{
	[VIEW(c) setHidden:YES];
}

static void singleViewCommitEnable(uiControl *c)
{
	NSControl *cc;

	if ([VIEW(c) respondsToSelector:@selector(setEnabled:)]) {
		// use NSControl to avoid compiler warnings
		cc = (NSControl *) VIEW(c);
		[cc setEnabled:YES];
	}
}

static void singleViewCommitDisable(uiControl *c)
{
	NSControl *cc;

	if ([VIEW(c) respondsToSelector:@selector(setEnabled:)]) {
		// use NSControl to avoid compiler warnings
		cc = (NSControl *) VIEW(c);
		[cc setEnabled:NO];
	}
}

static uintptr_t singleViewStartZOrder(uiControl *c)
{
	// we don't need to do anything; Cocoa does it for us
	return 0;
}

static uintptr_t singleViewSetZOrder(uiControl *c, uintptr_t insertAfter)
{
	// we don't need to do anything; Cocoa does it for us
	return 0;
}

static int singleViewHasTabStops(uiControl *c)
{
	complain("singleViewHasTabStops() meaningless on OS X");
	return 0;		// keep compiler happy
}

// called after creating the control's widget
void uiDarwinMakeSingleWidgetControl(uiControl *c, NSView *view)
{
	// we have to retain the view so we can reparent it
	[view retain];

	uiControl(c)->CommitDestroy = singleViewCommitDestroy;
	uiControl(c)->CommitSetParent = singleViewCommitSetParent;
	uiControl(c)->PreferredSize = singleViewPreferredSize;
	uiControl(c)->Resize = singleViewResize;
	uiControl(c)->Sizing = singleViewSizing;
	uiControl(c)->CommitShow = singleViewCommitShow;
	uiControl(c)->CommitHide = singleViewCommitHide;
	uiControl(c)->CommitEnable = singleViewCommitEnable;
	uiControl(c)->CommitDisable = singleViewCommitDisable;
	uiControl(c)->StartZOrder = singleViewStartZOrder;
	uiControl(c)->SetZOrder = singleViewSetZOrder;
	uiControl(c)->HasTabStops = singleViewHasTabStops;
}

void queueResize(uiControl *c)
{
	// TODO
}
