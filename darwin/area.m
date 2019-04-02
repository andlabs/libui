// 9 september 2015
#import "uipriv_darwin.h"

// 10.8 fixups
#define NSEventModifierFlags NSUInteger

@interface areaView : uiprivAreaCommonView {
	uiArea *libui_a;
	NSSize libui_ss;
}
- (id)initWithFrame:(NSRect)r area:(uiArea *)a;
- (void)setScrollingSize:(NSSize)s;
@end

struct uiArea {
	uiDarwinControl c;
	NSView *view;			// either sv or area depending on whether it is scrolling
	uiAreaHandler *ah;
	NSEvent *dragevent;
	BOOL scrolling;
	NSScrollView *sv;
	areaView *area;
	uiprivScrollViewData *d;
};

@implementation areaView

- (id)initWithFrame:(NSRect)r area:(uiArea *)a
{
	self = [super initWithFrame:r];
	if (self) {
		self->libui_a = a;
		[self setArea:a];
		[self setupNewTrackingArea];
		self->libui_ss = r.size;
		self->libui_enabled = YES;
	}
	return self;
}

- (void)drawRect:(NSRect)r
{
	uiArea *a = self->libui_a;
	CGContextRef c;
	uiAreaDrawParams dp;

	c = (CGContextRef) [[NSGraphicsContext currentContext] graphicsPort];
	// see draw.m under text for why we need the height
	dp.Context = uiprivDrawNewContext(c, [self bounds].size.height);

	dp.AreaWidth = 0;
	dp.AreaHeight = 0;
	if (!a->scrolling) {
		dp.AreaWidth = [self frame].size.width;
		dp.AreaHeight = [self frame].size.height;
	}

	dp.ClipX = r.origin.x;
	dp.ClipY = r.origin.y;
	dp.ClipWidth = r.size.width;
	dp.ClipHeight = r.size.height;

	// no need to save or restore the graphics state to reset transformations; Cocoa creates a brand-new context each time
	(*(a->ah->Draw))(a->ah, a, &dp);

	uiprivDrawFreeContext(dp.Context);
}

- (void)setScrollingSize:(NSSize)s
{
	self->libui_ss = s;
	[self setFrameSize:s];
}

- (NSSize)intrinsicContentSize
{
	if (!self->libui_a->scrolling)
		return [super intrinsicContentSize];
	return self->libui_ss;
}

@end

uiDarwinControlAllDefaultsExceptDestroy(uiArea, view)

static void uiAreaDestroy(uiControl *c)
{
	uiArea *a = uiArea(c);

	if (a->scrolling)
		uiprivScrollViewFreeData(a->sv, a->d);
	[a->area release];
	if (a->scrolling)
		[a->sv release];
	uiFreeControl(uiControl(a));
}

// called by subclasses of -[NSApplication sendEvent:]
// by default, NSApplication eats some key events
// this prevents that from happening with uiArea
// see http://stackoverflow.com/questions/24099063/how-do-i-detect-keyup-in-my-nsview-with-the-command-key-held and http://lists.apple.com/archives/cocoa-dev/2003/Oct/msg00442.html
int uiprivSendAreaEvents(NSEvent *e)
{
	NSEventType type;
	id focused;
	areaView *view;

	type = [e type];
	if (type != NSKeyDown && type != NSKeyUp && type != NSFlagsChanged)
		return 0;
	focused = [[e window] firstResponder];
	if (focused == nil)
		return 0;
	if (![focused isKindOfClass:[uiprivAreaCommonView class]])
		return 0;
	view = (areaView *) focused;
	switch (type) {
	case NSKeyDown:
		return [view doKeyDown:e];
	case NSKeyUp:
		return [view doKeyUp:e];
	case NSFlagsChanged:
		return [view doFlagsChanged:e];
	}
	return 0;
}

void uiAreaSetSize(uiArea *a, int width, int height)
{
	if (!a->scrolling)
		uiprivUserBug("You cannot call uiAreaSetSize() on a non-scrolling uiArea. (area: %p)", a);
	[a->area setScrollingSize:NSMakeSize(width, height)];
}

void uiAreaQueueRedrawAll(uiArea *a)
{
	[a->area setNeedsDisplay:YES];
}

void uiAreaScrollTo(uiArea *a, double x, double y, double width, double height)
{
	if (!a->scrolling)
		uiprivUserBug("You cannot call uiAreaScrollTo() on a non-scrolling uiArea. (area: %p)", a);
	[a->area scrollRectToVisible:NSMakeRect(x, y, width, height)];
	// don't worry about the return value; it just says whether scrolling was needed
}

void uiAreaBeginUserWindowMove(uiArea *a)
{
	uiprivNSWindow *w;

	w = (uiprivNSWindow *) [a->area window];
	if (w == nil)
		return;		// TODO
	if (a->dragevent == nil)
		return;		// TODO
	[w uiprivDoMove:a->dragevent];
}

void uiAreaBeginUserWindowResize(uiArea *a, uiWindowResizeEdge edge)
{
	uiprivNSWindow *w;

	w = (uiprivNSWindow *) [a->area window];
	if (w == nil)
		return;		// TODO
	if (a->dragevent == nil)
		return;		// TODO
	[w uiprivDoResize:a->dragevent on:edge];
}

uiArea *uiNewArea(uiAreaHandler *ah)
{
	uiArea *a;

	uiDarwinNewControl(uiArea, a);

	a->ah = ah;
	a->scrolling = NO;

	a->area = [[areaView alloc] initWithFrame:NSZeroRect area:a];

	a->view = a->area;

	return a;
}

uiArea *uiNewScrollingArea(uiAreaHandler *ah, int width, int height)
{
	uiArea *a;
	uiprivScrollViewCreateParams p;

	uiDarwinNewControl(uiArea, a);

	a->ah = ah;
	a->scrolling = YES;

	a->area = [[areaView alloc] initWithFrame:NSMakeRect(0, 0, width, height)
		area:a];

	memset(&p, 0, sizeof (uiprivScrollViewCreateParams));
	p.DocumentView = a->area;
	p.BackgroundColor = [NSColor controlColor];
	p.DrawsBackground = 1;
	p.Bordered = NO;
	p.HScroll = YES;
	p.VScroll = YES;
	a->sv = uiprivMkScrollView(&p, &(a->d));

	a->view = a->sv;

	return a;
}
