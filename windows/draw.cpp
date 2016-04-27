// 7 september 2015
#include "uipriv_windows.hpp"

// TODO
// - write a test for transform followed by clip and clip followed by transform to make sure they work the same as on gtk+ and cocoa

static ID2D1Factory *d2dfactory = NULL;

HRESULT initDraw(void)
{
	D2D1_FACTORY_OPTIONS opts;

	ZeroMemory(&opts, sizeof (D2D1_FACTORY_OPTIONS));
	// TODO make this an option
	opts.debugLevel = D2D1_DEBUG_LEVEL_NONE;
	return D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED,
		IID_ID2D1Factory,
		&opts,
		(void **) (&d2dfactory));
}

void uninitDraw(void)
{
	d2dfactory->Release();
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
		logLastError(L"error getting DC to find DPI");

	ZeroMemory(&props, sizeof (D2D1_RENDER_TARGET_PROPERTIES));
	props.type = D2D1_RENDER_TARGET_TYPE_DEFAULT;
	props.pixelFormat.format = DXGI_FORMAT_UNKNOWN;
	props.pixelFormat.alphaMode = D2D1_ALPHA_MODE_UNKNOWN;
	props.dpiX = GetDeviceCaps(dc, LOGPIXELSX);
	props.dpiY = GetDeviceCaps(dc, LOGPIXELSY);
	props.usage = D2D1_RENDER_TARGET_USAGE_NONE;
	props.minLevel = D2D1_FEATURE_LEVEL_DEFAULT;

	if (ReleaseDC(hwnd, dc) == 0)
		logLastError(L"error releasing DC for finding DPI");

	if (getClientRect(hwnd, &r) == 0)
		logLastError(L"error getting current size of window");

	ZeroMemory(&hprops, sizeof (D2D1_HWND_RENDER_TARGET_PROPERTIES));
	hprops.hwnd = hwnd;
	hprops.pixelSize.width = r.right - r.left;
	hprops.pixelSize.height = r.bottom - r.top;
	// according to Rick Brewster, some drivers will misbehave if we don't specify this (see http://stackoverflow.com/a/33222983/3408572)
	hprops.presentOptions = D2D1_PRESENT_OPTIONS_RETAIN_CONTENTS;

	hr = d2dfactory->CreateHwndRenderTarget(
		&props,
		&hprops,
		&rt);
	if (hr != S_OK)
		logHRESULT(L"error creating area HWND render target", hr);
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
	hr = d2dfactory->CreatePathGeometry(&(p->path));
	if (hr != S_OK)
		logHRESULT(L"error creating path", hr);
	hr = p->path->Open(&(p->sink));
	if (hr != S_OK)
		logHRESULT(L"error opening path", hr);
	switch (fillmode) {
	case uiDrawFillModeWinding:
		p->sink->SetFillMode(D2D1_FILL_MODE_WINDING);
		break;
	case uiDrawFillModeAlternate:
		p->sink->SetFillMode(D2D1_FILL_MODE_ALTERNATE);
		break;
	}
	return p;
}

void uiDrawFreePath(uiDrawPath *p)
{
	if (p->inFigure)
		p->sink->EndFigure(D2D1_FIGURE_END_OPEN);
	if (p->sink != NULL)
		// TODO close sink first?
		p->sink->Release();
	p->path->Release();
	uiFree(p);
}

void uiDrawPathNewFigure(uiDrawPath *p, double x, double y)
{
	D2D1_POINT_2F pt;

	if (p->inFigure)
		p->sink->EndFigure(D2D1_FIGURE_END_OPEN);
	pt.x = x;
	pt.y = y;
	p->sink->BeginFigure(pt, D2D1_FIGURE_BEGIN_FILLED);
	p->inFigure = TRUE;
}

// Direct2D arcs require a little explanation.
// An arc in Direct2D is defined by the chord between the endpoints.
// There are four possible arcs with the same two endpoints that you can draw this way.
// See https://www.youtube.com/watch?v=ATS0ANW1UxQ for a demonstration.
// There is a property rotationAngle which deals with the rotation /of the entire ellipse that forms an ellpitical arc/ - it's effectively a transformation on the arc.
// That is to say, it's NOT THE SWEEP.
// The sweep is defined by the start and end points and whether the arc is "large".
// As a result, this design does not allow for full circles or ellipses with a single arc; they have to be simulated with two.

