// 7 september 2015
#include "uipriv_windows.h"

static ID2D1Factory *d2dfactory = NULL;

HRESULT initDraw(void)
{
	D2D1_FACTORY_OPTIONS opts;

	ZeroMemory(&opts, sizeof (D2D1_FACTORY_OPTIONS));
	// TODO make this an option
	opts.debugLevel = D2D1_DEBUG_LEVEL_NONE;
	return D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED,
		&IID_ID2D1Factory,
		&opts,
		(void **) (&d2dfactory));
}

void uninitDraw(void)
{
	ID2D1Factory_Release(d2dfactory);
}

ID2D1HwndRenderTarget *makeHWNDRenderTarget(HWND hwnd)
{
	D2D1_RENDER_TARGET_PROPERTIES props;
	D2D1_HWND_RENDER_TARGET_PROPERTIES hprops;
	HDC dc;
	RECT r;
	ID2D1HwndRenderTarget *rt;
	HRESULT hr;

	// we need a DC for the DPI
	// we *could* just use the screen DPI but why when we have a window handle and its DC has a DPI
	dc = GetDC(hwnd);
	if (dc == NULL)
		logLastError("error getting DC to find DPI in makeHWNDRenderTarget()");

	ZeroMemory(&props, sizeof (D2D1_RENDER_TARGET_PROPERTIES));
	props.type = D2D1_RENDER_TARGET_TYPE_DEFAULT;
	props.pixelFormat.format = DXGI_FORMAT_UNKNOWN;
	props.pixelFormat.alphaMode = D2D1_ALPHA_MODE_UNKNOWN;
	props.dpiX = GetDeviceCaps(dc, LOGPIXELSX);
	props.dpiY = GetDeviceCaps(dc, LOGPIXELSY);
	props.usage = D2D1_RENDER_TARGET_USAGE_NONE;
	props.minLevel = D2D1_FEATURE_LEVEL_DEFAULT;

	if (ReleaseDC(hwnd, dc) == 0)
		logLastError("error releasing DC for finding DPI in makeHWNDRenderTarget()");

	if (GetClientRect(hwnd, &r) == 0)
		logLastError("error getting current size of window in makeHWNDRenderTarget()");

	ZeroMemory(&hprops, sizeof (D2D1_HWND_RENDER_TARGET_PROPERTIES));
	hprops.hwnd = hwnd;
	hprops.pixelSize.width = r.right - r.left;
	hprops.pixelSize.height = r.bottom - r.top;
	hprops.presentOptions = D2D1_PRESENT_OPTIONS_NONE;

	hr = ID2D1Factory_CreateHwndRenderTarget(d2dfactory,
		&props,
		&hprops,
		&rt);
	if (hr != S_OK)
		logHRESULT("error creating area HWND render target in makeHWNDRenderTarget()", hr);
	return rt;
}

struct uiDrawPath {
	ID2D1PathGeometry *path;
	ID2D1GeometrySink *sink;
	BOOL inFigure;
};

uiDrawPath *uiDrawNewPath(uiDrawFillMode fillmode)
{
	uiDrawPath *p;
	HRESULT hr;

	p = uiNew(uiDrawPath);
	hr = ID2D1Factory_CreatePathGeometry(d2dfactory,
		&(p->path));
	if (hr != S_OK)
		logHRESULT("error creating path in uiDrawNewPath()", hr);
	hr = ID2D1PathGeometry_Open(p->path,
		&(p->sink));
	if (hr != S_OK)
		logHRESULT("error opening path in uiDrawNewPath()", hr);
	switch (fillmode) {
	case uiDrawFillModeWinding:
		ID2D1GeometrySink_SetFillMode(p->sink,
			D2D1_FILL_MODE_WINDING);
		break;
	case uiDrawFillModeAlternate:
		ID2D1GeometrySink_SetFillMode(p->sink,
			D2D1_FILL_MODE_ALTERNATE);
		break;
	}
p->inFigure = FALSE;
	return p;
}

void uiDrawFreePath(uiDrawPath *p)
{
	if (p->inFigure)
		ID2D1GeometrySink_EndFigure(p->sink,
			D2D1_FIGURE_END_OPEN);
	if (p->sink != NULL)
		// TODO close sink first?
		ID2D1GeometrySink_Release(p->sink);
	ID2D1PathGeometry_Release(p->path);
	uiFree(p);
}

