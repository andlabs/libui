// 4 september 2015
#define _GNU_SOURCE
#include "area.h"
#include <math.h>

// #qo LDFLAGS: -framework Foundation -framework AppKit -framework CoreGraphics

struct handler {
	uiAreaHandler ah;
};

static uiArea *area;
static struct handler h;
//static NSTextField *nhspinb;
//static NSTextField *nvspinb;

static void handlerDraw(uiAreaHandler *a, uiArea *area, uiAreaDrawParams *p)
{
	uiDrawStrokeParams sp;

	uiDrawBeginPathRGB(p->Context, 0xFF, 0x00, 0x00);
	uiDrawMoveTo(p->Context, p->ClipX + 5, p->ClipY + 5);
	uiDrawLineTo(p->Context, (p->ClipX + p->ClipWidth) - 5, (p->ClipY + p->ClipHeight) - 5);
	sp.Cap = uiDrawLineCapFlat;
	sp.Join = uiDrawLineJoinMiter;
	sp.Thickness = 1;
	sp.MiterLimit = uiDrawDefaultMiterLimit;
	uiDrawStroke(p->Context, &sp);

	uiDrawBeginPathRGB(p->Context, 0x00, 0x00, 0xC0);
	uiDrawMoveTo(p->Context, p->ClipX, p->ClipY);
	uiDrawLineTo(p->Context, p->ClipX + p->ClipWidth, p->ClipY);
	uiDrawLineTo(p->Context, 50, 150);
	uiDrawLineTo(p->Context, 50, 50);
	uiDrawCloseFigure(p->Context);
	sp.Cap = uiDrawLineCapFlat;
	sp.Join = uiDrawLineJoinRound;
	sp.Thickness = 5;
	uiDrawStroke(p->Context, &sp);

	uiDrawBeginPathRGBA(p->Context, 0x00, 0xC0, 0x00, 0x80);
	uiDrawRectangle(p->Context, 120, 80, 50, 50);
	uiDrawFill(p->Context, uiDrawFillModeWinding);

	uiDrawBeginPathRGB(p->Context, 0x00, 0x80, 0x00);
	uiDrawMoveTo(p->Context, 5, 10);
	uiDrawLineTo(p->Context, 5, 50);
	sp.Cap = uiDrawLineCapFlat;
	sp.Join = uiDrawLineJoinMiter;
	sp.Thickness = 1;
	sp.MiterLimit = uiDrawDefaultMiterLimit;
	uiDrawStroke(p->Context, &sp);

	uiDrawBeginPathRGB(p->Context, 0x80, 0xC0, 0x00);
	uiDrawMoveTo(p->Context, 400, 100);
	uiDrawArcTo(p->Context,
		400, 100,
		50,
		30. * (M_PI / 180.),
		// note the end angle here
		// in GDI, the second angle to AngleArc() is relative to the start, not to 0
		330. * (M_PI / 180.),
		1);
	// TODO add a checkbox for this
	uiDrawLineTo(p->Context, 400, 100);
	uiDrawArcTo(p->Context,
		510, 100,
		50,
		30. * (M_PI / 180.),
		330. * (M_PI / 180.),
		0);
	uiDrawCloseFigure(p->Context);
	sp.Cap = uiDrawLineCapFlat;
	sp.Join = uiDrawLineJoinMiter;
	sp.Thickness = 1;
	sp.MiterLimit = uiDrawDefaultMiterLimit;
	uiDrawStroke(p->Context, &sp);

	uiDrawBeginPathRGB(p->Context, 0x00, 0x80, 0xC0);
	uiDrawMoveTo(p->Context, 300, 300);
	uiDrawBezierTo(p->Context,
		350, 320,
		310, 390,
		435, 372);
	sp.Cap = uiDrawLineCapFlat;
	sp.Join = uiDrawLineJoinMiter;
	sp.Thickness = 1;
	sp.MiterLimit = uiDrawDefaultMiterLimit;
	uiDrawStroke(p->Context, &sp);
}

static uintmax_t handlerHScrollMax(uiAreaHandler *a, uiArea *area)
{/* TODO
	WCHAR c[50];

	GetWindowTextW(nhspinb, c, 50);
	return _wtoi(c);
*/}

static uintmax_t handlerVScrollMax(uiAreaHandler *a, uiArea *area)
{/* TODO
	WCHAR c[50];

	GetWindowTextW(nvspinb, c, 50);
	return _wtoi(c);
*/}

// areaUpdateScroll(area);

@interface appDelegate : NSObject<NSApplicationDelegate, NSTextFieldDelegate>
@end

@implementation appDelegate

- (void)controlTextDidChange:(NSNotification *)note
{
	areaUpdateScroll(area);
}

- (void)applicationDidFinishLaunching:(NSApplication *)app
{
	NSWindow *mainwin;
	NSView *contentView;
	NSView *areav;
	NSArray *constraints;
	NSDictionary *views;

	mainwin = [[NSWindow alloc] initWithContentRect:NSMakeRect(0, 0, 500, 500)
		styleMask:(NSTitledWindowMask | NSClosableWindowMask | NSMiniaturizableWindowMask | NSResizableWindowMask)
		backing:NSBackingStoreBuffered
		defer:YES];
	contentView = [mainwin contentView];

	area = newArea((uiAreaHandler *) (&h));
	areav = areaView(area);
	[areav setTranslatesAutoresizingMaskIntoConstraints:NO];
	[contentView addSubview:areav];

	views = @{
		@"areav":		areav,
	};
	constraints = [NSLayoutConstraint constraintsWithVisualFormat:@"H:|-[areav]-|"
		options:0
		metrics:nil
		views:views];
	[contentView addConstraints:constraints];
	constraints = [NSLayoutConstraint constraintsWithVisualFormat:@"V:|-[areav]-|"
		options:0
		metrics:nil
		views:views];
	[contentView addConstraints:constraints];

	[mainwin makeKeyAndOrderFront:nil];
}

@end

int main(void)
{
	NSApplication *app;

	h.ah.Draw = handlerDraw;
	h.ah.HScrollMax = handlerHScrollMax;
	h.ah.VScrollMax = handlerVScrollMax;

	app = [NSApplication sharedApplication];
	[app setActivationPolicy:NSApplicationActivationPolicyRegular];
	[app setDelegate:[appDelegate new]];
	[app run];
	return 0;
}
