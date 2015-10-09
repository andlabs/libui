// 9 october 2015
#include "test.h"

struct drawtest {
	const char *name;
	void (*draw)(uiAreaDrawParams *p);
	// TODO mouse event
};

static void drawOriginal(uiAreaDrawParams *p)
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
}

// Direct2D Examples

static void d2dColorToRGB(uint32_t color, double *r, double *g, double *b)
{
	uint8_t rr, gg, bb;

	rr = (color & 0xFF0000) >> 16;
	gg = (color & 0x00FF00) >> 8;
	bb = color & 0x0000FF;
	*r = ((double) rr) / 255.0;
	*g = ((double) gg) / 255.0;
	*b = ((double) bb) / 255.0;
}
#define d2dBlack 0x000000
#define d2dLightSlateGray 0x778899
#define d2dCornflowerBlue 0x6495ED
#define d2dWhite 0xFFFFFF
#define d2dYellowGreen 0x9ACD32
#define d2dYellow 0xFFFF00
#define d2dForestGreen 0x228B22
#define d2dOliveDrab 0x6B8E23
#define d2dLightSkyBlue 0x87CEFA

static void d2dSolidBrush(uiDrawBrush *brush, uint32_t color, double alpha)
{
	brush->Type = uiDrawBrushTypeSolid;
	d2dColorToRGB(color, &(brush->R), &(brush->G), &(brush->B));
	brush->A = alpha;
}

static void d2dClear(uiAreaDrawParams *p, uint32_t color, double alpha)
{
	uiDrawPath *path;
	uiDrawBrush brush;

	d2dSolidBrush(&brush, color, alpha);
	path = uiDrawNewPath(uiDrawFillModeWinding);
	uiDrawPathAddRectangle(path, 0, 0, p->ClientWidth, p->ClientHeight);
	uiDrawPathEnd(path);
	uiDrawFill(p->Context, path, &brush);
	uiDrawFreePath(path);
}

// from https://msdn.microsoft.com/en-us/library/windows/desktop/hh780340%28v=vs.85%29.aspx
// also at https://msdn.microsoft.com/en-us/library/windows/desktop/dd535473%28v=vs.85%29.aspx
static void drawD2DW8QS(uiAreaDrawParams *p)
{
	uiDrawPath *path;
	uiDrawBrush brush;

	d2dSolidBrush(&brush, d2dBlack, 1.0);
	path = uiDrawNewPath(uiDrawFillModeWinding);
	uiDrawPathAddRectangle(path,
		100,
		100,
		(p->ClientWidth - 100) - 100,
		(p->ClientHeight - 100) - 100);
	uiDrawPathEnd(path);
	uiDrawFill(p->Context, path, &brush);
	uiDrawFreePath(path);
}

// from https://msdn.microsoft.com/en-us/library/windows/desktop/dd370994%28v=vs.85%29.aspx
static void drawD2DSimpleApp(uiAreaDrawParams *p)
{
	uiDrawPath *path;
	uiDrawBrush lightSlateGray;
	uiDrawBrush cornflowerBlue;
	uiDrawStrokeParams sp;
	int x, y;

	d2dSolidBrush(&lightSlateGray, d2dLightSlateGray, 1.0);
	d2dSolidBrush(&cornflowerBlue, d2dCornflowerBlue, 1.0);

	d2dClear(p, d2dWhite, 1.0);

	sp.Thickness = 0.5;
	sp.Cap = uiDrawLineCapFlat;
	sp.Join = uiDrawLineJoinMiter;
	sp.MiterLimit = uiDrawDefaultMiterLimit;

	for (x = 0; x < p->ClientWidth; x += 10) {
		path = uiDrawNewPath(uiDrawFillModeWinding);
		uiDrawPathNewFigure(path, x, 0);
		uiDrawPathLineTo(path, x, p->ClientHeight);
		uiDrawPathEnd(path);
		uiDrawStroke(p->Context, path, &lightSlateGray, &sp);
		uiDrawFreePath(path);
	}

	for (y = 0; y < p->ClientHeight; y += 10) {
		path = uiDrawNewPath(uiDrawFillModeWinding);
		uiDrawPathNewFigure(path, 0, y);
		uiDrawPathLineTo(path, p->ClientWidth, y);
		uiDrawPathEnd(path);
		uiDrawStroke(p->Context, path, &lightSlateGray, &sp);
		uiDrawFreePath(path);
	}

	// TODO rounding errors ahoy; deal with when changing coordinate systems of uiAreaDrawParams
	double left, top, right, bottom;

	left = p->ClientWidth / 2.0 - 50.0;
	right = p->ClientWidth / 2.0 + 50.0;
	top = p->ClientHeight / 2.0 - 50.0;
	bottom = p->ClientHeight / 2.0 + 50.0;
	path = uiDrawNewPath(uiDrawFillModeWinding);
	uiDrawPathAddRectangle(path, left, top, right - left, bottom - top);
	uiDrawPathEnd(path);
	uiDrawFill(p->Context, path, &lightSlateGray);
	uiDrawFreePath(path);

	left = p->ClientWidth / 2.0 - 100.0;
	right = p->ClientWidth / 2.0 + 100.0;
	top = p->ClientHeight / 2.0 - 100.0;
	bottom = p->ClientHeight / 2.0 + 100.0;
	path = uiDrawNewPath(uiDrawFillModeWinding);
	uiDrawPathAddRectangle(path, left, top, right - left, bottom - top);
	uiDrawPathEnd(path);
	sp.Thickness = 1.0;
	uiDrawStroke(p->Context, path, &cornflowerBlue, &sp);
	uiDrawFreePath(path);
}

