// 7 april 2015
#include "uipriv_darwin.h"

struct singleView {
	NSView *view;
	NSScrollView *scrollView;
	NSView *immediate;		// the control that is added to the parent container; either view or scrollView
	uiContainer *parent;
	int hidden;
	int userDisabled;
	int containerDisabled;
	void (*onDestroy)(void *);
	void *onDestroyData;
};

static void singleDestroy(uiControl *c)
{
	struct singleView *s = (struct singleView *) (c->Internal);

	if (s->parent != NULL)
		complain("attempt to destroy a uiControl at %p while it still has a parent", c);
	(*(s->onDestroy))(s->onDestroyData);
	// release the reference we took on creation to destroy the widget
	[s->immediate release];
	uiFree(s);
}

static uintptr_t singleHandle(uiControl *c)
{
	struct singleView *s = (struct singleView *) (c->Internal);

	return (uintptr_t) (s->view);
}

static void singleSetParent(uiControl *c, uiContainer *parent)
{
	struct singleView *s = (struct singleView *) (c->Internal);
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
	struct singleView *s = (struct singleView *) (c->Internal);
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
	struct singleView *s = (struct singleView *) (c->Internal);
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
	struct singleView *s = (struct singleView *) (c->Internal);

	return !s->hidden;
}

static void singleShow(uiControl *c)
{
	struct singleView *s = (struct singleView *) (c->Internal);

	[s->immediate setHidden:NO];
	s->hidden = 0;
	if (s->parent != NULL)
		uiContainerUpdate(s->parent);
}

static void singleHide(uiControl *c)
{
	struct singleView *s = (struct singleView *) (c->Internal);

	[s->immediate setHidden:YES];
	s->hidden = 1;
	if (s->parent != NULL)
		uiContainerUpdate(s->parent);
}

static void singleEnable(uiControl *c)
{
	struct singleView *s = (struct singleView *) (c->Internal);

	s->userDisabled = 0;
	if (!s->containerDisabled)
		if ([s->view respondsToSelector:@selector(setEnabled:)])
			[((NSControl *) (s->view)) setEnabled:YES];
}

static void singleDisable(uiControl *c)
{
	struct singleView *s = (struct singleView *) (c->Internal);

	s->userDisabled = 1;
	if ([s->view respondsToSelector:@selector(setEnabled:)])
		[((NSControl *) (s->view)) setEnabled:NO];
}

static void singleSysFunc(uiControl *c, uiControlSysFuncParams *p)
{
	struct singleView *s = (struct singleView *) (c->Internal);

	switch (p->Func) {
	case uiDarwinSysFuncContainerEnable:
		s->containerDisabled = 0;
		if (!s->userDisabled)
			if ([s->view respondsToSelector:@selector(setEnabled:)])
				[((NSControl *) (s->view)) setEnabled:YES];
		return;
	case uiDarwinSysFuncContainerDisable:
		s->containerDisabled = 1;
		if ([s->view respondsToSelector:@selector(setEnabled:)])
			[((NSControl *) (s->view)) setEnabled:NO];
		return;
	}
	complain("unknown p->Func %d in singleSysFunc()", p->Func);
}

void uiDarwinMakeControl(uiControl *c, Class class, BOOL inScrollView, BOOL scrollViewHasBorder, void (*onDestroy)(void *), void *onDestroyData)
{
	struct singleView *s;

	s = uiNew(struct singleView);
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

	uiControl(c)->Internal = s;
	uiControl(c)->Destroy = singleDestroy;
	uiControl(c)->Handle = singleHandle;
	uiControl(c)->SetParent = singleSetParent;
	uiControl(c)->PreferredSize = singlePreferredSize;
	uiControl(c)->Resize = singleResize;
	uiControl(c)->Visible = singleVisible;
	uiControl(c)->Show = singleShow;
	uiControl(c)->Hide = singleHide;
	uiControl(c)->Enable = singleEnable;
	uiControl(c)->Disable = singleDisable;
	uiControl(c)->SysFunc = singleSysFunc;
}
