// 7 september 2015
#include "area.h"

struct uiDrawContext {
	HDC mainDC;

	// when drawing in RGBA mode, we need to use this
	HDC alphaDC;
	HBITMAP alphaBitmap;

	// and this is the DC we actually draw to, either mainDC or alphaDC
	HDC currentDC;

	// source color
	BOOL useRGB;
	BOOL useAlpha;
	uint8_t r;
	uint8_t g;
	uint8_t b;
	uint8_t a;
};

uiDrawContext *newContext(HDC dc)
{
	uiDrawContext *c;

	c = (uiDrawContext *) g_malloc0(sizeof (uiDrawContext));
	c->mainDC = dc;
	return c;
}

void uiDrawBeginPathRGB(uiDrawContext *c, uint8_t r, uint8_t g, uint8_t b)
{
	c->useRGB = TRUE;
	c->useAlpha = FALSE;
	c->r = r;
	c->g = g;
	c->b = b;
	c->currentDC = c->mainDC;
	if (BeginPath(c->currentDC) == 0)
		logLastError("error beginning new path in uiDrawBeginPathRGB()");
}

void uiDrawBeginPathRGBA(uiDrawContext *c, uint8_t r, uint8_t g, uint8_t b, uint8_t a)
{
	// TODO
}

void uiDrawMoveTo(uiDrawContext *c, intmax_t x, intmax_t y)
{
	if (MoveToEx(c->currentDC, x, y, NULL) == 0)
		logLastError("error moving to point in uiDrawMoveTo()");
}

void uiDrawLineTo(uiDrawContext *c, intmax_t x, intmax_t y)
{
	if (LineTo(c->currentDC, x, y) == 0)
		logLastError("error drawing line in uiDrawLineTo()");
}

void uiDrawRectangle(uiDrawContext *c, intmax_t x, intmax_t y, intmax_t width, intmax_t height)
{
	if (Rectangle(c->currentDC, x, y, x + width, y + height) == 0)
		logLastError("error drawing rectangle in uiDrawRectangle()");
}

void uiDrawArc(uiDrawContext *c, intmax_t xCenter, intmax_t yCenter, intmax_t radius, double startAngle, double endAngle, int lineFromCurrentPointToStart)
{
	if (!lineFromCurrentPointToStart) {
		// TODO
	}
	// TODO convert radians to degrees
	if (AngleArc(c->currentDC,
		xCenter, yCenter
		radius,
		startAngle,
		// the "sweep angle" is relative to the start angle, not to 0
		endAngle - startAngle) == 0)
		logLastError("error drawing arc in uiDrawArc()");
}

void uiDrawCloseFigure(uiDrawContext *c)
{
	if (CloseFigure(c->currentDC) == 0)
		logLastError("error closing figure in uiDrawCloseFigure()");
}

void uiDrawStroke(uiDrawContext *c, uiDrawStrokeParams *p)
{
	// TODO

	switch (p->Cap) {
	case uiDrawLineCapFlat:
		break;
	case uiDrawLineCapRound:
		break;
	case uiDrawLineCapSquare:
		break;
	}
	switch (p->Join) {
	case uiDrawLineJoinMiter:
		break;
	case uiDrawLineJoinRound:
		break;
	case uiDrawLineJoinBevel:
		break;
	}
}

void uiDrawFill(uiDrawContext *c, uiDrawFillMode mode)
{
	// TODO

	switch (mode) {
	case uiDrawFillModeWinding:
		break;
	case uiDrawFillModeAlternate:
		break;
	}
}
