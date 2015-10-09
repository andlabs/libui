// 6 september 2015
#import "uipriv_darwin.h"

struct uiDrawPath {
	CGMutablePathRef path;
	uiDrawFillMode fillMode;
	BOOL ended;
};

uiDrawPath *uiDrawNewPath(uiDrawFillMode mode)
{
	uiDrawPath *p;

	p = uiNew(uiDrawPath);
	p->path = CGPathCreateMutable();
	p->fillMode = mode;
	return p;
}

void uiDrawFreePath(uiDrawPath *p)
{
	CGPathRelease((CGPathRef) (p->path));
	uiFree(p);
}

void uiDrawPathNewFigure(uiDrawPath *p, double x, double y)
{
	if (p->ended)
		complain("attempt to add figure to ended path in uiDrawPathNewFigure()");
	CGPathMoveToPoint(p->path, NULL, x, y);
}

void uiDrawPathNewFigureWithArc(uiDrawPath *p, double xCenter, double yCenter, double radius, double startAngle, double endAngle)
{
	double sinStart, cosStart;
	double startx, starty;

	if (p->ended)
		complain("attempt to add figure to ended path in uiDrawPathNewFigureWithArc()");
	sinStart = sin(startAngle);
	cosStart = cos(startAngle);
	startx = xCenter + radius * cosStart;
	starty = yCenter + radius * sinStart;
	CGPathMoveToPoint(p->path, NULL, startx, starty);
	uiDrawPathArcTo(p, xCenter, yCenter, radius, startAngle, endAngle);
}

void uiDrawPathLineTo(uiDrawPath *p, double x, double y)
{
	if (p->ended)
		complain("attempt to add line to ended path in uiDrawPathLineTo()");
	CGPathAddLineToPoint(p->path, NULL, x, y);
}

void uiDrawPathArcTo(uiDrawPath *p, double xCenter, double yCenter, double radius, double startAngle, double endAngle)
{
	if (p->ended)
		complain("attempt to add arc to ended path in uiDrawPathArcTo()");
	CGPathAddArc(p->path, NULL,
		xCenter, yCenter,
		radius,
		startAngle, endAngle,
		NO);
}

void uiDrawPathBezierTo(uiDrawPath *p, double c1x, double c1y, double c2x, double c2y, double endX, double endY)
{
	if (p->ended)
		complain("attempt to add bezier to ended path in uiDrawPathBezierTo()");
	CGPathAddCurveToPoint(p->path, NULL,
		c1x, c1y,
		c2x, c2y,
		endX, endY);
}

void uiDrawPathCloseFigure(uiDrawPath *p)
{
	if (p->ended)
		complain("attempt to close figure of ended path in uiDrawPathCloseFigure()");
	CGPathCloseSubpath(p->path);
}

void uiDrawPathAddRectangle(uiDrawPath *p, double x, double y, double width, double height)
{
	if (p->ended)
		complain("attempt to add rectangle to ended path in uiDrawPathAddRectangle()");
	CGPathAddRect(p->path, NULL, CGRectMake(x, y, width, height));
}

void uiDrawPathEnd(uiDrawPath *p)
{
	p->ended = TRUE;
}

struct uiDrawContext {
	CGContextRef c;
};

uiDrawContext *newContext(CGContextRef ctxt)
{
	uiDrawContext *c;

	c = uiNew(uiDrawContext);
	c->c = ctxt;
	return c;
}

void freeContext(uiDrawContext *c)
{
	uiFree(c);
}

// a stroke is identical to a fill of a stroked path
// we need to do this in order to stroke with a gradient; see http://stackoverflow.com/a/25034854/3408572
// doing this for other brushes works too
void uiDrawStroke(uiDrawContext *c, uiDrawPath *path, uiDrawBrush *b, uiDrawStrokeParams *p)
{
	CGLineCap cap;
	CGLineJoin join;
	uiDrawPath p2;

	if (!path->ended)
		complain("path not ended in uiDrawStroke()");

	switch (p->Cap) {
	case uiDrawLineCapFlat:
		cap = kCGLineCapButt;
		break;
	case uiDrawLineCapRound:
		cap = kCGLineCapRound;
		break;
	case uiDrawLineCapSquare:
		cap = kCGLineCapSquare;
		break;
	}
	switch (p->Join) {
	case uiDrawLineJoinMiter:
		join = kCGLineJoinMiter;
		CGContextSetMiterLimit(c->c, p->MiterLimit);
		break;
	case uiDrawLineJoinRound:
		join = kCGLineJoinRound;
		break;
	case uiDrawLineJoinBevel:
		join = kCGLineJoinBevel;
		break;
	}

	// create a temporary path identical to the previous one
	// the cast is safe; we never modify the CGPathRef and always cast it back to a CGPathRef anyway
	p2.path = (CGMutablePathRef) CGPathCreateCopyByStrokingPath(path->path,
		NULL,
		p->Thickness,
		cap,
		join,
		p->MiterLimit);
	p2.fillMode = path->fillMode;
	p2.ended = path->ended;
	uiDrawFill(c, &p2, b);
	// and clean up
	CGPathRelease((CGPathRef) (p2.path));
}

