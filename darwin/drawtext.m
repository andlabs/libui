// 2 january 2017
#import "uipriv_darwin.h"
#import "draw.h"

struct uiDrawTextLayout {
	CFAttributedStringRef attrstr;
	double width;
	CTFramesetterRef framesetter;
	// note: technically, metrics returned from frame are relative to CGPathGetPathBoundingBox(tl->path)
	// however, from what I can gather, for a path created by CGPathCreateWithRect(), like we do (with a NULL transform), CGPathGetPathBoundingBox() seems to just return the standardized form of the rect used to create the path
	// (this I confirmed through experimentation)
	// so we can just use tl->size for adjustments
	// we don't need to adjust coordinates by any origin since our rect origin is (0, 0)
	CGSize size;
	CGPathRef path;
	CTFrameRef frame;
	CFArrayRef lines;
	size_t *u16tou8;
	size_t nu16tou8;		// TODO I don't like the casing of this name
};

static CTFontRef fontdescToCTFont(uiDrawFontDescriptor *fd)
{
	CTFontDescriptorRef desc;
	CTFontRef font;

	desc = fontdescToCTFontDescriptor(fd);
	font = CTFontCreateWithFontDescriptor(desc, fd->Size, NULL);
	CFRelease(desc);			// TODO correct?
	return font;
}

static CFAttributedStringRef attrstrToCoreFoundation(uiAttributedString *s, uiDrawFontDescriptor *defaultFont)
{
	CFStringRef cfstr;
	CFMutableDictionaryRef defaultAttrs;
	CTFontRef defaultCTFont;
	CFAttributedStringRef base;
	CFMutableAttributedStringRef mas;

	cfstr = CFStringCreateWithCharacters(NULL, attrstrUTF16(s), attrstrUTF16Len(s));
	if (cfstr == NULL) {
		// TODO
	}
	defaultAttrs = CFDictionaryCreateMutable(NULL, 1,
		&kCFCopyStringDictionaryKeyCallBacks,
		&kCFTypeDictionaryValueCallBacks);
	if (defaultAttrs == NULL) {
		// TODO
	}
	defaultCTFont = fontdescToCTFont(defaultFont);
	CFDictionaryAddValue(defaultAttrs, kCTFontAttributeName, defaultCTFont);
	CFRelease(defaultCTFont);

	base = CFAttributedStringCreate(NULL, cfstr, defaultAttrs);
	if (base == NULL) {
		// TODO
	}
	CFRelease(cfstr);
	CFRelease(defaultAttrs);
	mas = CFAttributedStringCreateMutableCopy(NULL, 0, base);
	CFRelease(base);

	CFAttributedStringBeginEditing(mas);
	// TODO copy in the attributes
	CFAttributedStringEndEditing(mas);

	return mas;
}

uiDrawTextLayout *uiDrawNewTextLayout(uiAttributedString *s, uiDrawFontDescriptor *defaultFont, double width)
{
	uiDrawTextLayout *tl;
	CGFloat cgwidth;
	CFRange range, unused;
	CGRect rect;

	tl = uiNew(uiDrawTextLayout);
	tl->attrstr = attrstrToCoreFoundation(s, defaultFont);
	range.location = 0;
	range.length = CFAttributedStringGetLength(tl->attrstr);
	tl->width = width;

	// TODO CTFrameProgression for RTL/LTR
	// TODO kCTParagraphStyleSpecifierMaximumLineSpacing, kCTParagraphStyleSpecifierMinimumLineSpacing, kCTParagraphStyleSpecifierLineSpacingAdjustment for line spacing
	tl->framesetter = CTFramesetterCreateWithAttributedString(tl->attrstr);
	if (tl->framesetter == NULL) {
		// TODO
	}

	cgwidth = (CGFloat) width;
	if (cgwidth < 0)
		cgwidth = CGFLOAT_MAX;
	// TODO these seem to be floor()'d or truncated?
	// TODO double check to make sure this TODO was right
	tl->size = CTFramesetterSuggestFrameSizeWithConstraints(tl->framesetter,
		range,
		// TODO kCTFramePathWidthAttributeName?
		NULL,
		CGSizeMake(cgwidth, CGFLOAT_MAX),
		&unused);			// not documented as accepting NULL

	rect.origin = CGZeroPoint;
	rect.size = tl->size;
	tl->path = CGPathCreateWithRect(rect, NULL);
	tl->frame = CTFramesetterCreateFrame(tl->framesetter,
		range,
		tl->path,
		// TODO kCTFramePathWidthAttributeName?
		NULL);
	if (tl->frame == NULL) {
		// TODO
	}

	tl->lines = CTFrameGetLines(tl->frame);

	// and finally copy the UTF-16 to UTF-8 index conversion table
	tl->u16tou8 = attrstrCopyUTF16ToUTF8(s, &(tl->nu16tou8));

	return tl;
}

