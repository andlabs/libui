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
	uiDrawPath *path;
	uiDrawBrush brush;
	uiDrawStrokeParams sp;

	brush.Type = uiDrawBrushTypeSolid;
	brush.A = 1;

	brush.R = 1;
	brush.G = 0;
	brush.B = 0;
	path = uiDrawNewPath(uiDrawFillModeWinding);
	uiDrawPathNewFigure(path, p->ClipX + 5, p->ClipY + 5);
	uiDrawPathLineTo(path, (p->ClipX + p->ClipWidth) - 5, (p->ClipY + p->ClipHeight) - 5);
	uiDrawPathEnd(path);
	sp.Cap = uiDrawLineCapFlat;
	sp.Join = uiDrawLineJoinMiter;
	sp.Thickness = 1;
	sp.MiterLimit = uiDrawDefaultMiterLimit;
	uiDrawStroke(p->Context, path, &brush, &sp);
	uiDrawFreePath(path);

	brush.R = 0;
	brush.G = 0;
	brush.B = 0.75;
	path = uiDrawNewPath(uiDrawFillModeWinding);
	uiDrawPathNewFigure(path, p->ClipX, p->ClipY);
	uiDrawPathLineTo(path, p->ClipX + p->ClipWidth, p->ClipY);
	uiDrawPathLineTo(path, 50, 150);
	uiDrawPathLineTo(path, 50, 50);
	uiDrawPathCloseFigure(path);
	uiDrawPathEnd(path);
	sp.Cap = uiDrawLineCapFlat;
	sp.Join = uiDrawLineJoinRound;
	sp.Thickness = 5;
	uiDrawStroke(p->Context, path, &brush, &sp);
	uiDrawFreePath(path);

	brush.R = 0;
	brush.G = 0.75;
	brush.B = 0;
	brush.A = 0.5;
	path = uiDrawNewPath(uiDrawFillModeWinding);
	uiDrawPathAddRectangle(path, 120, 80, 50, 50);
	uiDrawPathEnd(path);
	uiDrawFill(p->Context, path, &brush);
	uiDrawFreePath(path);
	brush.A = 1;

	brush.R = 0;
	brush.G = 0.5;
	brush.B = 0;
	path = uiDrawNewPath(uiDrawFillModeWinding);
	uiDrawPathNewFigure(path, 5.5, 10.5);
	uiDrawPathLineTo(path, 5.5, 50.5);
	uiDrawPathEnd(path);
	sp.Cap = uiDrawLineCapFlat;
	sp.Join = uiDrawLineJoinMiter;
	sp.Thickness = 1;
	sp.MiterLimit = uiDrawDefaultMiterLimit;
	uiDrawStroke(p->Context, path, &brush, &sp);
	uiDrawFreePath(path);

	brush.R = 0.5;
	brush.G = 0.75;
	brush.B = 0;
	path = uiDrawNewPath(uiDrawFillModeWinding);
	uiDrawPathNewFigure(path, 400, 100);
	uiDrawPathArcTo(path,
		400, 100,
		50,
		30. * (M_PI / 180.),
		// note the end angle here
		// in GDI, the second angle to AngleArc() is relative to the start, not to 0
		330. * (M_PI / 180.));
	// TODO add a checkbox for this
	uiDrawPathLineTo(path, 400, 100);
	uiDrawPathNewFigureWithArc(path,
		510, 100,
		50,
		30. * (M_PI / 180.),
		330. * (M_PI / 180.));
	uiDrawPathCloseFigure(path);
	uiDrawPathEnd(path);
	sp.Cap = uiDrawLineCapFlat;
	sp.Join = uiDrawLineJoinMiter;
	sp.Thickness = 1;
	sp.MiterLimit = uiDrawDefaultMiterLimit;
	uiDrawStroke(p->Context, path, &brush, &sp);
	uiDrawFreePath(path);

	brush.R = 0;
	brush.G = 0.5;
	brush.B = 0.75;
	path = uiDrawNewPath(uiDrawFillModeWinding);
	uiDrawPathNewFigure(path, 300, 300);
	uiDrawPathBezierTo(path,
		350, 320,
		310, 390,
		435, 372);
	uiDrawPathEnd(path);
	sp.Cap = uiDrawLineCapFlat;
	sp.Join = uiDrawLineJoinMiter;
	sp.Thickness = 1;
	sp.MiterLimit = uiDrawDefaultMiterLimit;
	uiDrawStroke(p->Context, path, &brush, &sp);
	uiDrawFreePath(path);

	// based on https://msdn.microsoft.com/en-us/library/windows/desktop/dd756682%28v=vs.85%29.aspx
	path = uiDrawNewPath(uiDrawFillModeWinding);
