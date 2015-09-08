// 7 september 2015
#include "area.h"

struct uiDrawContext {
	HDC dc;

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

	// TODO use uiNew
	c = (uiDrawContext *) malloc(sizeof (uiDrawContext));
	c->dc = dc;
	return c;
}

void uiDrawBeginPathRGB(uiDrawContext *c, uint8_t r, uint8_t g, uint8_t b)
{
	c->useRGB = TRUE;
	c->useAlpha = FALSE;
	c->r = r;
	c->g = g;
	c->b = b;
	if (BeginPath(c->dc) == 0)
		logLastError("error beginning new path in uiDrawBeginPathRGB()");
}

void uiDrawBeginPathRGBA(uiDrawContext *c, uint8_t r, uint8_t g, uint8_t b, uint8_t a)
{
	c->useRGB = TRUE;
	c->useAlpha = TRUE;
	c->r = r;
	c->g = g;
	c->b = b;
	c->a = a;
	if (BeginPath(c->dc) == 0)
		logLastError("error beginning new path in uiDrawBeginPathRGB()");
}

void uiDrawMoveTo(uiDrawContext *c, intmax_t x, intmax_t y)
{
	if (MoveToEx(c->dc, x, y, NULL) == 0)
		logLastError("error moving to point in uiDrawMoveTo()");
}

void uiDrawLineTo(uiDrawContext *c, intmax_t x, intmax_t y)
{
	if (LineTo(c->dc, x, y) == 0)
		logLastError("error drawing line in uiDrawLineTo()");
}

void uiDrawRectangle(uiDrawContext *c, intmax_t x, intmax_t y, intmax_t width, intmax_t height)
{
	if (Rectangle(c->dc, x, y, x + width, y + height) == 0)
		logLastError("error drawing rectangle in uiDrawRectangle()");
}

void uiDrawArc(uiDrawContext *c, intmax_t xCenter, intmax_t yCenter, intmax_t radius, double startAngle, double endAngle, int lineFromCurrentPointToStart)
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
		if (Arc(c->dc, bx, by, bx2, by2, sx, sy, ex, ey) == 0)
			logLastError("error drawing current point arc in uiDrawArc()");
		return;
	}
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
}

void uiDrawBezierTo(uiDrawContext *c, intmax_t c1x, intmax_t c1y, intmax_t c2x, intmax_t c2y, intmax_t endX, intmax_t endY)
{
	POINT points[3];

	points[0].x = c1x;
	points[0].y = c1y;
	points[1].x = c2x;
	points[1].y = c2y;
	points[2].x = endX;
	points[2].y = endY;
	if (PolyBezierTo(c->dc, points, 3) == 0)
		logLastError("error drawing bezier curve in uiDrawBezierTo()");
}

void uiDrawCloseFigure(uiDrawContext *c)
{
	if (CloseFigure(c->dc) == 0)
		logLastError("error closing figure in uiDrawCloseFigure()");
}

static HPEN toPen(uiDrawContext *c, uiDrawStrokeParams *p)
{
	DWORD style;
	LOGBRUSH lb;
	HPEN pen;

	style = PS_GEOMETRIC;
	switch (p->Cap) {
	case uiDrawLineCapFlat:
		style |= PS_ENDCAP_FLAT;
		break;
	case uiDrawLineCapRound:
		style |= PS_ENDCAP_ROUND;
		break;
	case uiDrawLineCapSquare:
		style |= PS_ENDCAP_SQUARE;
		break;
	}
	switch (p->Join) {
	case uiDrawLineJoinMiter:
		style |= PS_JOIN_MITER;
		break;
	case uiDrawLineJoinRound:
		style |= PS_JOIN_ROUND;
		break;
	case uiDrawLineJoinBevel:
		style |= PS_JOIN_BEVEL;
		break;
	}
	ZeroMemory(&lb, sizeof (LOGBRUSH));
	if (c->useRGB) {
		lb.lbStyle = BS_SOLID;
		// we don't need to worry about alpha premultiplication; see below
		lb.lbColor = RGB(c->r, c->g, c->b);
	} else {
		// TODO
	}
	pen = ExtCreatePen(style,
		p->Thickness,
		&lb,
		0,
		NULL);
	if (pen == NULL)
		logLastError("error creating pen in toPen()");
	return pen;
}