void uiDrawFreeTextLayout(uiDrawTextLayout *tl)
{
	uiFree(tl->u16tou8);
	// TODO release tl->lines?
	CFRelease(tl->frame);
	CFRelease(tl->path);
	CFRelease(tl->framesetter);
	CFRelease(tl->attrstr);
	uiFree(tl);
}

// TODO double-check helvetica
// TODO document that (x,y) is the top-left corner of the *entire frame*
void uiDrawText(uiDrawContext *c, uiDrawTextLayout *tl, double x, double y)
{
	CGContextSaveGState(c->c);

	// Core Text doesn't draw onto a flipped view correctly; we have to pretend it was unflipped
	// see the iOS bits of the first example at https://developer.apple.com/library/mac/documentation/StringsTextFonts/Conceptual/CoreText_Programming/LayoutOperations/LayoutOperations.html#//apple_ref/doc/uid/TP40005533-CH12-SW1 (iOS is naturally flipped)
	// TODO how is this affected by a non-identity CTM?
	CGContextTranslateCTM(c->c, 0, cheight);
	CGContextScaleCTM(c->c, 1.0, -1.0);
	CGContextSetTextMatrix(c->c, CGAffineTransformIdentity);

	// wait, that's not enough; we need to offset y values to account for our new flipping
	y = c->height - y;

	// CTFrameDraw() draws in the path we specified when creating the frame
	// this means that in our usage, CTFrameDraw() will draw at (0,0)
	// so move the origin to be at (x,y) instead
	// TODO are the signs correct?
	CGContextTranslateCTM(c->c, x, y);

	CTFrameDraw(tl->frame, c->c);

	CGContextRestoreGState(c->c);
}

// TODO document that the width and height of a layout is not necessarily the sum of the widths and heights of its constituent lines; this is definitely untrue on OS X, where lines are placed in such a way that the distance between baselines is always integral
void uiDrawTextLayoutExtents(uiDrawTextLayout *tl, double *width, double *height)
{
	*width = tl->size.width;
	*height = tl->size.height;
}

int uiDrawTextLayoutNumLines(uiDrawTextLayout *tl)
{
	return CFArrayGetCount(tl->lines);
}

// TODO release when done?
#define getline(tl, line) ((CTLineRef) CFArrayGetValueAtIndex(tl->lines, line))

void uiDrawTextLayoutLineByteRange(uiDrawTextLayout *tl, int line, size_t *start, size_t *end)
{
	CTLineRef lr;
	CFRange range;

	lr = getline(tl, line);
	range = CTLineGetStringRange(lr);
	*start = tl->u16tou8[range.location];
	*end = tl->u16tou8[range.location + range.length];
}

