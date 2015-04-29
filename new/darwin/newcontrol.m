// 7 april 2015
#include "uipriv_darwin.h"

typedef struct singleView singleView;

struct singleView {
	NSView *view;
	NSScrollView *scrollView;
	NSView *immediate;		// the control that is added to the parent container; either view or scrollView
	uiContainer *parent;
	int hidden;
	void (*onDestroy)(void *);
	void *onDestroyData;
};

static void singleDestroy(uiControl *c)
{
	singleView *s = (singleView *) (c->Internal);

	if (s->parent != NULL)
		complain("attempt to destroy a uiControl at %p while it still has a parent", c);
	[s->immediate retain];		// to keep alive when removing
	(*(s->onDestroy))(s->onDestroyData);
	[s->immediate release];
}

static uintptr_t singleHandle(uiControl *c)
{
	singleView *s = (singleView *) (c->Internal);

	return (uintptr_t) (s->view);
}

// TODO update refcounting here and in the GTK+ port
static void singleSetParent(uiControl *c, uiContainer *parent)
{
	singleView *s = (singleView *) (c->Internal);
	NSView *parentView;
	uiContainer *oldparent;

	oldparent = s->parent;
	s->parent = parent;
	if (oldparent != NULL)
		[s->immediate removeFromSuperview];
	if (s->parent != NULL) {
		parentView = (NSView *) uiControlHandle(uiControl(s->parent));
		[parentView addSubview:s->immediate];
	}
	if (oldparent != NULL)
		uiContainerUpdate(oldparent);
	if (s->parent != NULL)
		uiContainerUpdate(s->parent);
}

// also good for NSBox and NSProgressIndicator
static void singlePreferredSize(uiControl *c, uiSizing *d, intmax_t *width, intmax_t *height)
{
	singleView *s = (singleView *) (c->Internal);
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
	singleView *s = (singleView *) (c->Internal);
	NSRect frame;

	frame.origin.x = x;
	// mac os x coordinate system has (0,0) in the lower-left
	frame.origin.y = ([[s->immediate superview] bounds].size.height - height) - y;
	frame.size.width = width;
	frame.size.height = height;
	frame = [s->immediate frameForAlignmentRect:frame];
	[s->immediate setFrame:frame];
}

static int singleVisible(uiControl *c)
{
	singleView *s = (singleView *) (c->Internal);

	return !s->hidden;
}

static void singleShow(uiControl *c)
{
	singleView *s = (singleView *) (c->Internal);

	[s->immediate setHidden:NO];
	if (s->parent != NULL)
		uiContainerUpdate(s->parent);
	s->hidden = 0;
}

static void singleHide(uiControl *c)
{
	singleView *s = (singleView *) (c->Internal);

	[s->immediate setHidden:YES];
	if (s->parent != NULL)
		uiContainerUpdate(s->parent);
	s->hidden = 1;
}

static void singleEnable(uiControl *c)
{
	singleView *s = (singleView *) (c->Internal);

	if ([s->view respondsToSelector:@selector(setEnabled:)])
		[((NSControl *) (s->view)) setEnabled:YES];
}

static void singleDisable(uiControl *c)
{
	singleView *s = (singleView *) (c->Internal);

	if ([s->view respondsToSelector:@selector(setEnabled:)])
		[((NSControl *) (s->view)) setEnabled:NO];
}

void uiDarwinNewControl(uiControl *c, Class class, BOOL inScrollView, BOOL scrollViewHasBorder, void (*onDestroy)(void *), void *onDestroyData)
{
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

	s->onDestroy = onDestroy;
	s->onDestroyData = onDestroyData;

	// and keep a reference to s->immediate for when we remove the control from its parent
	[s->immediate retain];

	// TODO update all of these to say uiControl(c)
	c->Internal = s;
	c->Destroy = singleDestroy;
	c->Handle = singleHandle;
	c->SetParent = singleSetParent;
	c->PreferredSize = singlePreferredSize;
	c->Resize = singleResize;
	c->Visible = singleVisible;
	c->Show = singleShow;
	c->Hide = singleHide;
	c->Enable = singleEnable;
	c->Disable = singleDisable;
}