// unfortunately alpha blending in GDI is limited to bitmap on bitmap
// to do alpha vector graphics, we have to fake it by drawing to an offscreen bitmap first
// also we can't just use regions, we have to copy the path...
// we don't need to worry about alpha premultiplication; see below
struct alpha {
	HDC dc;
	HBITMAP bitmap;
	VOID *ppvBits;
	HBITMAP prevbitmap;
	RECT r;
};

static void startAlpha(uiDrawContext *c, struct alpha *a)
{
	int n;
	POINT *points;
	BYTE *ops;
	HRGN region;
	BITMAPINFO bi;
	DWORD le;

	ZeroMemory(a, sizeof (struct alpha));

	// first, get the path
	// we need to do this first because PathToRegion() clears the path
	SetLastError(0);
	n = GetPath(c->dc, NULL, NULL, 0);
	if (n == 0) {
		le = GetLastError();
		SetLastError(le);		// just in case
		if (le != 0)
			logLastError("error getting path point count in startAlpha()");
	}
	// TODO switch to uiAlloc
	points = (POINT *) malloc(n * sizeof (POINT));
	ops = (BYTE *) malloc(n * sizeof (BYTE));
	if (GetPath(c->dc, points, ops, n) != n)
		logLastError("error getting path in startAlpha()");

	// next we need to know the bounding box of the path so we can create the bitmap
	region = PathToRegion(c->dc);
	if (region == NULL)
		logLastError("error getting path region in startAlpha()");
	if (GetRgnBox(region, &(a->r)) == 0)
		logLastError("error getting region bounding box in startAlpha()");
	if (DeleteObject(region) == 0)
		logLastError("error deleting path region in startAlpha()");

	// now create a DC compatible with the original that we can copy the path to and AlphaBlend() from
	a->dc = CreateCompatibleDC(c->dc);
	if (a->dc == NULL)
		logLastError("error creating compatible DC in startAlpha()");

	// now create and select in a *device-independent* bitmap that will hold the data that we're going to alpha blend
	ZeroMemory(&bi, sizeof (BITMAPINFO));
	bi.bmiHeader.biSize = sizeof (BITMAPINFOHEADER);
	bi.bmiHeader.biWidth = a->r.right - a->r.left;
	bi.bmiHeader.biHeight = -(a->r.bottom - a->r.top);		// negative to draw top-down
	bi.bmiHeader.biPlanes = 1;
	bi.bmiHeader.biBitCount = 32;
	bi.bmiHeader.biCompression = BI_RGB;
	a->bitmap = CreateDIBSection(c->dc, &bi, DIB_RGB_COLORS,
		&(a->ppvBits), NULL, 0);
	if (a->bitmap == NULL)
		logLastError("error creating bitmap in startAlpha()");
	a->prevbitmap = SelectObject(a->dc, a->bitmap);
	if (a->prevbitmap == NULL)
		logLastError("error selecting bitmap into DC in startAlpha()");

	// now we can finally copy the path like we were going to earlier
	if (BeginPath(a->dc) == 0)
		logLastError("error beginning path in startAlpha()");
	if (PolyDraw(a->dc, points, ops, n) == 0)
		logLastError("error copying path in startAlpha()");
	if (EndPath(a->dc) == 0)
		logLastError("error ending path in startAlpha()");
	free(points);
	free(ops);
}