void uiDrawPathNewFigure(uiDrawPath *p, double x, double y)
{
	D2D1_POINT_2F pt;

	if (p->inFigure)
		ID2D1GeometrySink_EndFigure(p->sink,
			D2D1_FIGURE_END_OPEN);
	pt.x = x;
	pt.y = y;
	ID2D1GeometrySink_BeginFigure(p->sink,
		pt,
		D2D1_FIGURE_BEGIN_FILLED);
	p->inFigure = TRUE;
}

static void arcEndpoint(double xCenter, double yCenter, double radius, double startAngle, double *startX, double *startY)
{
	FLOAT sinStart, cosStart;

	// unfortunately D2D1SinCos() is only defined on Windows 8 and newer
	// the MSDN page doesn't say this, but says it requires d2d1_1.h, which is listed as only supported on Windows 8 and newer elsewhere on MSDN
	// so we must use sin() and cos() and hope it's right...
	sinStart = sin(startAngle);
	cosStart = cos(startAngle);
	*startX = xCenter + radius * cosStart;
	*startY = yCenter - radius * sinStart;
}

// An arc in Direct2D is defined by the chord between its endpoints, not solely by the sweep angle.
// There are four possible arcs with the same sweep amount that you can draw this way.
// See https://www.youtube.com/watch?v=ATS0ANW1UxQ for a demonstration.
// TODO clean this up, document it better, and merge it with the above and below functions
static void doDrawArc(ID2D1GeometrySink *sink, double endX, double endY, double radius, double sweep)
{
	D2D1_ARC_SEGMENT as;

	if (sweep > 2 * M_PI)
		sweep = 2 * M_PI;
	as.point.x = endX;
	as.point.y = endY;
	as.size.width = radius;
	as.size.height = radius;
	as.rotationAngle = sweep * (180.0 / M_PI);
	as.sweepDirection = D2D1_SWEEP_DIRECTION_COUNTER_CLOCKWISE;
	if (sweep > M_PI)
		as.arcSize = D2D1_ARC_SIZE_LARGE;
	else
		as.arcSize = D2D1_ARC_SIZE_SMALL;
	ID2D1GeometrySink_AddArc(sink, &as);
}

void uiDrawPathNewFigureWithArc(uiDrawPath *p, double xCenter, double yCenter, double radius, double startAngle, double sweep)
{
	double startX, startY;
	double endX, endY;

	// make the new figure
	arcEndpoint(xCenter, yCenter, radius, startAngle, &startX, &startY);
	uiDrawPathNewFigure(p, startX, startY);

	// draw the arc
	arcEndpoint(xCenter, yCenter, radius, startAngle + sweep, &endX, &endY);
	doDrawArc(p->sink, endX, endY, radius, sweep);
}

void uiDrawPathLineTo(uiDrawPath *p, double x, double y)
{
	D2D1_POINT_2F pt;

	pt.x = x;
	pt.y = y;
	ID2D1GeometrySink_AddLine(p->sink, pt);
}

void uiDrawPathArcTo(uiDrawPath *p, double xCenter, double yCenter, double radius, double startAngle, double sweep)
{
	double startX, startY;
	double endX, endY;

	// draw the starting line
	arcEndpoint(xCenter, yCenter, radius, startAngle, &startX, &startY);
	uiDrawPathLineTo(p, startX, startY);

	// draw the arc
	arcEndpoint(xCenter, yCenter, radius, startAngle + sweep, &endX, &endY);
	doDrawArc(p->sink, endX, endY, radius, sweep);
}

void uiDrawPathBezierTo(uiDrawPath *p, double c1x, double c1y, double c2x, double c2y, double endX, double endY)
{
	D2D1_BEZIER_SEGMENT s;

	s.point1.x = c1x;
	s.point1.y = c1y;
	s.point2.x = c2x;
	s.point2.y = c2y;
	s.point3.x = endX;
	s.point3.y = endY;
	ID2D1GeometrySink_AddBezier(p->sink, &s);
}

void uiDrawPathCloseFigure(uiDrawPath *p)
{
	ID2D1GeometrySink_EndFigure(p->sink,
		D2D1_FIGURE_END_CLOSED);
	p->inFigure = FALSE;
}

void uiDrawPathAddRectangle(uiDrawPath *p, double x, double y, double width, double height)
{
	// this is the same algorithm used by cairo and Core Graphics, according to their documentations
	uiDrawPathNewFigure(p, x, y);
	uiDrawPathLineTo(p, x + width, y);
	uiDrawPathLineTo(p, x + width, y + height);
	uiDrawPathLineTo(p, x, y + height);
	uiDrawPathCloseFigure(p);
}