struct arc {
	double xCenter;
	double yCenter;
	double radius;
	double startAngle;
	double sweep;
	int negative;
};

// this is used for the comparison below
// if it falls apart it can be changed later
#define aerMax 6 * DBL_EPSILON

static void drawArc(uiDrawPath *p, struct arc *a, void (*startFunction)(uiDrawPath *, double, double))
{
	double sinx, cosx;
	double startX, startY;
	double endX, endY;
	D2D1_ARC_SEGMENT as;
	BOOL fullCircle;
	double absSweep;

	// as above, we can't do a full circle with one arc
	// simulate it with two half-circles
	// of course, we have a dragon: equality on floating-point values!
	// I've chosen to do the AlmostEqualRelative() technique in https://randomascii.wordpress.com/2012/02/25/comparing-floating-point-numbers-2012-edition/
	fullCircle = FALSE;
	// use the absolute value to tackle both ≥2π and ≤-2π at the same time
	absSweep = fabs(a->sweep);
	if (absSweep > (2 * M_PI))		// this part is easy
		fullCircle = TRUE;
	else {
		double aerDiff;

		aerDiff = fabs(absSweep - (2 * M_PI));
		// if we got here then we know a->sweep is larger (or the same!)
		fullCircle = aerDiff <= absSweep * aerMax;
	}
	// TODO make sure this works right for the negative direction
	if (fullCircle) {
		a->sweep = M_PI;
		drawArc(p, a, startFunction);
		a->startAngle += M_PI;
		drawArc(p, a, NULL);
		return;
	}

	// first, figure out the arc's endpoints
	// unfortunately D2D1SinCos() is only defined on Windows 8 and newer
	// the MSDN page doesn't say this, but says it requires d2d1_1.h, which is listed as only supported on Windows 8 and newer elsewhere on MSDN
	// so we must use sin() and cos() and hope it's right...
	sinx = sin(a->startAngle);
	cosx = cos(a->startAngle);
	startX = a->xCenter + a->radius * cosx;
	startY = a->yCenter + a->radius * sinx;
	sinx = sin(a->startAngle + a->sweep);
	cosx = cos(a->startAngle + a->sweep);
	endX = a->xCenter + a->radius * cosx;
	endY = a->yCenter + a->radius * sinx;

	// now do the initial step to get the current point to be the start point
	// this is either creating a new figure, drawing a line, or (in the case of our full circle code above) doing nothing
	if (startFunction != NULL)
		(*startFunction)(p, startX, startY);

	// now we can draw the arc
	as.point.x = endX;
	as.point.y = endY;
	as.size.width = a->radius;
	as.size.height = a->radius;
	as.rotationAngle = 0;		// as above, not relevant for circles
	if (a->negative)
		as.sweepDirection = D2D1_SWEEP_DIRECTION_COUNTER_CLOCKWISE;
	else
		as.sweepDirection = D2D1_SWEEP_DIRECTION_CLOCKWISE;
	// TODO explain the outer if
	if (!a->negative)
		if (a->sweep > M_PI)
			as.arcSize = D2D1_ARC_SIZE_LARGE;
		else
			as.arcSize = D2D1_ARC_SIZE_SMALL;
	else
		// TODO especially this part
		if (a->sweep > M_PI)
			as.arcSize = D2D1_ARC_SIZE_SMALL;
		else
			as.arcSize = D2D1_ARC_SIZE_LARGE;
	p->sink->AddArc(&as);
}

void uiDrawPathNewFigureWithArc(uiDrawPath *p, double xCenter, double yCenter, double radius, double startAngle, double sweep, int negative)
{
	struct arc a;

	a.xCenter = xCenter;
	a.yCenter = yCenter;
	a.radius = radius;
	a.startAngle = startAngle;
	a.sweep = sweep;
	a.negative = negative;
	drawArc(p, &a, uiDrawPathNewFigure);
}

void uiDrawPathLineTo(uiDrawPath *p, double x, double y)
{
	D2D1_POINT_2F pt;

	pt.x = x;
	pt.y = y;
	p->sink->AddLine(pt);
}