static void finishAlpha(uiDrawContext *c, struct alpha *a)
{
	BLENDFUNCTION bf;

	bf.BlendOp = AC_SRC_OVER;
	bf.BlendFlags = 0;

	// fortunately blending this is easy; we don't have to touch the pixels at all because BLENDFUNCTION has a way to take care of this...
	bf.SourceConstantAlpha = c->a;
	bf.AlphaFormat = 0;		// no per-pixel apha
	// a consequence of this setup is that the image data doesn't need to be alpha-premultiplied, because SourceConstantAlpha will be multiplied for us (according to the docs, anyway)
	// TODO is this actually the case?

	// and we can finally alpha-blend this!
	if (AlphaBlend(c->dc, a->r.left, a->r.top,
		a->r.right - a->r.left, a->r.bottom - a->r.top,
		a->dc, 0, 0,
		a->r.right - a->r.left, a->r.bottom - a->r.top,
		bf) == FALSE)
		logLastError("error alpha blending in finishAlpha()");

	// and clean it all up
	if (SelectObject(a->dc, a->prevbitmap) != a->bitmap)
		logLastError("error deselecting bitmap in finishAlpha()");
	if (DeleteObject(a->bitmap) == 0)
		logLastError("error deleting object in finishAlpha()");
	if (DeleteDC(a->dc) == 0)
		logLastError("error deleting DC in finishAlpha()");
}

void uiDrawStroke(uiDrawContext *c, uiDrawStrokeParams *p)
{
	HPEN pen, prevpen;
	HDC strokeDC;
	struct alpha alpha;

	if (EndPath(c->dc) == 0)
		logLastError("error ending path in uiDrawStroke()");

	if (c->useAlpha) {
		startAlpha(c, &alpha);
		strokeDC = alpha.dc;
	} else		// no alpha; just stroke directly
		strokeDC = c->dc;

	pen = toPen(c, p);
	if (p->Join == uiDrawLineJoinMiter)
		if (SetMiterLimit(strokeDC, p->MiterLimit, NULL) == 0)
			logLastError("error setting miter limit in uiDrawStroke()");
	prevpen = SelectObject(strokeDC, pen);
	if (prevpen == NULL)
		logLastError("error selecting pen into DC in uiDrawStroke()");
	if (StrokePath(strokeDC) == 0)
		logLastError("error stroking path in uiDrawStroke()");
	if (SelectObject(strokeDC, prevpen) != pen)
		logLastError("error deselecting pen from DC in uiDrawStroke()");
	if (DeleteObject(pen) == 0)
		logLastError("error deleting pen in uiDrawStroke()");

	if (c->useAlpha)
		finishAlpha(c, &alpha);
}

static HBRUSH toBrush(uiDrawContext *c)
{
	HBRUSH brush;

	if (c->useRGB) {
		brush = CreateSolidBrush(RGB(c->r, c->g, c->b));
		if (brush == NULL)
			logLastError("error creating solid brush in toBrush()");
		return brush;
	}
	// TODO
	return NULL;
}

void uiDrawFill(uiDrawContext *c, uiDrawFillMode mode)
{
	HBRUSH brush, prevbrush;
	HDC fillDC;
	struct alpha alpha;
	int pfm;

	if (EndPath(c->dc) == 0)
		logLastError("error ending path in uiDrawFill()");

	if (c->useAlpha) {
		startAlpha(c, &alpha);
		fillDC = alpha.dc;
	} else		// no alpha; just stroke directly
		fillDC = c->dc;

	switch (mode) {
	case uiDrawFillModeWinding:
		pfm = WINDING;
		break;
	case uiDrawFillModeAlternate:
		pfm = ALTERNATE;
		break;
	}
	if (SetPolyFillMode(fillDC, pfm) == 0)
		logLastError("error setting fill mode in uiDrawFill()");
	brush = toBrush(c);
	prevbrush = SelectObject(fillDC, brush);
	if (prevbrush == NULL)
		logLastError("error selecting brush into DC in uiDrawFill()");
	if (FillPath(fillDC) == 0)
		logLastError("error filling path in uiDrawFill()");
	if (SelectObject(fillDC, prevbrush) != brush)
		logLastError("error deselecting brush from DC in uiDrawFill()");
	if (DeleteObject(brush) == 0)
		logLastError("error deleting pen in uiDrawStroke()");

	if (c->useAlpha)
		finishAlpha(c, &alpha);
}
