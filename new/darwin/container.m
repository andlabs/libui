// 28 april 2015
#import "uipriv_darwin.h"

@interface containerView : NSView {
	uiContainer *c;
	uiContainer *parent;
	int hidden;
}
@property uiContainer *containerParent;
@property int containerHidden;
- (void)setContainer:(uiContainer *)cc;
- (void)containerUpdate;
@end

@implementation containerView

// TODO allocation logger

- (void)setContainer:(uiContainer *)cc
{
	self->c = cc;
}

// These are based on measurements from Interface Builder.
// These seem to be based on Auto Layout constants, but I don't see an API that exposes these...
// This one is 8 for most pairs of controls that I've tried; the only difference is between two pushbuttons, where it's 12...
#define macXPadding 8
// Likewise, this one appears to be 12 for pairs of push buttons...
#define macYPadding 8

- (void)containerUpdate
{
	uiSizing d;
	intmax_t x, y, width, height;

	x = [self bounds].origin.x;
	y = [self bounds].origin.y;
	width = [self bounds].size.width;
	height = [self bounds].size.height;
	d.xPadding = macXPadding;
	d.yPadding = macYPadding;
	uiContainerResizeChildren(self->c, x, y, width, height, &d);
}

- (void)setFrameSize:(NSSize)s
{
	[super setFrameSize:s];
	[self containerUpdate];
}

@end

// subclasses override this and call back here when all children are destroyed
static void containerDestroy(uiControl *cc)
{
	containerView *c = (containerView *) (cc->Internal);

	if (c.containerParent != NULL)
		complain("attempt to destroy uiContainer %p while it has a parent", cc);
	[c release];		// release our initial reference, which destroys the view
}

static uintptr_t containerHandle(uiControl *cc)
{
	containerView *c = (containerView *) (cc->Internal);

	return (uintptr_t) c;
}

static void containerSetParent(uiControl *cc, uiContainer *parent)
{
	containerView *c = (containerView *) (cc->Internal);
	uiContainer *oldparent;
	NSView *newcontainer;

	oldparent = c.containerParent;
	c.containerParent = parent;
	if (oldparent != NULL)
		[c removeFromSuperview];
	if (c.containerParent != NULL) {
		newcontainer = (NSView *) uiControlHandle(uiControl(c.containerParent));
		[newcontainer addSubview:c];
	}
	if (oldparent != NULL)
		uiContainerUpdate(oldparent);
	if (c.containerParent != NULL)
		uiContainerUpdate(c.containerParent);
}

static void containerResize(uiControl *cc, intmax_t x, intmax_t y, intmax_t width, intmax_t height, uiSizing *d)
{
	containerView *c = (containerView *) (cc->Internal);
	NSRect r;

	r.origin.x = x;
	r.origin.y = y;
	r.size.width = width;
	r.size.height = height;
	// we can safely use setFrame here since we have no alignment rect to worry about
	[c setFrame:r];
}

static int containerVisible(uiControl *cc)
{
	containerView *c = (containerView *) (cc->Internal);

	return !c.containerHidden;
}

static void containerShow(uiControl *cc)
{
	containerView *c = (containerView *) (cc->Internal);

	[c setHidden:NO];
	c.containerHidden = 0;
}

static void containerHide(uiControl *cc)
{
	containerView *c = (containerView *) (cc->Internal);

	[c setHidden:YES];
	c.containerHidden = 1;
}

static void containerEnable(uiControl *cc)
{
	containerView *c = (containerView *) (cc->Internal);

	// TODO
}

static void containerDisable(uiControl *cc)
{
	containerView *c = (containerView *) (cc->Internal);

	// TODO
}

static void containerUpdate(uiContainer *cc)
{
	containerView *c = (containerView *) (uiControl(cc)->Internal);

	[c containerUpdate];
}

void uiMakeContainer(uiContainer *cc)
{
	containerView *c;

	c = [[containerView alloc] initWithFrame:NSZeroRect];
	[c setContainer:cc];
	// keep a reference to our container so it stays alive when reparented
	[c retain];

	uiControl(cc)->Internal = c;
	uiControl(cc)->Destroy = containerDestroy;
	uiControl(cc)->Handle = containerHandle;
	uiControl(cc)->SetParent = containerSetParent;
	// PreferredSize() is provided by subclasses
	uiControl(cc)->Resize = containerResize;
	uiControl(cc)->Visible = containerVisible;
	uiControl(cc)->Show = containerShow;
	uiControl(cc)->Hide = containerHide;
	uiControl(cc)->Enable = containerEnable;
	uiControl(cc)->Disable = containerDisable;

	// ResizeChildren() is provided by subclasses
	uiContainer(cc)->Update = containerUpdate;
}
