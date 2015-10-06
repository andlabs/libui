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

// TODO move below the path stuff
struct uiDrawContext {
	ID2D1RenderTarget *rt;

	// source color
	BOOL useRGB;
	uint8_t r;
	uint8_t g;
	uint8_t b;
	uint8_t a;
};

uiDrawContext *newContext(ID2D1RenderTarget *rt)
{
	uiDrawContext *c;

	// TODO use uiNew
	c = (uiDrawContext *) malloc(sizeof (uiDrawContext));
	c->rt = rt;
	return c;
}

void uiDrawBeginPathRGB(uiDrawContext *c, uint8_t r, uint8_t g, uint8_t b)
{
	uiDrawBeginPathRGBA(c, r, g, b, 1);
}

void uiDrawBeginPathRGBA(uiDrawContext *c, uint8_t r, uint8_t g, uint8_t b, uint8_t a)
{
	HRESULT hr;

	c->useRGB = TRUE;
	c->r = r;
	c->g = g;
	c->b = b;
	c->a = a;

	hr = ID2D1Factory_CreatePathGeometry(d2dfactory, &(c->path));
	if (hr != S_OK)
		logHRESULT("error creating path in uiDrawBeginPathRGBA()", hr);
	hr = ID2D1PathGeometry_Open(c->path, &(c->sink));
	if (hr != S_OK)
		logHRESULT("error opening path sink in uiDrawBeginPathRGBA()", hr);
	c->inFigure = FALSE;
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

void uiDrawPathLineTo(uiDrawPath *p, double x, double y)
{
	D2D1_POINT_2F pt;

	pt.x = x;
	pt.y = y;
	ID2D1GeometrySink_AddLine(p->sink, pt);
}

void uiDrawPathArcTo(uiDrawPath *p, double xCenter, double yCenter, double radius, double startAngle, double endAngle)
{
	// TODO
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

static ID2D1SolidColorBrush *makeBrush(uiDrawContext *c)
{
	D2D1_COLOR_F color;
	D2D1_BRUSH_PROPERTIES props;
	ID2D1SolidColorBrush *brush;
	HRESULT hr;

	color.r = ((FLOAT) (c->r)) / 255;
	color.g = ((FLOAT) (c->g)) / 255;
	color.b = ((FLOAT) (c->b)) / 255;
	color.a = ((FLOAT) (c->a)) / 255;

	ZeroMemory(&props, sizeof (D2D1_BRUSH_PROPERTIES));
	props.opacity = 1.0;
	// identity matrix
	props.transform._11 = 1;
	props.transform._22 = 1;

	hr = ID2D1RenderTarget_CreateSolidColorBrush(c->rt,
		&color,
		&props,
		&brush);
	if (hr != S_OK)
		logHRESULT("error creating brush in makeBrush()", hr);
	return brush;
}

void uiDrawStroke(uiDrawContext *c, uiDrawStrokeParams *p)
{
	ID2D1SolidColorBrush *brush;
	HRESULT hr;

	if (c->inFigure)
		ID2D1GeometrySink_EndFigure(c->sink,
			D2D1_FIGURE_END_OPEN);
	hr = ID2D1GeometrySink_Close(c->sink);
	if (hr != S_OK)
		logHRESULT("error closing geometry sink in uiDrawStroke()", hr);
	brush = makeBrush(c);
	ID2D1RenderTarget_DrawGeometry(c->rt,
		(ID2D1Geometry *) (c->path),
		(ID2D1Brush *) brush,
		p->Thickness,
		NULL);
	ID2D1SolidColorBrush_Release(brush);
	ID2D1GeometrySink_Release(c->sink);
	ID2D1PathGeometry_Release(c->path);
}

void uiDrawFill(uiDrawContext *c, uiDrawFillMode mode)
{
	ID2D1SolidColorBrush *brush;
	HRESULT hr;

	if (c->inFigure)
		ID2D1GeometrySink_EndFigure(c->sink,
			D2D1_FIGURE_END_OPEN);
	hr = ID2D1GeometrySink_Close(c->sink);
	if (hr != S_OK)
		logHRESULT("error closing geometry sink in uiDrawStroke()", hr);
	brush = makeBrush(c);
	ID2D1RenderTarget_FillGeometry(c->rt,
		(ID2D1Geometry *) (c->path),
		(ID2D1Brush *) brush,
		NULL);
	ID2D1SolidColorBrush_Release(brush);
	ID2D1GeometrySink_Release(c->sink);
	ID2D1PathGeometry_Release(c->path);
}