void uiDrawPathArcTo(uiDrawPath *p, double xCenter, double yCenter, double radius, double startAngle, double sweep, int negative)
{
	struct arc a;

	a.xCenter = xCenter;
	a.yCenter = yCenter;
	a.radius = radius;
	a.startAngle = startAngle;
	a.sweep = sweep;
	a.negative = negative;
	drawArc(p, &a, uiDrawPathLineTo);
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
	p->sink->AddBezier(&s);
}

void uiDrawPathCloseFigure(uiDrawPath *p)
{
	p->sink->EndFigure(D2D1_FIGURE_END_CLOSED);
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
		p->sink->EndFigure(D2D1_FIGURE_END_OPEN);
		// needed for uiDrawFreePath()
		p->inFigure = FALSE;
	}
	hr = p->sink->Close();
	if (hr != S_OK)
		logHRESULT(L"error closing path", hr);
	p->sink->Release();
	// also needed for uiDrawFreePath()
	p->sink = NULL;
}

struct uiDrawContext {
	ID2D1RenderTarget *rt;
	// TODO move to std::vector
	struct ptrArray *states;
	ID2D1PathGeometry *currentClip;
};

// declared below
static void initStates(uiDrawContext *);
static void uninitStates(uiDrawContext *);

static void resetTarget(ID2D1RenderTarget *rt)
{
	D2D1_MATRIX_3X2_F dm;

	// transformations persist
	// reset to the identity matrix
	ZeroMemory(&dm, sizeof (D2D1_MATRIX_3X2_F));
	dm._11 = 1;
	dm._22 = 1;
	rt->SetTransform(&dm);
}

uiDrawContext *newContext(ID2D1RenderTarget *rt)
{
	uiDrawContext *c;

	c = uiNew(uiDrawContext);
	c->rt = rt;
	initStates(c);
	resetTarget(c->rt);
	return c;
}

void freeContext(uiDrawContext *c)
{
	if (c->currentClip != NULL)
		c->currentClip->Release();
	uninitStates(c);
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

	hr = rt->CreateSolidColorBrush(
		&color,
		props,
		&brush);
	if (hr != S_OK)
		logHRESULT(L"error creating solid brush", hr);
	return brush;
}

