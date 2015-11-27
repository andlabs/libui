// 19 november 2015
#include "uipriv_winforms.hpp"

uiDrawPath *uiDrawNewPath(uiDrawFillMode fillMode)
{
	// TODO
	return NULL;
}

void uiDrawFreePath(uiDrawPath *p)
{
	// TODO
}

void uiDrawPathNewFigure(uiDrawPath *p, double x, double y)
{
	// TODO
}

void uiDrawPathNewFigureWithArc(uiDrawPath *p, double xCenter, double yCenter, double radius, double startAngle, double sweep, int negative)
{
	// TODO
}

void uiDrawPathLineTo(uiDrawPath *p, double x, double y)
{
	// TODO
}

void uiDrawPathArcTo(uiDrawPath *p, double xCenter, double yCenter, double radius, double startAngle, double sweep, int negative)
{
	// TODO
}

void uiDrawPathBezierTo(uiDrawPath *p, double c1x, double c1y, double c2x, double c2y, double endX, double endY)
{
	// TODO
}

void uiDrawPathCloseFigure(uiDrawPath *p)
{
	// TODO
}

void uiDrawPathAddRectangle(uiDrawPath *p, double x, double y, double width, double height)
{
	// TODO
}

void uiDrawPathEnd(uiDrawPath *p)
{
	// TODO
}

struct uiDrawContext {
	int TODO;
};

uiDrawContext *newContext(/* TODO */)
{
	// TODO
	return NULL;
}

void freeContext(uiDrawContext *c)
{
	uiFree(c);
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
	setIdentity(m);
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

// TODO not documented on api.haiku-os.org
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
