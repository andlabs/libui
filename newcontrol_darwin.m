// 7 april 2015
#include "uipriv_darwin.h"

typedef struct uiSingleViewControl uiSingleViewControl;

struct uiSingleViewControl {
	uiControl control;
	NSView *view;
	NSScrollView *scrollView;
	NSView *immediate;		// the control that is added to the parent container; either view or scrollView
	uintptr_t parent;
};

#define S(c) ((uiSingleViewControl *) (c))

// TODO this will need to change if we want to provide removal
static void singleDestroy(uiControl *c)
{
	[S(c)->view removeFromSuperview];
}

static uintptr_t singleHandle(uiControl *c)
{
	return (uintptr_t) (S(c)->view);
}

static void singleSetParent(uiControl *c, uintptr_t parent)
{
	uiSingleViewControl *s = S(c);
	NSView *parentView;

	s->parent = parent;
	parentView = (NSView *) (s->parent);
	[parentView addSubview:s->immediate];
	updateParent(s->parent);
}

static void singleRemoveParent(uiControl *c)
{
	uiSingleViewControl *s = S(c);
	uintptr_t oldparent;

	oldparent = s->parent;
	s->parent = 0;
	[s->immediate removeFromSuperview];
	updateParent(oldparent);
}

// also good for NSBox and NSProgressIndicator
static uiSize singlePreferredSize(uiControl *c, uiSizing *d)
{
	uiSize size;
	NSControl *control;
	NSRect r;

	control = (NSControl *) (S(c)->view);
	[control sizeToFit];
	// use alignmentRect here instead of frame because we'll be resizing based on that
	r = [control alignmentRectForFrame:[control frame]];
	size.width = (intmax_t) r.size.width;
	size.height = (intmax_t) r.size.height;
	return size;
}

static void singleResize(uiControl *c, intmax_t x, intmax_t y, intmax_t width, intmax_t height, uiSizing *d)
{
	NSRect frame;

	frame.origin.x = x;
	// mac os x coordinate system has (0,0) in the lower-left
	frame.origin.y = ([[S(c)->immediate superview] bounds].size.height - height) - y;
	frame.size.width = width;
	frame.size.height = height;
	frame = [S(c)->immediate frameForAlignmentRect:frame];
	[S(c)->immediate setFrame:frame];
}

uiControl *uiDarwinNewControl(Class class, BOOL inScrollView, BOOL scrollViewHasBorder)
{
	uiSingleViewControl *c;

	c = uiNew(uiSingleViewControl);
	// thanks to autoxr and arwyn in irc.freenode.net/#macdev
	c->view = (NSView *) [[class alloc] initWithFrame:NSZeroRect];
	c->immediate = c->view;

	if (inScrollView) {
		c->scrollView = [[NSScrollView alloc] initWithFrame:NSZeroRect];
		[c->scrollView setDocumentView:c->view];
		[c->scrollView setHasHorizontalScroller:YES];
		[c->scrollView setHasVerticalScroller:YES];
		[c->scrollView setAutohidesScrollers:YES];
		if (scrollViewHasBorder)
			[c->scrollView setBorderType:NSBezelBorder];
		else
			[c->scrollView setBorderType:NSNoBorder];
		c->immediate = (NSView *) (c->scrollView);
	}

	c->control.destroy = singleDestroy;
	c->control.handle = singleHandle;
	c->control.setParent = singleSetParent;
	c->control.removeParent = singleRemoveParent;
	c->control.preferredSize = singlePreferredSize;
	c->control.resize = singleResize;

	return (uiControl *) c;
}

BOOL uiDarwinControlFreeWhenAppropriate(uiControl *c, NSView *newSuperview)
{
	if (newSuperview == nil) {
		uiFree(c);
		return YES;
	}
	return NO;
}