static ID2D1GradientStopCollection *mkstops(uiDrawBrush *b, ID2D1RenderTarget *rt)
{
	ID2D1GradientStopCollection *s;
	D2D1_GRADIENT_STOP *stops;
	size_t i;
	HRESULT hr;

	stops = (D2D1_GRADIENT_STOP *) uiAlloc(b->NumStops * sizeof (D2D1_GRADIENT_STOP), "D2D1_GRADIENT_STOP[]");
	for (i = 0; i < b->NumStops; i++) {
		stops[i].position = b->Stops[i].Pos;
		stops[i].color.r = b->Stops[i].R;
		stops[i].color.g = b->Stops[i].G;
		stops[i].color.b = b->Stops[i].B;
		stops[i].color.a = b->Stops[i].A;
	}

	hr = rt->CreateGradientStopCollection(
		stops,
		b->NumStops,
		D2D1_GAMMA_2_2,			// this is the default for the C++-only overload of ID2D1RenderTarget::GradientStopCollection()
		D2D1_EXTEND_MODE_CLAMP,
		&s);
	if (hr != S_OK)
		logHRESULT(L"error creating stop collection", hr);

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

	hr = rt->CreateLinearGradientBrush(
		&gprops,
		props,
		stops,
		&brush);
	if (hr != S_OK)
		logHRESULT(L"error creating gradient brush", hr);

	// the example at https://msdn.microsoft.com/en-us/library/windows/desktop/dd756682%28v=vs.85%29.aspx says this is safe to do now
	stops->Release();
	return brush;
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

	hr = rt->CreateRadialGradientBrush(
		&gprops,
		props,
		stops,
		&brush);
	if (hr != S_OK)
		logHRESULT(L"error creating gradient brush", hr);

	stops->Release();
	return brush;
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

// how clipping works:
// every fill and stroke is done on a temporary layer with the clip geometry applied to it
// this is really the only way to clip in Direct2D that doesn't involve opacity images
// reference counting:
// - initially the clip is NULL, which means do not use a layer
// - the first time uiDrawClip() is called, we take a reference on the path passed in (this is also why uiPathEnd() is needed)
// - every successive time, we create a new PathGeometry and merge the current clip with the new path, releasing the reference we took earlier and taking a reference to the new one
// - in Save, we take another reference; in Restore we drop the refernece to the existing path geometry and transfer that saved ref to the new path geometry over to the context
// uiDrawFreePath() doesn't destroy the path geometry, it just drops the reference count, so a clip can exist independent of its path

static ID2D1Layer *applyClip(uiDrawContext *c)
{
	ID2D1Layer *layer;
	D2D1_LAYER_PARAMETERS params;
	HRESULT hr;

	// if no clip, don't do anything
	if (c->currentClip == NULL)
		return NULL;

	// create a layer for clipping
	// we have to explicitly make the layer because we're still targeting Windows 7
	hr = c->rt->CreateLayer(NULL, &layer);
	if (hr != S_OK)
		logHRESULT(L"error creating clip layer", hr);

	// apply it as the clip
	ZeroMemory(&params, sizeof (D2D1_LAYER_PARAMETERS));
	// this is the equivalent of InfiniteRect() in d2d1helper.h
	params.contentBounds.left = -FLT_MAX;
	params.contentBounds.top = -FLT_MAX;
	params.contentBounds.right = FLT_MAX;
	params.contentBounds.bottom = FLT_MAX;
	params.geometricMask = (ID2D1Geometry *) (c->currentClip);
	// TODO is this correct?
	params.maskAntialiasMode = c->rt->GetAntialiasMode();
	// identity matrix
	params.maskTransform._11 = 1;
	params.maskTransform._22 = 1;
	params.opacity = 1.0;
	params.opacityBrush = NULL;
	params.layerOptions = D2D1_LAYER_OPTIONS_NONE;
	// TODO is this correct?
	if (c->rt->GetTextAntialiasMode() == D2D1_TEXT_ANTIALIAS_MODE_CLEARTYPE)
		params.layerOptions = D2D1_LAYER_OPTIONS_INITIALIZE_FOR_CLEARTYPE;
	c->rt->PushLayer(&params, layer);

	// return the layer so it can be freed later
	return layer;
}

static void unapplyClip(uiDrawContext *c, ID2D1Layer *layer)
{
	if (layer == NULL)
		return;
	c->rt->PopLayer();
	layer->Release();
}

void uiDrawStroke(uiDrawContext *c, uiDrawPath *p, uiDrawBrush *b, uiDrawStrokeParams *sp)
{
	ID2D1Brush *brush;
	ID2D1StrokeStyle *style;
	D2D1_STROKE_STYLE_PROPERTIES dsp;
	FLOAT *dashes;
	size_t i;
	ID2D1Layer *cliplayer;
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
	dashes = NULL;
	// note that dash widths and the dash phase are scaled up by the thickness by Direct2D
	// TODO be sure to formally document this
	if (sp->NumDashes != 0) {
		dsp.dashStyle = D2D1_DASH_STYLE_CUSTOM;
		dashes = (FLOAT *) uiAlloc(sp->NumDashes * sizeof (FLOAT), "FLOAT[]");
		for (i = 0; i < sp->NumDashes; i++)
			dashes[i] = sp->Dashes[i] / sp->Thickness;
	}
	dsp.dashOffset = sp->DashPhase / sp->Thickness;
	hr = d2dfactory->CreateStrokeStyle(
		&dsp,
		dashes,
		sp->NumDashes,
		&style);
	if (hr != S_OK)
		logHRESULT(L"error creating stroke style", hr);
	if (sp->NumDashes != 0)
		uiFree(dashes);

	cliplayer = applyClip(c);
	c->rt->DrawGeometry(
		(ID2D1Geometry *) (p->path),
		brush,
		sp->Thickness,
		style);
	unapplyClip(c, cliplayer);

	style->Release();
	brush->Release();
}

void uiDrawFill(uiDrawContext *c, uiDrawPath *p, uiDrawBrush *b)
{
	ID2D1Brush *brush;
	ID2D1Layer *cliplayer;

	brush = makeBrush(b, c->rt);
	cliplayer = applyClip(c);
	c->rt->FillGeometry(
		(ID2D1Geometry *) (p->path),
		brush,
		NULL);
	unapplyClip(c, cliplayer);
	brush->Release();
}

static void m2d(uiDrawMatrix *m, D2D1_MATRIX_3X2_F *d)
{
	d->_11 = m->M11;
	d->_12 = m->M12;
	d->_21 = m->M21;
	d->_22 = m->M22;
	d->_31 = m->M31;
	d->_32 = m->M32;
}

static void d2m(D2D1_MATRIX_3X2_F *d, uiDrawMatrix *m)
{
	m->M11 = d->_11;
	m->M12 = d->_12;
	m->M21 = d->_21;
	m->M22 = d->_22;
	m->M31 = d->_31;
	m->M32 = d->_32;
}

void uiDrawMatrixSetIdentity(uiDrawMatrix *m)
{
	setIdentity(m);
}

// frustratingly all of the operations on a matrix except rotation and skeweing are provided by the C++-only d2d1helper.h
// we'll have to recreate their functionalities here
// the implementations are all in the main matrix.c file
// TODO switch to these instead actually

void uiDrawMatrixTranslate(uiDrawMatrix *m, double x, double y)
{
	fallbackTranslate(m, x, y);
}

void uiDrawMatrixScale(uiDrawMatrix *m, double xCenter, double yCenter, double x, double y)
{
	fallbackScale(m, xCenter, yCenter, x, y);
}

void uiDrawMatrixRotate(uiDrawMatrix *m, double x, double y, double amount)
{
	D2D1_POINT_2F center;
	D2D1_MATRIX_3X2_F dm;
	uiDrawMatrix rm;

	amount *= 180 / M_PI;		// must be in degrees
	center.x = x;
	center.y = y;
	D2D1MakeRotateMatrix(amount, center, &dm);
	d2m(&dm, &rm);
	uiDrawMatrixMultiply(m, &rm);
}

void uiDrawMatrixSkew(uiDrawMatrix *m, double x, double y, double xamount, double yamount)
{
	D2D1_POINT_2F center;
	D2D1_MATRIX_3X2_F dm;
	uiDrawMatrix sm;

	xamount *= 180 / M_PI;		// must be in degrees
	yamount *= 180 / M_PI;		// must be in degrees
	center.x = x;
	center.y = y;
	D2D1MakeSkewMatrix(xamount, yamount, center, &dm);
	d2m(&dm, &sm);
	uiDrawMatrixMultiply(m, &sm);
}

void uiDrawMatrixMultiply(uiDrawMatrix *dest, uiDrawMatrix *src)
{
	fallbackMultiply(dest, src);
}

int uiDrawMatrixInvertible(uiDrawMatrix *m)
{
	D2D1_MATRIX_3X2_F d;

	m2d(m, &d);
	return D2D1IsMatrixInvertible(&d) != FALSE;
}

int uiDrawMatrixInvert(uiDrawMatrix *m)
{
	D2D1_MATRIX_3X2_F d;

	m2d(m, &d);
	if (D2D1InvertMatrix(&d) == FALSE)
		return 0;
	d2m(&d, m);
	return 1;
}

void uiDrawMatrixTransformPoint(uiDrawMatrix *m, double *x, double *y)
{
	fallbackTransformPoint(m, x, y);
}

void uiDrawMatrixTransformSize(uiDrawMatrix *m, double *x, double *y)
{
	fallbackTransformSize(m, x, y);
}

// TODO move to using d2d matrices exclusively
void uiDrawTransform(uiDrawContext *c, uiDrawMatrix *m)
{
	D2D1_MATRIX_3X2_F dm;
	uiDrawMatrix already;
	uiDrawMatrix temp;

	c->rt->GetTransform(&dm);
	d2m(&dm, &already);
	temp = *m;		// don't modify m
	// you would think we have to do already * m, right?
	// WRONG! we have to do m * already
	// why? a few reasons
	// a) this lovely comment in cairo's source - http://cgit.freedesktop.org/cairo/tree/src/cairo-matrix.c?id=0537479bd1d4c5a3bc0f6f41dec4deb98481f34a#n330
	// 	Direct2D uses column vectors and I don't know if this is even documented
	// b) that's what Core Graphics does
	// TODO see if Microsoft says to do this
	uiDrawMatrixMultiply(&temp, &already);
	m2d(&temp, &dm);
	c->rt->SetTransform(&dm);
}

// TODO for this, stroke, and fill, make sure that a non-ended uiDrawPath causes error state
void uiDrawClip(uiDrawContext *c, uiDrawPath *path)
{
	ID2D1PathGeometry *newPath;
	ID2D1GeometrySink *newSink;
	HRESULT hr;

	// if there's no current clip, borrow the path
	if (c->currentClip == NULL) {
		c->currentClip = path->path;
		// we have to take our own reference to that clip
		c->currentClip->AddRef();
		return;
	}

	// otherwise we have to intersect the current path with the new one
	// we do that into a new path, and then replace c->currentClip with that new path
	hr = d2dfactory->CreatePathGeometry(&newPath);
	if (hr != S_OK)
		logHRESULT(L"error creating new path", hr);
	hr = newPath->Open(&newSink);
	if (hr != S_OK)
		logHRESULT(L"error opening new path", hr);
	hr = c->currentClip->CombineWithGeometry(
		path->path,
		D2D1_COMBINE_MODE_INTERSECT,
		NULL,
		// TODO is this correct or can this be set per target?
		D2D1_DEFAULT_FLATTENING_TOLERANCE,
		newSink);
	if (hr != S_OK)
		logHRESULT(L"error intersecting old path with new path", hr);
	hr = newSink->Close();
	if (hr != S_OK)
		logHRESULT(L"error closing new path", hr);
	newSink->Release();

	// okay we have the new clip; we just need to replace the old one with it
	c->currentClip->Release();
	c->currentClip = newPath;
	// we have a reference already; no need for another
}

struct state {
	ID2D1DrawingStateBlock *dsb;
	ID2D1PathGeometry *clip;
};

static void initStates(uiDrawContext *c)
{
	c->states = newPtrArray();
}

static void uninitStates(uiDrawContext *c)
{
	// have a helpful diagnostic here
	if (c->states->len > 0)
		complain("imbalanced save/restore count in uiDrawContext in uninitStates(); did you save without restoring?");
	ptrArrayDestroy(c->states);
}

void uiDrawSave(uiDrawContext *c)
{
	struct state *state;
	ID2D1DrawingStateBlock *dsb;
	HRESULT hr;

	hr = d2dfactory->CreateDrawingStateBlock(
		// TODO verify that these are correct
		NULL,
		NULL,
		&dsb);
	if (hr != S_OK)
		logHRESULT(L"error creating drawing state block", hr);
	c->rt->SaveDrawingState(dsb);

	// if we have a clip, we need to hold another reference to it
	if (c->currentClip != NULL)
		c->currentClip->AddRef();

	state = uiNew(struct state);
	state->dsb = dsb;
	state->clip = c->currentClip;
	ptrArrayAppend(c->states, state);
}

void uiDrawRestore(uiDrawContext *c)
{
	struct state *state;

	state = ptrArrayIndex(c->states, struct state *, c->states->len - 1);

	c->rt->RestoreDrawingState(state->dsb);
	state->dsb->Release();

	// if we have a current clip, we need to drop it
	if (c->currentClip != NULL)
		c->currentClip->Release();
	// no need to explicitly addref or release; just transfer the ref
	c->currentClip = state->clip;

	uiFree(state);
	ptrArrayDelete(c->states, c->states->len - 1);
}

// TODO C++-ize the rest of the file

// TODO document that fully opaque black is the default text color; figure out whether this is upheld in various scenarios on other platforms
void uiDrawText(uiDrawContext *c, double x, double y, uiDrawTextLayout *layout)
{
	uiDrawBrush brush;
	ID2D1Brush *black;

	ZeroMemory(&brush, sizeof (uiDrawBrush));
	brush.Type = uiDrawBrushTypeSolid;
	brush.R = 0.0;
	brush.G = 0.0;
	brush.B = 0.0;
	brush.A = 1.0;
	black = makeBrush(&brush, c->rt);
	doDrawText(c->rt, black, x, y, layout);
	black->Release();
}

// TODO this is a mess
ID2D1Brush *createSolidColorBrushInternal(ID2D1RenderTarget *rt, double r, double g, double b, double a)
{
	uiDrawBrush brush;

	ZeroMemory(&brush, sizeof (uiDrawBrush));
	brush.Type = uiDrawBrushTypeSolid;
	brush.R = r;
	brush.G = g;
	brush.B = b;
	brush.A = a;
	return makeBrush(&brush, rt);
}