void uiDrawPathEnd(uiDrawPath *p)
{
	HRESULT hr;

	if (p->inFigure) {
		ID2D1GeometrySink_EndFigure(p->sink,
			D2D1_FIGURE_END_OPEN);
		// needed for uiDrawFreePath()
		p->inFigure = FALSE;
	}
	hr = ID2D1GeometrySink_Close(p->sink);
	if (hr != S_OK)
		logHRESULT("error closing path in uiDrawPathEnd()", hr);
	ID2D1GeometrySink_Release(p->sink);
	p->sink = NULL;
}

struct uiDrawContext {
	ID2D1RenderTarget *rt;
};

uiDrawContext *newContext(ID2D1RenderTarget *rt)
{
	uiDrawContext *c;

	c = uiNew(uiDrawContext);
	c->rt = rt;
	return c;
}

void freeContext(uiDrawContext *c)
{
	uiFree(c);
}

static ID2D1Brush *makeSolidBrush(uiDrawBrush *b, ID2D1RenderTarget *rt, D2D1_BRUSH_PROPERTIES *props)
{
	D2D1_COLOR_F color;
	ID2D1SolidColorBrush *brush;
	HRESULT hr;

	color.r = b->R;
	color.g = b->G;
	color.b = b->B;
	color.a = b->A;

	hr = ID2D1RenderTarget_CreateSolidColorBrush(rt,
		&color,
		props,
		&brush);
	if (hr != S_OK)
		logHRESULT("error creating solid brush in makeSolidBrush()", hr);
	return (ID2D1Brush *) brush;
}

static ID2D1GradientStopCollection *mkstops(uiDrawBrush *b, ID2D1RenderTarget *rt)
{
	ID2D1GradientStopCollection *s;
	D2D1_GRADIENT_STOP *stops;
	size_t i;
	HRESULT hr;

	stops = uiAlloc(b->NumStops * sizeof (D2D1_GRADIENT_STOP), "D2D1_GRADIENT_STOP[]");
	for (i = 0; i < b->NumStops; i++) {
		stops[i].position = b->Stops[i].Pos;
		stops[i].color.r = b->Stops[i].R;
		stops[i].color.g = b->Stops[i].G;
		stops[i].color.b = b->Stops[i].B;
		stops[i].color.a = b->Stops[i].A;
	}

	// TODO BUG IN MINGW
	// the Microsoft headers give this all 6 parameters
	// the MinGW headers use the 4-parameter version
	hr = (*(rt->lpVtbl->CreateGradientStopCollection))(rt,
		stops,
		b->NumStops,
		D2D1_GAMMA_2_2,			// this is the default for the C++-only overload of ID2D1RenderTarget::GradientStopCollection()
		D2D1_EXTEND_MODE_CLAMP,
		&s);
	if (hr != S_OK)
		logHRESULT("error creating stop collection in mkstops()", hr);

	uiFree(stops);
	return s;
}

static ID2D1Brush *makeLinearBrush(uiDrawBrush *b, ID2D1RenderTarget *rt, D2D1_BRUSH_PROPERTIES *props)
{
	ID2D1LinearGradientBrush *brush;
	D2D1_LINEAR_GRADIENT_BRUSH_PROPERTIES gprops;
	ID2D1GradientStopCollection *stops;
	HRESULT hr;

	ZeroMemory(&gprops, sizeof (D2D1_LINEAR_GRADIENT_BRUSH_PROPERTIES));
	gprops.startPoint.x = b->X0;
	gprops.startPoint.y = b->Y0;
	gprops.endPoint.x = b->X1;
	gprops.endPoint.y = b->Y1;

	stops = mkstops(b, rt);

	hr = ID2D1RenderTarget_CreateLinearGradientBrush(rt,
		&gprops,
		props,
		stops,
		&brush);
	if (hr != S_OK)
		logHRESULT("error creating gradient brush in makeLinearBrush()", hr);

	// the example at https://msdn.microsoft.com/en-us/library/windows/desktop/dd756682%28v=vs.85%29.aspx says this is safe to do now
	ID2D1GradientStopCollection_Release(stops);
	return (ID2D1Brush *) brush;
}

