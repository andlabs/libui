// 19 november 2015
#include "uipriv_haiku.hpp"

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

void uiDrawStroke(uiDrawContext *c, uiDrawPath *path, uiDrawBrush *b, uiDrawStrokeParams *p)
{
	// TODO
}

void uiDrawFill(uiDrawContext *c, uiDrawPath *path, uiDrawBrush *b)
{
	// TODO
}

void uiDrawMatrixSetIdentity(uiDrawMatrix *m)
{
	// TODO
}

void uiDrawMatrixTranslate(uiDrawMatrix *m, double x, double y)
{
	// TODO
}

void uiDrawMatrixScale(uiDrawMatrix *m, double xCenter, double yCenter, double x, double y)
{
	// TODO
}

void uiDrawMatrixRotate(uiDrawMatrix *m, double x, double y, double amount)
{
	// TODO
}

void uiDrawMatrixSkew(uiDrawMatrix *m, double x, double y, double xamount, double yamount)
{
	// TODO
}

void uiDrawMatrixMultiply(uiDrawMatrix *dest, uiDrawMatrix *src)
{
	// TODO
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
	// TODO
}

void uiDrawClip(uiDrawContext *c, uiDrawPath *path)
{
	// TODO
}

void uiDrawSave(uiDrawContext *c)
{
	// TODO
}

void uiDrawRestore(uiDrawContext *c)
{
	// TODO
}