// for a solid fill, we can merely have Core Graphics fill directly
static void fillSolid(CGContextRef ctxt, uiDrawPath *p, uiDrawBrush *b)
{
	CGContextSetRGBFillColor(ctxt, b->R, b->G, b->B, b->A);
	switch (p->fillMode) {
	case uiDrawFillModeWinding:
		CGContextFillPath(ctxt);
		break;
	case uiDrawFillModeAlternate:
		CGContextEOFillPath(ctxt);
		break;
	}
}

// for a gradient fill, we need to clip to the path and then draw the gradient
// see http://stackoverflow.com/a/25034854/3408572
static void fillGradient(CGContextRef ctxt, uiDrawPath *p, uiDrawBrush *b)
{
	CGGradientRef gradient;
	CGColorSpaceRef colorspace;
	CGFloat *colors;
	CGFloat *locations;
	size_t i;

	// gradients need a color space
	// for consistency with windows, use sRGB
	colorspace = CGColorSpaceCreateWithName(kCGColorSpaceSRGB);

	// make the gradient
	colors = uiAlloc(b->NumStops * 4 * sizeof (CGFloat), "CGFloat[]");
	locations = uiAlloc(b->NumStops * sizeof (CGFloat), "CGFloat[]");
	for (i = 0; i < b->NumStops; i++) {
		colors[i * 4 + 0] = b->Stops[i].R;
		colors[i * 4 + 1] = b->Stops[i].G;
		colors[i * 4 + 2] = b->Stops[i].B;
		colors[i * 4 + 3] = b->Stops[i].A;
		locations[i] = b->Stops[i].Pos;
	}
	gradient = CGGradientCreateWithColorComponents(colorspace, colors, locations, b->NumStops);
	uiFree(locations);
	uiFree(colors);

	// because we're mucking with clipping, we need to save the graphics state and restore it later
	CGContextSaveGState(ctxt);

	// clip
	switch (p->fillMode) {
	case uiDrawFillModeWinding:
		CGContextClip(ctxt);
		break;
	case uiDrawFillModeAlternate:
		CGContextEOClip(ctxt);
		break;
	}

	// draw the gradient
	switch (b->Type) {
	case uiDrawBrushTypeLinearGradient:
		CGContextDrawLinearGradient(ctxt,
			gradient,
			CGPointMake(b->X0, b->Y0),
			CGPointMake(b->X1, b->Y1),
			kCGGradientDrawsBeforeStartLocation | kCGGradientDrawsAfterEndLocation);
		break;
	case uiDrawBrushTypeRadialGradient:
		CGContextDrawRadialGradient(ctxt,
			gradient,
			CGPointMake(b->X0, b->Y0),
			// make the start circle radius 0 to make it a point
			0,
			CGPointMake(b->X1, b->Y1),
			b->OuterRadius,
			kCGGradientDrawsBeforeStartLocation | kCGGradientDrawsAfterEndLocation);
		break;
	}

	// and clean up
	CGContextRestoreGState(ctxt);
	CGGradientRelease(gradient);
	CGColorSpaceRelease(colorspace);
}

void uiDrawFill(uiDrawContext *c, uiDrawPath *path, uiDrawBrush *b)
{
	if (!path->ended)
		complain("path not ended in uiDrawFill()");
	CGContextAddPath(c->c, (CGPathRef) (path->path));
	switch (b->Type) {
	case uiDrawBrushTypeSolid:
		fillSolid(c->c, path, b);
		return;
	case uiDrawBrushTypeLinearGradient:
	case uiDrawBrushTypeRadialGradient:
		fillGradient(c->c, path, b);
		return;
//	case uiDrawBrushTypeImage:
		// TODO
		return;
	}
	complain("unknown brush type %d in uiDrawFill()", b->Type);
}
