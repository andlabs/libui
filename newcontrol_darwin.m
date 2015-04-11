// 7 april 2015
#include "uipriv_darwin.h"

typedef struct singleView singleView;

struct singleView {
	NSView *view;
	NSScrollView *scrollView;
	NSView *immediate;		// the control that is added to the parent container; either view or scrollView
	uintptr_t parent;
};

static void singleDestroy(uiControl *c)
{
	singleView *s = (singleView *) (c->internal);

	[deletedControlsView addSubview:s->immediate];
}

static uintptr_t singleHandle(uiControl *c)
{
	singleView *s = (singleView *) (c->internal);

	return (uintptr_t) (s->view);
}

static void singleSetParent(uiControl *c, uintptr_t parent)
{
	singleView *s = (singleView *) (c->internal);
	NSView *parentView;

	s->parent = parent;
	parentView = (NSView *) (s->parent);
	[parentView addSubview:s->immediate];
	updateParent(s->parent);
}

static void singleRemoveParent(uiControl *c)
{
	singleView *s = (singleView *) (c->internal);
	uintptr_t oldparent;

	oldparent = s->parent;
	s->parent = 0;
	[s->immediate removeFromSuperview];
	updateParent(oldparent);
}

// also good for NSBox and NSProgressIndicator
static void singlePreferredSize(uiControl *c, uiSizing *d, intmax_t *width, intmax_t *height)
{
	singleView *s = (singleView *) (c->internal);
	NSControl *control;
	NSRect r;

	control = (NSControl *) (s->view);
	[control sizeToFit];
	// use alignmentRect here instead of frame because we'll be resizing based on that
	r = [control alignmentRectForFrame:[control frame]];
	*width = (intmax_t) r.size.width;
	*height = (intmax_t) r.size.height;
}

static void singleResize(uiControl *c, intmax_t x, intmax_t y, intmax_t width, intmax_t height, uiSizing *d)
{
	singleView *s = (singleView *) (c->internal);
	NSRect frame;

	frame.origin.x = x;
	// mac os x coordinate system has (0,0) in the lower-left
	frame.origin.y = ([[s->immediate superview] bounds].size.height - height) - y;
	frame.size.width = width;
	frame.size.height = height;
	frame = [s->immediate frameForAlignmentRect:frame];
	[s->immediate setFrame:frame];
}

uiControl *uiDarwinNewControl(Class class, BOOL inScrollView, BOOL scrollViewHasBorder)
{
	uiControl *c;
	singleView *s;

	s = uiNew(singleView);
	// thanks to autoxr and arwyn in irc.freenode.net/#macdev
	s->view = (NSView *) [[class alloc] initWithFrame:NSZeroRect];
	s->immediate = s->view;

	if (inScrollView) {
		s->scrollView = [[NSScrollView alloc] initWithFrame:NSZeroRect];
		[s->scrollView setDocumentView:s->view];
		[s->scrollView setHasHorizontalScroller:YES];
		[s->scrollView setHasVerticalScroller:YES];
		[s->scrollView setAutohidesScrollers:YES];
		if (scrollViewHasBorder)
			[s->scrollView setBorderType:NSBezelBorder];
		else
			[s->scrollView setBorderType:NSNoBorder];
		s->immediate = (NSView *) (s->scrollView);
	}

	// and keep a reference to s->immediate for when we remove the control from its parent
	[s->immediate retain];

	c = uiNew(uiControl);
	c->internal = s;
	c->destroy = singleDestroy;
	c->handle = singleHandle;
	c->setParent = singleSetParent;
	c->removeParent = singleRemoveParent;
	c->preferredSize = singlePreferredSize;
	c->resize = singleResize;

	return c;
}

BOOL uiDarwinControlFreeWhenAppropriate(uiControl *c, NSView *newSuperview)
{
	singleView *s = (singleView *) (c->internal);

	if (newSuperview == deletedControlsView) {
		[s->immediate release];		// we don't need the reference anymore
		uiFree(s);
		uiFree(c);
		return YES;
	}
	return NO;
}
