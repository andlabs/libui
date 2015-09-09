// 6 september 2015
#include "area.h"

struct uiDrawContext {
	CGContextRef c;

	BOOL useRGBA;
	CGFloat r;
	CGFloat g;
	CGFloat b;
	CGFloat a;
};

uiDrawContext *newContext(CGContextRef ctxt)
{
	uiDrawContext *c;

	// TODO use uiNew
	c = (uiDrawContext *) malloc(sizeof (uiDrawContext));
	c->c = ctxt;
	return c;
}

void uiDrawBeginPathRGB(uiDrawContext *c, uint8_t r, uint8_t g, uint8_t b)
{
	c->useRGBA = YES;
	c->r = ((CGFloat) r) / 255;
	c->g = ((CGFloat) g) / 255;
	c->b = ((CGFloat) b) / 255;
	c->a = 1.0;
	CGContextBeginPath(c->c);
}

void uiDrawBeginPathRGBA(uiDrawContext *c, uint8_t r, uint8_t g, uint8_t b, uint8_t a)
{
	c->useRGBA = YES;
	c->r = ((CGFloat) r) / 255;
	c->g = ((CGFloat) g) / 255;
	c->b = ((CGFloat) b) / 255;
	c->a = ((CGFloat) a) / 255;
	CGContextBeginPath(c->c);
}

void uiDrawMoveTo(uiDrawContext *c, intmax_t x, intmax_t y)
{
	CGContextMoveToPoint(c->c, x, y);
}

void uiDrawLineTo(uiDrawContext *c, intmax_t x, intmax_t y)
{
	CGContextAddLineToPoint(c->c, x, y);
}

void uiDrawRectangle(uiDrawContext *c, intmax_t x, intmax_t y, intmax_t width, intmax_t height)
{
	CGContextAddRect(c->c, CGRectMake(x, y, width, height));
}

void uiDrawArcTo(uiDrawContext *c, intmax_t xCenter, intmax_t yCenter, intmax_t radius, double startAngle, double endAngle, int lineFromCurrentPointToStart)
{
	if (!lineFromCurrentPointToStart) {
		// see http://stackoverflow.com/questions/31489157/extra-line-when-drawing-an-arc-in-swift
		// TODO verify correctness
		CGFloat x, y;

		x = xCenter;
		y = yCenter;
		x += radius * cos(startAngle);
		y -= radius * sin(startAngle);
		CGContextMoveToPoint(c->c, x, y);
	}
	CGContextAddArc(c->c,
		xCenter, yCenter,
		radius,
		startAngle, endAngle,
		0);
}

void uiDrawBezierTo(uiDrawContext *c, intmax_t c1x, intmax_t c1y, intmax_t c2x, intmax_t c2y, intmax_t endX, intmax_t endY)
{
	CGContextAddCurveToPoint(c->c,
		c1x, c1y,
		c2x, c2y,
		endX, endY);
}

void uiDrawCloseFigure(uiDrawContext *c)
{
	CGContextClosePath(c->c);
}

void uiDrawStroke(uiDrawContext *c, uiDrawStrokeParams *p)
{
	switch (p->Cap) {
	case uiDrawLineCapFlat:
		CGContextSetLineCap(c->c, kCGLineCapButt);
		break;
	case uiDrawLineCapRound:
		CGContextSetLineCap(c->c, kCGLineCapRound);
		break;
	case uiDrawLineCapSquare:
		CGContextSetLineCap(c->c, kCGLineCapSquare);
		break;
	}
	switch (p->Join) {
	case uiDrawLineJoinMiter:
		CGContextSetLineJoin(c->c, kCGLineJoinMiter);
		CGContextSetMiterLimit(c->c, p->MiterLimit);
		break;
	case uiDrawLineJoinRound:
		CGContextSetLineJoin(c->c, kCGLineJoinRound);
		break;
	case uiDrawLineJoinBevel:
		CGContextSetLineJoin(c->c, kCGLineJoinBevel);
		break;
	}
	CGContextSetLineWidth(c->c, p->Thickness);
	if (c->useRGBA)
		CGContextSetRGBStrokeColor(c->c, c->r, c->g, c->b, c->a);
	else {
		// TODO
	}
	CGContextStrokePath(c->c);
}

void uiDrawFill(uiDrawContext *c, uiDrawFillMode mode)
{
	if (c->useRGBA)
		CGContextSetRGBFillColor(c->c, c->r, c->g, c->b, c->a);
	else {
		// TODO
	}
	switch (mode) {
	case uiDrawFillModeWinding:
		CGContextFillPath(c->c);
		break;
	case uiDrawFillModeAlternate:
		CGContextEOFillPath(c->c);
		break;
	}
}
