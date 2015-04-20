// 17 april 2015
#import "uipriv_darwin.h"

@interface uipParent : NSView {
	uiControl *mainControl;
	intmax_t marginLeft;
	intmax_t marginTop;
	intmax_t marginRight;
	intmax_t marginBottom;
}
- (void)uipDestroyMainControl;
- (void)uipSetMainControl:(uiControl *)mainControl parent:(uiParent *)p;
- (void)uipSetMarginLeft:(intmax_t)left top:(intmax_t)top right:(intmax_t)right bottom:(intmax_t)bottom;
- (void)uipUpdate;
@end

@implementation uipParent

uiLogObjCClassAllocations

- (void)setFrameSize:(NSSize)s
{
	[super setFrameSize:s];
	[self uipUpdate];
}

- (void)uipDestroyMainControl
{
	if (self->mainControl != NULL) {
		// we have to do this before we can destroy controls
		uiControlSetParent(p->mainControl, NULL);
		uiControlDestroy(self->mainControl);
		self->mainControl = NULL;
	}
}

- (void)uipSetMainControl:(uiControl *)mainControl parent:(uiParent *)p
{
	if (self->mainControl != NULL)
		uiControlSetParent(self->mainControl, NULL);
	self->mainControl = mainControl;
	if (self->mainControl != NULL)
		uiControlSetParent(self->mainControl, p);
}

- (void)uipSetMarginLeft:(intmax_t)left top:(intmax_t)top right:(intmax_t)right bottom:(intmax_t)bottom
{
	self->marginLeft = left;
	self->marginTop = top;
	self->marginRight = right;
	self->marginBottom = bottom;
}

// These are based on measurements from Interface Builder.
// These seem to be based on Auto Layout constants, but I don't see an API that exposes these...
// This one is 8 for most pairs of controls that I've tried; the only difference is between two pushbuttons, where it's 12...
#define macXPadding 8
// Likewise, this one appears to be 12 for pairs of push buttons...
#define macYPadding 8

- (void)uipUpdate
{
	uiSizing d;
	intmax_t x, y, width, height;

	if (self->mainControl == NULL)
		return;
	x = [self bounds].origin.x + self->marginLeft;
	y = [self bounds].origin.y + self->marginTop;
	width = [self bounds].size.width - (self->marginLeft + self->marginRight);
	height = [self bounds].size.height - (self->marginTop + self->marginBottom);
	d.xPadding = macXPadding;
	d.yPadding = macYPadding;
	uiControlResize(self->mainControl, x, y, width, height, &d);
}

@end

static void parentDestroy(uiParent *pp)
{
	uipParent *p = (uipParent *) (pp->Internal);

	[p retain];		// to avoid destruction upon removing from superview
	[p uipDestroyMainControl];
	[p removeFromSuperview];
	[destroyedControlsView addSubview:p];
	[p release];
}

static uintptr_t parentHandle(uiParent *p)
{
	return (uintptr_t) (p->Internal);
}

static void parentSetMainControl(uiParent *pp, uiControl *mainControl)
{
	uipParent *p = (uipParent *) (pp->Internal);

	[p uipSetMainControl:mainControl parent:pp];
}

static void parentSetMargins(uiParent *pp, intmax_t left, intmax_t top, intmax_t right, intmax_t bottom)
{
	uipParent *p = (uipParent *) (pp->Internal);

	[p uipSetMarginLeft:left top:top right:right bottom:bottom];
}

static void parentUpdate(uiParent *pp)
{
	uipParent *p = (uipParent *) (pp->Internal);

	[p uipUpdate];
}

uiParent *uiNewParent(uintptr_t osParent)
{
	uiParent *p;
	uipParent *pp;

	p = uiNew(uiParent);

	pp = [[uipParent alloc] initWithFrame:NSZeroRect];
	// don't use osParent; we'll need to call specific selectors to set the parent view
	p->Internal = pp;

	p->Destroy = parentDestroy;
	p->Handle = parentHandle;
	p->SetMainControl = parentSetMainControl;
	p->SetMargins = parentSetMargins;
	p->Update = parentUpdate;

	return p;
}
