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

struct uiDrawContext {
	ID2D1RenderTarget *rt;

	ID2D1PathGeometry *path;
	ID2D1GeometrySink *sink;
	BOOL inFigure;

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

void uiDrawMoveTo(uiDrawContext *c, intmax_t x, intmax_t y)
{
	D2D1_POINT_2F p;

	if (c->inFigure)
		ID2D1GeometrySink_EndFigure(c->sink,
			D2D1_FIGURE_END_OPEN);
	p.x = ((FLOAT) x) + 0.5;
	p.y = ((FLOAT) y) + 0.5;
	ID2D1GeometrySink_BeginFigure(c->sink,
		p,
		D2D1_FIGURE_BEGIN_FILLED);
	c->inFigure = TRUE;
}

void uiDrawLineTo(uiDrawContext *c, intmax_t x, intmax_t y)
{
	D2D1_POINT_2F p;

	p.x = ((FLOAT) x) + 0.5;
	p.y = ((FLOAT) y) + 0.5;
	ID2D1GeometrySink_AddLine(c->sink, p);
}

void uiDrawRectangle(uiDrawContext *c, intmax_t x, intmax_t y, intmax_t width, intmax_t height)
{
	// TODO
}

void uiDrawArcTo(uiDrawContext *c, intmax_t xCenter, intmax_t yCenter, intmax_t radius, double startAngle, double endAngle, int lineFromCurrentPointToStart)
{
	if (!lineFromCurrentPointToStart) {
		int bx, by, bx2, by2;
		int sx, sy, ex, ey;

		// see http://stackoverflow.com/questions/32465446/how-do-i-inhibit-the-initial-line-segment-of-an-anglearc
		// the idea for floor(x + 0.5) is inspired by wine
		// TODO make sure this is an accurate imitation of AngleArc()
		bx = xCenter - radius;
		by = yCenter - radius;
		bx2 = xCenter + radius;
		by2 = yCenter + radius;
		sx = xCenter + floor((double) radius * cos(startAngle));
		sy = yCenter - floor((double) radius * sin(startAngle));
		ex = xCenter + floor((double) radius * cos(endAngle));
		ey = yCenter - floor((double) radius * sin(endAngle));
/* TODO
		if (Arc(c->dc, bx, by, bx2, by2, sx, sy, ex, ey) == 0)
			logLastError("error drawing current point arc in uiDrawArc()");
		return;
*/
	}
/* TODO
	// AngleArc() expects degrees
	startAngle *= (180.0 / M_PI);
	endAngle *= (180.0 / M_PI);
	if (AngleArc(c->dc,
		xCenter, yCenter,
		radius,
		startAngle,
		// the "sweep angle" is relative to the start angle, not to 0
		endAngle - startAngle) == 0)
		logLastError("error drawing arc in uiDrawArc()");
*/
}

void uiDrawBezierTo(uiDrawContext *c, intmax_t c1x, intmax_t c1y, intmax_t c2x, intmax_t c2y, intmax_t endX, intmax_t endY)
{
	// TODO
}

void uiDrawCloseFigure(uiDrawContext *c)
{
	ID2D1GeometrySink_EndFigure(c->sink,
		D2D1_FIGURE_END_CLOSED);
	c->inFigure = FALSE;
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