void uiDrawTextLayoutLineGetMetrics(uiDrawTextLayout *tl, int line, uiDrawTextLayoutLineMetrics *m)
{
	CTLineRef lr;
	CFRange range;
	CGPoint origin;
	CGFloat ascent, descent, leading;

	range.location = line;
	range.length = 1;
	CTFrameGetLineOrigins(tl->frame, range, &origin);
	m->X = origin.x;
	// and remember that the frame is flipped
	m->BaselineY = tl->size.height - origin.y;

	lr = getline(tl, line);
	// though CTLineGetTypographicBounds() returns 0 on error, it also returns 0 on an empty string, so we can't reasonably check for error
	m->Width = CTLineGetTypographicBounds(lr, &ascent, &descent, &leading);
	m->Ascent = ascent;
	m->Descent = descent;
	m->Leading = leading;
}

void uiDrawTextLayoutByteIndexToGraphemeRect(uiDrawTextLayout *tl, size_t pos, int *line, double *x, double *y, double *width, double *height)
{
}

static CGPoint *mkLineOrigins(uiDrawTextLayout *tl)
{
	CGPoint *origins;
	CFRange range;
	CFIndex i, n;
	CTLine line;
	CGFloat ascent;

	n = CFArrayGetCount(tl->lines);
	range.location = 0;
	range.length = n;
	origins = (CGPoint *) uiAlloc(n * sizeof (CGPoint), "CGPoint[]");
	CTFrameGetLineOrigins(tl->frame, range, origins);
	for (i = 0; i < n; i++) {
		line = getline(tl, i);
		CTLineGetTypographicBounds(line, &ascent, NULL, NULL);
		origins[i].y = tl->size.height - (origins[i].y + ascent);
	}
	return origins;
}

void uiDrawTextLayoutHitTest(uiDrawTextLayout *tl, double x, double y, uiDrawTextLayoutHitTestResult *result)
{
	CGPoint *mkLineOrigins;
	CFIndex i, n;
	CTLineRef line;
	double firstYForLine;
	CGFloat width, NULL, descent, leading;
	CFRange range;

	n = CFArrayGetCount(tl->lines);
	if (n == 0) {
		// TODO fill result
		return;
	}

	origins = mkLineOrigins(tl);
	if (y < 0) {
		line = getline(tl, 0);
		width = CTLineGetTypographicBounds(line, NULL, NULL, NULL);
		i = 0;
	} else {
		firstYForLine = 0;
		for (i = 0; i < n; i++) {
			line = getline(tl, i);
			width = CTLineGetTypographicBounds(line, NULL, &descent, &leading);
			if (y < maxYForLine)
				break;
			firstYForLine = origins[i].y + descent + leading;
		}
	}
	if (i == n) {
		i--;
		result->Line = i;
		result->YPosition = uiDrawTextLayoutHitTestPositionAfter;
	} else {
		result->Line = i;
		result->YPosition = uiDrawTextLayoutHitTestPositionInside;
		if (i == 0 && y < 0)
			result->YPosition = uiDrawTextLayoutHitTestPositionBefore;
	}
	result->InTrailingWhitespace = 0;
	range = CTLineGetStringRange(line);
	if (x < 0) {
		result->Start = tl->u16tou8[range.location];
		result->End = result->Start;
		result->XPosition = uiDrawTextLayoutHitTestPositionBefore;
	} else if (x > tl->size.width) {
		result->Start = tl->u16tou8[range.location + range.length];
		result->End = result->Start;
		result->XPosition = uiDrawTextLayoutHitTestPositionAfter;
	} else {
		CGPoint pos;
		CFIndex index;

		result->XPosition = uiDrawTextLa
youtHitTestPositionInside;
		pos.x = x;
		// TODO this isn't set properly in any of the fast-track cases
		pos.y = y - firstYForLine;
		index = CTLineGetStringIn
dexForPosition(line, pos);
		if (index == kCFNotFound) {
			// TODO
		}
		result->Pos = tl->u16tou8[index];
		// TODO compute the fractional offset
		result->InTrailingWhitespace = x < origins[i].x || x >= (origins[i].x + width);
	}
	uiFree(origins);
}

void uiDrawTextLayoutByteRangeToRectangle(uiDrawTextLayout *tl, size_t start, size_t end, uiDrawTextLayoutByteRangeRectangle *r)
{
}
