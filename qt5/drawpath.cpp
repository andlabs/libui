
#include "uipriv_qt5.hpp"

#include "draw.hpp"

#include <QtMath>

uiDrawPath *uiDrawNewPath(uiDrawFillMode mode)
{
	return new uiDrawPath(mode);
}

void uiDrawFreePath(uiDrawPath *p)
{
	delete p;
}

void uiDrawPathNewFigure(uiDrawPath *p, double x, double y)
{
	p->moveTo(x,y);
}

void uiDrawPathNewFigureWithArc(uiDrawPath *p, double xCenter, double yCenter, double radius, double startAngle, double sweep, int negative)
{
	qreal xStart = xCenter + radius*qCos(startAngle);
	qreal yStart = yCenter + radius*qSin(startAngle);
	p->moveTo(xStart, yStart);
	if (negative) {
		sweep = -(2*M_PI-sweep);
	}
	p->arcTo(xCenter-radius, yCenter-radius, radius*2, radius*2, -qRadiansToDegrees(startAngle), -qRadiansToDegrees(sweep));
}

void uiDrawPathLineTo(uiDrawPath *p, double x, double y)
{
	p->lineTo(x,y);
}

void uiDrawPathArcTo(uiDrawPath *p, double xCenter, double yCenter, double radius, double startAngle, double sweep, int negative)
{
	if (negative) {
		sweep = -(2*M_PI-sweep);
	}
	p->arcTo(xCenter-radius, yCenter-radius, radius*2, radius*2, -qRadiansToDegrees(startAngle), -qRadiansToDegrees(sweep));
}

void uiDrawPathBezierTo(uiDrawPath *p, double c1x, double c1y, double c2x, double c2y, double endX, double endY)
{
	p->cubicTo(c1x, c1y, c2x, c2y, endX, endY);
}

void uiDrawPathCloseFigure(uiDrawPath *p)
{
	p->closeSubpath();
}

void uiDrawPathAddRectangle(uiDrawPath *p, double x, double y, double width, double height)
{
	p->addRect(x,y,width,height);
}

void uiDrawPathEnd(uiDrawPath *p)
{
	Q_UNUSED(p);
}

uiDrawFillMode pathFillMode(uiDrawPath *path)
{
	switch (path->fillRule())
	{
	case Qt::WindingFill: return uiDrawFillModeWinding;
	case Qt::OddEvenFill: return uiDrawFillModeAlternate;
	}
	Q_UNREACHABLE();
	return 0;
}
