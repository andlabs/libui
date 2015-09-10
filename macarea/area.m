// 9 september 2015
#include "area.h"

// model:
// - drawing and events happen to the NSClipView
// - the document view is only used for scroll extents

@interface areaClipView : NSClipView {
	uiArea *libui_a;
	uiAreaHandler *libui_ah;
}
- (id)initWithFrame:(NSRect)r area:(uiArea *)a areaHandler:(uiAreaHandler *)ah;
@end

@interface areaDocumentView : NSView
@end

struct uiArea {
//	uiDarwinControl c;
	NSScrollView *view;
	areaClipView *clipView;
	areaDocumentView *documentView;
	uiAreaHandler *ah;
};

@implementation areaClipView

- (id)initWithFrame:(NSRect)r area:(uiArea *)a areaHandler:(uiAreaHandler *)ah
{
	self = [super initWithFrame:r];
	if (self) {
		self->libui_a = a;
		self->libui_ah = ah;
	}
	return self;
}

- (void)drawRect:(NSRect)r
{
	CGContextRef c;
	uiAreaDrawParams dp;

	[super drawRect:r];

	c = (CGContextRef) [[NSGraphicsContext currentContext] graphicsPort];
	dp.Context = newContext(c);

	dp.ClientWidth = [self frame].size.width;
	dp.ClientHeight = [self frame].size.height;

	dp.ClipX = r.origin.x;
	dp.ClipY = r.origin.y;
	dp.ClipWidth = r.size.width;
	dp.ClipHeight = r.size.height;

	// TODO DPI

	// TODO scroll position

	(*(self->libui_ah->Draw))(self->libui_ah, self->libui_a, &dp);
}

- (BOOL)isFlipped
{
	return YES;
}

// see http://stackoverflow.com/questions/11318987/black-background-when-overriding-drawrect-in-uiscrollview (for iOS but same idea)
// TODO there has to be a better way to set this; how does OS X do it for its default clip view?
/*- (BOOL)isOpaque
{
	return NO;
}*/

@end

@implementation areaDocumentView

- (BOOL)isFlipped
{
	return YES;
}

@end

uiArea *newArea(uiAreaHandler *ah)
{
	uiArea *a;

	// TODO
	a = (uiArea *) malloc(sizeof (uiArea));

	a->ah = ah;

	a->view = [[NSScrollView alloc] initWithFrame:NSZeroRect];
	a->clipView = [[areaClipView alloc] initWithFrame:NSZeroRect area:a areaHandler:ah];
	a->documentView = [[areaDocumentView alloc] initWithFrame:NSZeroRect];

	[a->view setContentView:a->clipView];
	[a->view setDocumentView:a->documentView];

	// TODO set up scroll view
	// for some reason, without this line, NSLayoutConstraints complains about internal limits being exceeded
	[a->view setDrawsBackground:YES];
//TODO	[a->view setBackgroundColor:[NSColor controlColor]];

	// set initial state
	// TODO do this on other platforms?
	areaUpdateScroll(a);

	return a;
}

NSView *areaView(uiArea *a)
{
	return a->view;
}

void areaUpdateScroll(uiArea *a)
{
	NSRect frame;

	frame.origin = NSMakePoint(0, 0);
	frame.size.width = (*(a->ah->HScrollMax))(a->ah, a);
	frame.size.height = (*(a->ah->VScrollMax))(a->ah, a);
	[a->documentView setFrame:frame];
}