// TODO? https://msdn.microsoft.com/en-us/library/windows/desktop/dd372260(v=vs.85).aspx

// TODO https://msdn.microsoft.com/en-us/library/windows/desktop/dd756654%28v=vs.85%29.aspx

// TODO? all subsections too? https://msdn.microsoft.com/en-us/library/windows/desktop/hh973240%28v=vs.85%29.aspx

// TODO differing examples of? https://msdn.microsoft.com/en-us/library/windows/desktop/dd756651%28v=vs.85%29.aspx

// from https://msdn.microsoft.com/en-us/library/windows/desktop/dd756680%28v=vs.85%29.aspx
static void drawD2DSolidBrush(uiAreaDrawParams *p)
{
	uiDrawPath *path;
	uiDrawBrush black;
	uiDrawBrush yellowGreen;
	uiDrawStrokeParams sp;

	d2dSolidBrush(&black, d2dBlack, 1.0);
	d2dSolidBrush(&yellowGreen, d2dYellowGreen, 1.0);

	path = uiDrawNewPath(uiDrawFillModeWinding);
	// the example doesn't define a rectangle
	// 150x150 seems to be right given the other examples though
	uiDrawPathAddRectangle(path, 25, 25, 150, 150);
	uiDrawPathEnd(path);

	uiDrawFill(p->Context, path, &yellowGreen);
	sp.Thickness = 1.0;
	sp.Cap = uiDrawLineCapFlat;
	sp.Join = uiDrawLineJoinMiter;
	sp.MiterLimit = uiDrawDefaultMiterLimit;
	uiDrawStroke(p->Context, path, &black, &sp);

	uiDrawFreePath(path);
}

// from https://msdn.microsoft.com/en-us/library/windows/desktop/dd756678%28v=vs.85%29.aspx
static void drawD2DLinearBrush(uiAreaDrawParams *p)
{
	uiDrawPath *path;
	uiDrawBrush black;
	uiDrawBrush gradient;
	uiDrawBrushGradientStop stops[2];
	uiDrawStrokeParams sp;

	// TODO transform by (25,25) to leave some room

	gradient.Type = uiDrawBrushTypeLinearGradient;
	gradient.X0 = 0;
	gradient.Y0 = 0;
	gradient.X1 = 150;
	gradient.Y1 = 150;
	stops[0].Pos = 0.0;
	d2dColorToRGB(d2dYellow, &(stops[0].R), &(stops[0].G), &(stops[0].B));
	stops[0].A = 1.0;
	stops[1].Pos = 10;
	d2dColorToRGB(d2dForestGreen, &(stops[1].R), &(stops[1].G), &(stops[1].B));
	stops[1].A = 1.0;
	gradient.Stops = stops;
	gradient.NumStops = 2;

	d2dSolidBrush(&black, d2dBlack, 1.0);

	path = uiDrawNewPath(uiDrawFillModeWinding);
	uiDrawPathAddRectangle(path, 0, 0, 150, 150);
	uiDrawPathEnd(path);

	uiDrawFill(p->Context, path, &gradient);
	sp.Thickness = 1.0;
	sp.Cap = uiDrawLineCapFlat;
	sp.Join = uiDrawLineJoinMiter;
	sp.MiterLimit = uiDrawDefaultMiterLimit;
	uiDrawStroke(p->Context, path, &black, &sp);

	uiDrawFreePath(path);
}

