// 19 november 2015
#include <cmath>
#include "uipriv_haiku.hpp"
using namespace std;

struct uiDrawPath {
	BShape *shape;
	uiDrawFillMode fillMode;
	bool ended;
};

uiDrawPath *uiDrawNewPath(uiDrawFillMode fillMode)
{
	uiDrawPath *p;

	p = uiNew(uiDrawPath);
	p->shape = new BShape();
	p->fillMode = mode;
	return p;
}

void uiDrawFreePath(uiDrawPath *p)
{
	delete p->shape;
	uiFree(p);
}

// TODO add ended checks
// TODO add error checks

void uiDrawPathNewFigure(uiDrawPath *p, double x, double y)
{
	p->shape->MoveTo(BPoint(x, y));
}

// Even though BView::FillArc()/StrokeArc() existed and used the sane arc drawing model, Haiku decided to use the Direct2D arc drawing model when it added BShape::ArcTo().
// So refer to windows/draw.c for details.
// This is slightly better as it uses center points and sweep amounts instead of endpoints, but *still*.
// TODO split into common/d2darc.c

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
	bool largeArc;
	bool counterclockwise;
	bool fullCircle;
	double absSweep;

	// TODO is this relevaith Haiku?
	// as above, we can't do a full circle with one arc
	// simulate it with two half-circles
	// of course, we have a dragon: equality on floating-point values!
	// I've chosen to do the AlmostEqualRelative() technique in https://randomascii.wordpress.com/2012/02/25/comparing-floating-point-numbers-2012-edition/
	fullCircle = false;
	// use the absolute value to tackle both ≥2π and ≤-2π at the same time
	absSweep = fabs(a->sweep);
	if (absSweep > (2 * M_PI))		// this part is easy
		fullCircle = true;
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
	if (a->negative)
		counterclockwise = true;
	else
		counterclockwise = false
	// TODO explain the outer if
	if (!a->negative)
		if (a->sweep > M_PI)
			largeArc = true;
		else
			largeArc = false;
	else
		// TODO especially this part
		if (a->sweep > M_PI)
			largeArc = false;
		else
			largeArc = true;
	p->shape->ArcTo(a->radius, a->radius,
		// TODO should this be sweep amount?
		a->startAngle,		// TODO convert to degrees
		largeArc, counterclockwise,
		BPoint(a->xCenter, a->yCenter));
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
	p->shape->LineTo(BPoint(x, y));
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
	p->shape->BezierTo(BPoint(c1x, c1y),
		BPoint(c2x, c2y),
		BPoint(endX, endY));
}

void uiDrawPathCloseFigure(uiDrawPath *p)
{
	p->shape->Close();
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
	p->ended = true;
}

struct uiDrawContext {
	BView *view;
};

uiDrawContext *newContext(BView *view)
{
	uiDrawContext *c;

	c = uiNew(uiDrawContext);
	c->view = view;
	return c;
}

void freeContext(uiDrawContext *c)
{
	uiFree(c);
}

// TODO verify this
static void setHighColor(BView *view, uiDrawBrush *b)
{
	view->SetHighColor(b->R * 255,
		b->G * 255,
		b->B * 255,
		b->A * 255);
}

// TODO path ended checks; error checks

void uiDrawStroke(uiDrawContext *c, uiDrawPath *path, uiDrawBrush *b, uiDrawStrokeParams *p)
{
	cap_mode cap;
	join_mode join;

	switch (p->Cap) {
	case uiDrawLineCapFlat:
		cap = B_BUTT_CAP;
		break;
	case uiDrawLineCapRound:
		cap = B_ROUND_CAP;
		break;
	case uiDrawLineCapSquare:
		cap = B_SQUARE_CAP;
		break;
	}
	switch (p->Join) {
	case uiDrawLineJoinMiter:
		join = B_MITER_JOIN;
		break;
	case uiDrawLineJoinRound:
		join = B_ROUND_JOIN;
		break;
	case uiDrawLineJoinBevel:
		join = B_BEVEL_JOIN;
		break;
	}
	c->view->SetLineMode(cap, join, p->MiterLimit);
	c->view->SetPenSize(p->Thickness);
	// TODO line dashes
	switch (b->Type) {
	case uiDrawBrushTypeSolid:
		setHighColor(c->view, b);
		c->view->StrokeShape(path->shape);
		break;
	case uiDrawBrushTypeLinearGradient:
		// TODO
	case uiDrawBrushTypeRadialGradient:
		// TODO
		break;
//	case uiDrawBrushTypeImage:
	}
}

