// 7 september 2015
#include "area.h"

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

	// TODO uiNew
	p = malloc(sizeof (uiDrawPath));
	hr = ID2D1Factory_CreatePathGeometry(d2dfactory,
		&(p->path));
	// TODO make sure this is the only success code
	if (hr != S_OK)
		logHRESULT("error creating path in uiDrawNewPath()", hr);
	hr = ID2D1PathGeometry_Open(p->path,
		&(p->sink));
	if (hr != S_OK)
		logHRESULT("error opening path in uiDrawNewPath()", hr);
	switch (fillmode) {
	case uiDrawFIllModeWinding:
		ID2D1GeometrySink_SetFillMode(s->sink,
			D2D1_FILL_MODE_WINDING);
		break;
	case uiDrawFillModeAlternate:
		ID2D1GeometrySink_SetFillMode(s->sink,
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
	// TODO uiFree
	free(p);
}

void uiDrawPathNewFigure(uiDrawPath *p, double x, double y)
{
	D2D1_POINT_2F pt;

	if (p->inFigure)
		ID2D1GeometrySink_EndFigure(c->sink,
			D2D1_FIGURE_END_OPEN);
	p.x = x;
	p.y = y;
	ID2D1GeometrySink_BeginFigure(p->sink,
		pt,
		D2D1_FIGURE_BEGIN_FILLED);
	p->inFigure = TRUE;
}

static void arcStartXY(double xCenter, double yCenter, double radius, double startAngle, double *startX, double *startY)
{
	FLOAT sinStart, cosStart;

	ID2D1SinCos(startAngle, &sinStart, &cosStart);
	*startX = xCenter + radius * cosStart;
	*startY = yCenter + radius * sinStart;
}

static void doDrawArc(ID2D1GeometrySink *sink, double xCenter, double yCenter, double radius, double startAngle, double endAngle)
{
	double delta;
	double add;
	D2D1_ARC_SEGMENT as;
	double endX, endY;

	delta = endAngle - startAngle;
	add = M_PI;
	// TODO why do I have to do this? if delta == 360 nothing gets drawn regardless of parameter values
	while (delta > M_PI) {
		// this line alternates between getting the 180 degree and 360 degree point; the add variable does the alternating
		arcStartXY(xCenter, yCenter, radius, startAngle + add, &endX, &endY);
		as.point.x = endX;
		as.point.y = endY;
		as.size.width = radius;
		as.size.height = radius;
		as.rotationAngle = 180;
		as.sweepDirection = D2D1_SWEEP_DIRECTION_COUNTER_CLOCKWISE;
		as.arcSize = D2D1_ARC_SIZE_SMALL;
		ID2D1GeometrySink_AddArc(sink, &as);
		delta -= M_PI;
		add += M_PI;
	}

	arcStartXY(xCenter, yCenter, radius, endAngle, &endX, &endY);
	as.point.x = endX;
	as.point.y = endY;
	as.size.width = radius;
	as.size.height = radius;
	// Direct2D expects degrees
	as.rotationAngle = delta * (180.0 / M_PI);
	as.sweepDirection = D2D1_SWEEP_DIRECTION_COUNTER_CLOCKWISE;
	as.arcSize = D2D1_ARC_SIZE_SMALL;
	ID2D1GeometrySink_AddArc(sink, &as);
}

void uiDrawPathNewFigureWIthArc(uiDrawPath *p, double xCenter, double yCenter, double radius, double startAngle, double endAngle)
{
	double startX, startY;

	// make the new figure
	arcStartXY(xCenter, yCenter, radius, startAngle, &startX, &startY);
	uiDrawPathNewFigure(p, startX, startY);

	// draw the arc
	doDrawArc(p->sink, xCenter, yCenter, radius, startAngle, endAngle);
}

void uiDrawPathLineTo(uiDrawPath *p, double x, double y)
{
	D2D1_POINT_2F pt;

	pt.x = x;
	pt.y = y;
	ID2D1GeometrySink_AddLine(p->sink, pt);
}

void uiDrawPathArcTo(uiDrawPath *p, double xCenter, double yCenter, double radius, double startAngle, double endAngle)
{
	double startX, startY;

	// draw the starting line
	arcStartXY(xCenter, yCenter, radius, startAngle, &startX, &startY);
	uiDrawPathLineTo(p, startX, startY);

	// draw the arc
	doDrawArc(p->sink, xCenter, yCenter, radius, startAngle, endAngle);
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

void uiDrawPathAddRectangle(uiDrawPath *p, double x0, double y0, double xxxxTODOxxxx, double xxxTODOxxxxxxxxxx)
{
	// TODO
}

void uiDrawPathEnd(uiDrawPath *p)
{
	HRESULT hr;

	if (p->inFigure)
		ID2D1GeometrySink_EndFigure(c->sink,
			D2D1_FIGURE_END_OPEN);
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

	// TODO use uiNew
	c = (uiDrawContext *) malloc(sizeof (uiDrawContext));
	c->rt = rt;
	return c;
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
		&props,
		&brush);
	if (hr != S_OK)
		logHRESULT("error creating solid brush in makeSolidBrush()", hr);
	return (ID2D1Brush *) brush;
}

/* TODO
static ID2D1Brush *makeLinearBrush(uiDrawBrush *b, ID2D1RenderTarget *rt, D2D1_BRUSH_PROPERTIES *props)
{
}

static ID2D1Brush *makeRadialBrush(uiDrawBrush *b, ID2D1RenderTarget *rt, D2D1_BRUSH_PROPERTIES *props)
{
}
*/

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
//	case uiDrawBrushTypeLinearGradient:
//		return makeLinearBrush(b, rt, &props);
//	case uiDrawBrushTypeRadialGradient:
//		return makeRadialBrush(b, rt, &props);
//	case uiDrawBrushTypeImage:
//		TODO
	}

	complain("invalid brush type %d in makeBrush()", b->Type);
	return NULL;		// make compiler happy
}

void uiDrawStroke(uiDrawContext *c, uiDrawPath *p, uiDrawBrush *b, uiDrawStrokeParams *sp)
{
	ID2D1Brush *brush;
	HRESULT hr;

	brush = makeBrush(b, c->rt);
	ID2D1RenderTarget_DrawGeometry(c->rt,
		(ID2D1Geometry *) (p->path),
		brush,
		sp->Thickness,
		NULL);
	ID2D1Brush_Release(brush);
}

void uiDrawFill(uiDrawContext *c, uiDrawPath *p, uiDrawBrush *b)
{
	ID2D1Brush *brush;
	HRESULT hr;

	brush = makeBrush(b, c->rt);
	ID2D1RenderTarget_FillGeometry(c->rt,
		(ID2D1Geometry *) (p->path),
		brush,
		NULL);
	ID2D1Brush_Release(brush);
}