static ID2D1Brush *makeRadialBrush(uiDrawBrush *b, ID2D1RenderTarget *rt, D2D1_BRUSH_PROPERTIES *props)
{
	ID2D1RadialGradientBrush *brush;
	D2D1_RADIAL_GRADIENT_BRUSH_PROPERTIES gprops;
	ID2D1GradientStopCollection *stops;
	HRESULT hr;

	ZeroMemory(&gprops, sizeof (D2D1_RADIAL_GRADIENT_BRUSH_PROPERTIES));
	gprops.gradientOriginOffset.x = b->X0 - b->X1;
	gprops.gradientOriginOffset.y = b->Y0 - b->Y1;
	gprops.center.x = b->X1;
	gprops.center.y = b->Y1;
	gprops.radiusX = b->OuterRadius;
	gprops.radiusY = b->OuterRadius;

	stops = mkstops(b, rt);

	hr = ID2D1RenderTarget_CreateRadialGradientBrush(rt,
		&gprops,
		props,
		stops,
		&brush);
	if (hr != S_OK)
		logHRESULT("error creating gradient brush in makeRadialBrush()", hr);

	ID2D1GradientStopCollection_Release(stops);
	return (ID2D1Brush *) brush;
}

static ID2D1Brush *makeBrush(uiDrawBrush *b, ID2D1RenderTarget *rt)
{
	D2D1_BRUSH_PROPERTIES props;

	ZeroMemory(&props, sizeof (D2D1_BRUSH_PROPERTIES));
	props.opacity = 1.0;
	// identity matrix
	props.transform._11 = 1;
	props.transform._22 = 1;

	switch (b->Type) {
	case uiDrawBrushTypeSolid:
		return makeSolidBrush(b, rt, &props);
	case uiDrawBrushTypeLinearGradient:
		return makeLinearBrush(b, rt, &props);
	case uiDrawBrushTypeRadialGradient:
		return makeRadialBrush(b, rt, &props);
//	case uiDrawBrushTypeImage:
//		TODO
	}

	complain("invalid brush type %d in makeBrush()", b->Type);
	return NULL;		// make compiler happy
}

void uiDrawStroke(uiDrawContext *c, uiDrawPath *p, uiDrawBrush *b, uiDrawStrokeParams *sp)
{
	ID2D1Brush *brush;
	ID2D1StrokeStyle *style;
	D2D1_STROKE_STYLE_PROPERTIES dsp;
	HRESULT hr;

	brush = makeBrush(b, c->rt);

	ZeroMemory(&dsp, sizeof (D2D1_STROKE_STYLE_PROPERTIES));
	switch (sp->Cap) {
	case uiDrawLineCapFlat:
		dsp.startCap = D2D1_CAP_STYLE_FLAT;
		dsp.endCap = D2D1_CAP_STYLE_FLAT;
		dsp.dashCap = D2D1_CAP_STYLE_FLAT;
		break;
	case uiDrawLineCapRound:
		dsp.startCap = D2D1_CAP_STYLE_ROUND;
		dsp.endCap = D2D1_CAP_STYLE_ROUND;
		dsp.dashCap = D2D1_CAP_STYLE_ROUND;
		break;
	case uiDrawLineCapSquare:
		dsp.startCap = D2D1_CAP_STYLE_SQUARE;
		dsp.endCap = D2D1_CAP_STYLE_SQUARE;
		dsp.dashCap = D2D1_CAP_STYLE_SQUARE;
		break;
	}
	switch (sp->Join) {
	case uiDrawLineJoinMiter:
		dsp.lineJoin = D2D1_LINE_JOIN_MITER_OR_BEVEL;
		dsp.miterLimit = sp->MiterLimit;
		break;
	case uiDrawLineJoinRound:
		dsp.lineJoin = D2D1_LINE_JOIN_ROUND;
		break;
	case uiDrawLineJoinBevel:
		dsp.lineJoin = D2D1_LINE_JOIN_BEVEL;
		break;
	}
	dsp.dashStyle = D2D1_DASH_STYLE_SOLID;
	dsp.dashOffset = 0;
	hr = ID2D1Factory_CreateStrokeStyle(d2dfactory,
		&dsp,
		NULL,
		0,
		&style);
	if (hr != S_OK)
		logHRESULT("error creating stroke style in uiDrawStroke()", hr);

	ID2D1RenderTarget_DrawGeometry(c->rt,
		(ID2D1Geometry *) (p->path),
		brush,
		sp->Thickness,
		style);

	ID2D1StrokeStyle_Release(style);
	ID2D1Brush_Release(brush);
}

void uiDrawFill(uiDrawContext *c, uiDrawPath *p, uiDrawBrush *b)
{
	ID2D1Brush *brush;

	brush = makeBrush(b, c->rt);
	ID2D1RenderTarget_FillGeometry(c->rt,
		(ID2D1Geometry *) (p->path),
		brush,
		NULL);
	ID2D1Brush_Release(brush);
}