// from https://msdn.microsoft.com/en-us/library/windows/desktop/dd756679%28v=vs.85%29.aspx
// TODO expand this to change the origin point with a mouse click (not in the original but useful to have)
static void drawD2DRadialBrush(uiAreaDrawParams *p)
{
	uiDrawPath *path;
	uiDrawBrush black;
	uiDrawBrush gradient;
	uiDrawBrushGradientStop stops[2];
	uiDrawStrokeParams sp;

	// TODO transform by (25,25) to leave some room

	gradient.Type = uiDrawBrushTypeRadialGradient;
	gradient.X0 = 75;
	gradient.Y0 = 75;
	gradient.X1 = 75;
	gradient.Y1 = 75;
	gradient.OuterRadius = 75;
	stops[0].Pos = 0.0;
	d2dColorToRGB(d2dYellow, &(stops[0].R), &(stops[0].G), &(stops[0].B));
	stops[0].A = 1.0;
	stops[1].Pos = 10;
	d2dColorToRGB(d2dForestGreen, &(stops[1].R), &(stops[1].G), &(stops[1].B));
	stops[1].A = 1.0;
	gradient.Stops = stops;
	gradient.NumStops = 2;

	d2dSolidBrush(&black, d2dBlack, 1.0);

	path = uiDrawNewPath(uiDrawFillModeWinding);
	uiDrawPathNewFigure(path, 150, 75);
	uiDrawPathArcTo(path,
		75, 75,
		75,
		0,
		// TODO why doesn't 360° work
		2 * M_PI - 0.1);
	uiDrawPathEnd(path);

	uiDrawFill(p->Context, path, &gradient);
	sp.Thickness = 1.0;
	sp.Cap = uiDrawLineCapFlat;
	sp.Join = uiDrawLineJoinMiter;
	sp.MiterLimit = uiDrawDefaultMiterLimit;
	uiDrawStroke(p->Context, path, &black, &sp);

	uiDrawFreePath(path);
}

// TODO https://msdn.microsoft.com/en-us/library/windows/desktop/dd756677%28v=vs.85%29.aspx

// TODO? other pages have some of these https://msdn.microsoft.com/en-us/library/windows/desktop/dd756653%28v=vs.85%29.aspx