#define XO 50
#define YO 250
	uiDrawPathNewFigure(path, 0 + XO, 0 + YO);
	uiDrawPathLineTo(path, 200 + XO, 0 + YO);
	uiDrawPathBezierTo(path,
		150 + XO, 50 + YO,
		150 + XO, 150 + YO,
		200 + XO, 200 + YO);
	uiDrawPathLineTo(path, 0 + XO, 200 + YO);
	uiDrawPathBezierTo(path,
		50 + XO, 150 + YO,
		50 + XO, 50 + YO,
		0 + XO, 0 + YO);
	uiDrawPathCloseFigure(path);
	uiDrawPathEnd(path);
	// first the stroke
	brush.Type = uiDrawBrushTypeSolid;
	brush.R = 0;
	brush.G = 0;
	brush.B = 0;
	brush.A = 1;
	sp.Cap = uiDrawLineCapFlat;
	sp.Join = uiDrawLineJoinMiter;
	sp.MiterLimit = uiDrawDefaultMiterLimit;
	sp.Thickness = 10;
	uiDrawStroke(p->Context, path, &brush, &sp);
	// and now the fill
	{
		uiDrawBrushGradientStop stops[2];

		stops[0].Pos = 0.0;
		stops[0].R = 0.0;
		stops[0].G = 1.0;
		stops[0].B = 1.0;
		stops[0].A = 0.25;
		stops[1].Pos = 1.0;
		stops[1].R = 0.0;
		stops[1].G = 0.0;
		stops[1].B = 1.0;
		stops[1].A = 1.0;
		brush.Type = uiDrawBrushTypeLinearGradient;
		brush.X0 = 100 + XO;
		brush.Y0 = 0 + YO;
		brush.X1 = 100 + XO;
		brush.Y1 = 200 + YO;
		brush.Stops = stops;
		brush.NumStops = 2;
		uiDrawFill(p->Context, path, &brush);
	}
#undef YO
#undef XO
	uiDrawFreePath(path);

	// based on https://msdn.microsoft.com/en-us/library/windows/desktop/dd756679%28v=vs.85%29.aspx
	path = uiDrawNewPath(uiDrawFillModeWinding);
	uiDrawPathNewFigure(path, 585, 235);
	uiDrawPathArcTo(path,
		510, 235,
		75,
		0,
		// TODO why doesn't 360° work
		2 * M_PI - 0.1);
	uiDrawPathEnd(path);
	// first the stroke
	brush.Type = uiDrawBrushTypeSolid;
	brush.R = 0;
	brush.G = 0;
	brush.B = 0;
	brush.A = 1;
	sp.Cap = uiDrawLineCapFlat;
	sp.Join = uiDrawLineJoinMiter;
	sp.MiterLimit = uiDrawDefaultMiterLimit;
	sp.Thickness = 1;
	uiDrawStroke(p->Context, path, &brush, &sp);
	// then the fill
	{
		uiDrawBrushGradientStop stops[2];

		stops[0].Pos = 0.0;
		stops[0].R = 1.0;
		stops[0].G = 1.0;
		stops[0].B = 0.0;
		stops[0].A = 1.0;
		stops[1].Pos = 1.0;
		stops[1].R = ((double) 0x22) / 255.0;
		stops[1].G = ((double) 0x8B) / 255.0;
		stops[1].B = ((double) 0x22) / 255.0;
		stops[1].A = 1.0;
		brush.Type = uiDrawBrushTypeRadialGradient;
		// start point
		brush.X0 = 510;
		brush.Y0 = 235;
		// outer circle's center
		brush.X1 = 510;
		brush.Y1 = 235;
		brush.OuterRadius = 75;
		brush.Stops = stops;
		brush.NumStops = 2;
		uiDrawFill(p->Context, path, &brush);
	}
	uiDrawFreePath(path);
}

static uintmax_t handlerHScrollMax(uiAreaHandler *a, uiArea *area)
{return 1000;/* TODO
	WCHAR c[50];

	GetWindowTextW(nhspinb, c, 50);
	return _wtoi(c);
*/}

static uintmax_t handlerVScrollMax(uiAreaHandler *a, uiArea *area)
{return 1000;/* TODO
	WCHAR c[50];

	GetWindowTextW(nvspinb, c, 50);
	return _wtoi(c);
*/}

static int handlerRedrawOnResize(uiAreaHandler *a, uiArea *area)
{
	return 1;
}

static void handlerMouseEvent(uiAreaHandler *a, uiArea *area, uiAreaMouseEvent *e)
{
	printf("mouse (%d,%d):(%d,%d) down:%d up:%d count:%d mods:%x held:%x\n",
		(int) e->X,
		(int) e->Y,
		(int) e->HScrollPos,
		(int) e->VScrollPos,
		(int) e->Down,
		(int) e->Up,
		(int) e->Count,
		(uint32_t) e->Modifiers,
		e->Held1To64);
}

static void handlerDragBroken(uiAreaHandler *ah, uiArea *a)
{
	printf("drag broken\n");
}

static int handlerKeyEvent(uiAreaHandler *ah, uiArea *a, uiAreaKeyEvent *e)
{
	char k[4];

	k[0] = '\'';
	k[1] = e->Key;
	k[2] = '\'';
	k[3] = '\0';
	if (e->Key == 0) {
		k[0] = '0';
		k[1] = '\0';
	}
	printf("key key:%s extkey:%d mod:%d mods:%d up:%d\n",
		k,
		(int) e->ExtKey,
		(int) e->Modifier,
		(int) e->Modifiers,
		e->Up);
	return 0;
}

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
	areav = areaGetView(area);
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

@interface testApplication : NSApplication
@end

@implementation testApplication

- (void)sendEvent:(NSEvent *)e
{
	if (!sendAreaEvents(e))
		[super sendEvent:e];
}

@end

int main(void)
{
	NSApplication *app;

	h.ah.Draw = handlerDraw;
	h.ah.HScrollMax = handlerHScrollMax;
	h.ah.VScrollMax = handlerVScrollMax;
	h.ah.RedrawOnResize = handlerRedrawOnResize;
	h.ah.MouseEvent = handlerMouseEvent;
	h.ah.DragBroken = handlerDragBroken;
	h.ah.KeyEvent = handlerKeyEvent;

	app = [testApplication sharedApplication];
	[app setActivationPolicy:NSApplicationActivationPolicyRegular];
	[app setDelegate:[appDelegate new]];
	[app run];
	return 0;
}