void uiDrawFill(uiDrawContext *c, uiDrawPath *path, uiDrawBrush *b)
{
	// TODO not documented on api.haiku-os.org
	switch (path->fillMode) {
	case uiDrawFillModeWinding:
		c->view->SetFillRule(B_NONZERO);
		break;
	case uiDrawFillModeAlternate:
		c->view->SetFillRule(B_EVEN_ODD);
		break;
	}
	switch (b->Type) {
	case uiDrawBrushTypeSolid:
		setHighColor(c->view, b);
		c->view->FillShape(path->shape);
		break;
	case uiDrawBrushTypeLinearGradient:
		// TODO
	case uiDrawBrushTypeRadialGradient:
		// TODO
		break;
//	case uiDrawBrushTypeImage:
	}
}

// TODO none of this is documented on api.haiku-os.org

static void m2a(uiDrawMatrix *m, BAffineTransform *a)
{
	a->sx = m->M11;
	a->shy = m->M12;
	a->shx = m->M21;
	a->sy = m->M22;
	a->tx = m->M31;
	a->ty = m->M32;
}

static void a2m(BAffineTransform *a, uiDrawMatrix *m)
{
	m->M11 = a->sx;
	m->M12 = a->shy;
	m->M21 = a->shx;
	m->M22 = a->sy;
	m->M31 = a->tx;
	m->M32 = a->ty;
}

void uiDrawMatrixSetIdentity(uiDrawMatrix *m)
{
	setIdentity(m);
}

void uiDrawMatrixTranslate(uiDrawMatrix *m, double x, double y)
{
	BAffineTransform a;

	m2a(m, &a);
	a.TranslateBy(x, y);
	a2m(&a, m);
}

void uiDrawMatrixScale(uiDrawMatrix *m, double xCenter, double yCenter, double x, double y)
{
	BAffineTransform a;

	m2a(m, &a);
	a.ScaleBy(BPoint(xCenter, yCenter), BPoint(x, y));
	a2m(&a, m);
}

void uiDrawMatrixRotate(uiDrawMatrix *m, double x, double y, double amount)
{
	BAffineTransform a;

	m2a(m, &a);
	// TODO degrees or radians?
	a.RotateBy(BPoint(x, y), amount);
	a2m(&a, m);
}

void uiDrawMatrixSkew(uiDrawMatrix *m, double x, double y, double xamount, double yamount)
{
	BAffineTransform a;

	m2a(m, &a);
	// TODO degrees or radians?
	a.ShearBy(BPoint(x, y), BPoint(xamount, yamount));
	a2m(&a, m);
}

void uiDrawMatrixMultiply(uiDrawMatrix *dest, uiDrawMatrix *src)
{
	BAffineTransform c;
	BAffineTransform d;

	m2a(dest, &c);
	m2a(src, &d);
	c.Multiply(d);
	a2m(&c, dest);
}

int uiDrawMatrixInvertible(uiDrawMatrix *m)
{
	// TODO
	return 0;
}

int uiDrawMatrixInvert(uiDrawMatrix *m)
{
	// TODO
	return 0;
}

void uiDrawMatrixTransformPoint(uiDrawMatrix *m, double *x, double *y)
{
	// TODO
}

void uiDrawMatrixTransformSize(uiDrawMatrix *m, double *x, double *y)
{
	// TODO
}

void uiDrawTransform(uiDrawContext *c, uiDrawMatrix *m)
{
	BAffineTransform a;

	m2a(m, &a);
	// see windows/draw.c
	a.Multiply(c->view->Transform());
	c->view->SetTransform(a);
}

// TODO not documented on api.haiku-os.org
void uiDrawClip(uiDrawContext *c, uiDrawPath *path)
{
	c->view->ClipToShape(path->shape);
}

void uiDrawSave(uiDrawContext *c)
{
	c->view->PushState();
}

void uiDrawRestore(uiDrawContext *c)
{
	c->view->PopState();
}