// from https://msdn.microsoft.com/en-us/library/windows/desktop/ee264309%28v=vs.85%29.aspx
static void drawD2DPathGeometries(uiAreaDrawParams *p)
{
	uiDrawPath *leftMountain;
	uiDrawPath *rightMountain;
	uiDrawPath *sun;
	uiDrawPath *sunRays;
	uiDrawPath *river;
	uiDrawBrush radial;
	uiDrawBrush scene;
	uiDrawStrokeParams sp;
	uiDrawBrushGradientStop stops[2];

	// TODO this is definitely wrong but the example doesn't have the right brush in it
	radial.Type = uiDrawBrushTypeRadialGradient;
	radial.X0 = 75;
	radial.Y0 = 75;
	radial.X1 = 75;
	radial.Y1 = 75;
	radial.OuterRadius = 75;
	stops[0].Pos = 0.0;
	d2dColorToRGB(d2dYellow, &(stops[0].R), &(stops[0].G), &(stops[0].B));
	stops[0].A = 1.0;
	stops[1].Pos = 10;
	d2dColorToRGB(d2dForestGreen, &(stops[1].R), &(stops[1].G), &(stops[1].B));
	stops[1].A = 1.0;
	radial.Stops = stops;
	radial.NumStops = 2;

	leftMountain = uiDrawNewPath(uiDrawFillModeWinding);
	uiDrawPathNewFigure(leftMountain, 346, 255);
	uiDrawPathLineTo(leftMountain, 267, 177);
	uiDrawPathLineTo(leftMountain, 236, 192);
	uiDrawPathLineTo(leftMountain, 212, 160);
	uiDrawPathLineTo(leftMountain, 156, 255);
	uiDrawPathLineTo(leftMountain, 346, 255);
	uiDrawPathCloseFigure(leftMountain);
	uiDrawPathEnd(leftMountain);

	rightMountain = uiDrawNewPath(uiDrawFillModeWinding);
	uiDrawPathNewFigure(rightMountain, 575, 263);
	uiDrawPathLineTo(rightMountain, 481, 146);
	uiDrawPathLineTo(rightMountain, 449, 181);
	uiDrawPathLineTo(rightMountain, 433, 159);
	uiDrawPathLineTo(rightMountain, 401, 214);
	uiDrawPathLineTo(rightMountain, 381, 199);
	uiDrawPathLineTo(rightMountain, 323, 263);
	uiDrawPathLineTo(rightMountain, 575, 263);
	uiDrawPathCloseFigure(rightMountain);
	uiDrawPathEnd(rightMountain);

	sun = uiDrawNewPath(uiDrawFillModeWinding);
	uiDrawPathNewFigureWithArc(sun,
		440.0, 255,
		85,
		0, M_PI);
	uiDrawPathCloseFigure(sun);
	uiDrawPathEnd(sun);

	// the original examples had these as hollow figures
	// we don't support them, so we'll have to stroke it separately
	sunRays = uiDrawNewPath(uiDrawFillModeWinding);
	uiDrawPathNewFigure(sunRays, 299, 182);
	uiDrawPathBezierTo(sunRays,
		299, 182,
		294, 176,
		285, 178);
	uiDrawPathBezierTo(sunRays,
		276, 179,
		272, 173,
		272, 173);
	uiDrawPathNewFigure(sunRays, 354, 156);
	uiDrawPathBezierTo(sunRays,
		354, 156,
		358, 149,
		354, 142);
	uiDrawPathBezierTo(sunRays,
		349, 134,
		354, 127,
		354, 127);
	uiDrawPathNewFigure(sunRays, 322, 164);
	uiDrawPathBezierTo(sunRays,
		322, 164,
		322, 156,
		314, 152);
	uiDrawPathBezierTo(sunRays,
		306, 149,
		305, 141,
		305, 141);
	uiDrawPathNewFigure(sunRays, 385, 164);
	uiDrawPathBezierTo(sunRays,
		385, 164,
		392, 161,
		394, 152);
	uiDrawPathBezierTo(sunRays,
		395, 144,
		402, 141,
		402, 142);
	uiDrawPathNewFigure(sunRays, 408, 182);
	uiDrawPathBezierTo(sunRays,
		408, 182,
		416, 184,
		422, 178);
	uiDrawPathBezierTo(sunRays,
		428, 171,
		435, 173,
		435, 173);
	uiDrawPathEnd(sunRays);

	river = uiDrawNewPath(uiDrawFillModeWinding);
	uiDrawPathNewFigure(river, 183, 392);
	uiDrawPathBezierTo(river,
		238, 284,
		472, 345,
		356, 303);
	uiDrawPathBezierTo(river,
		237, 261,
		333, 256,
		333, 256);
	uiDrawPathBezierTo(river,
		335, 257,
		241, 261,
		411, 306);
	uiDrawPathBezierTo(river,
		574, 350,
		288, 324,
		296, 392);
	uiDrawPathEnd(river);

	d2dClear(p, d2dWhite, 1.0);

	// TODO draw the grid

	uiDrawFill(p->Context, sun, &radial);

	d2dSolidBrush(&scene, d2dBlack, 1.0);
	sp.Thickness = 1.0;
	sp.Cap = uiDrawLineCapFlat;
	sp.Join = uiDrawLineJoinMiter;
	sp.MiterLimit = uiDrawDefaultMiterLimit;
	uiDrawStroke(p->Context, sun, &scene, &sp);
	uiDrawStroke(p->Context, sunRays, &scene, &sp);

	d2dSolidBrush(&scene, d2dOliveDrab, 1.0);
	uiDrawFill(p->Context, leftMountain, &scene);

	d2dSolidBrush(&scene, d2dBlack, 1.0);
	uiDrawStroke(p->Context, leftMountain, &scene, &sp);

	d2dSolidBrush(&scene, d2dLightSkyBlue, 1.0);
	uiDrawFill(p->Context, river, &scene);

	d2dSolidBrush(&scene, d2dBlack, 1.0);
	uiDrawStroke(p->Context, river, &scene, &sp);

	d2dSolidBrush(&scene, d2dYellowGreen, 1.0);
	uiDrawFill(p->Context, rightMountain, &scene);

	d2dSolidBrush(&scene, d2dBlack, 1.0);
	uiDrawStroke(p->Context, rightMountain, &scene, &sp);

	uiDrawFreePath(leftMountain);
	uiDrawFreePath(rightMountain);
	uiDrawFreePath(sun);
	uiDrawFreePath(sunRays);
	uiDrawFreePath(river);
}

// TODO https://msdn.microsoft.com/en-us/library/windows/desktop/dd756690%28v=vs.85%29.aspx

// note: up to https://msdn.microsoft.com/en-us/library/windows/desktop/dd756681%28v=vs.85%29.aspx

static const struct drawtest tests[] = {
	{ "Original uiArea test", drawOriginal },
	{ "Direct2D: Direct2D Quickstart for Windows 8", drawD2DW8QS },
	{ "Direct2D: Creating a Simple Direct2D Application", drawD2DSimpleApp },
	{ "Direct2D: How to Create a Solid Color Brush", drawD2DSolidBrush },
	{ "Direct2D: How to Create a Linear Gradient Brush", drawD2DLinearBrush },
	{ "Direct2D: How to Create a Radial Gradient Brush", drawD2DRadialBrush },
	{ "Direct2D: Path Geometries Overview", drawD2DPathGeometries },
	{ NULL, NULL },
};

void runDrawTest(intmax_t n, uiAreaDrawParams *p)
{
	(*(tests[n].draw))(p);
}

void populateComboboxWithTests(uiCombobox *c)
{
	size_t i;

	for (i = 0; tests[i].name != NULL; i++)
		uiComboboxAppend(c, tests[i].name);
}
