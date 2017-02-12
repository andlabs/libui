// 2 january 2017
#import "uipriv_darwin.h"
#import "draw.h"

// TODO what happens if nLines == 0 in any function?

struct uiDrawTextLayout {
	CFAttributedStringRef attrstr;

	// the width as passed into uiDrawTextLayout constructors
	double width;

	CTFramesetterRef framesetter;

	// the *actual* size of the frame
	// note: technically, metrics returned from frame are relative to CGPathGetPathBoundingBox(tl->path)
	// however, from what I can gather, for a path created by CGPathCreateWithRect(), like we do (with a NULL transform), CGPathGetPathBoundingBox() seems to just return the standardized form of the rect used to create the path
	// (this I confirmed through experimentation)
	// so we can just use tl->size for adjustments
	// we don't need to adjust coordinates by any origin since our rect origin is (0, 0)
	CGSize size;

	CGPathRef path;
	CTFrameRef frame;

	CFArrayRef lines;
	CFIndex nLines;
	// we compute this once when first creating the layout
	uiDrawTextLayoutLineMetrics *lineMetrics;

	// for converting CFAttributedString indices from/to byte offsets
	size_t *u8tou16;
	size_t nUTF8;
	size_t *u16tou8;
	size_t nUTF16;
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

static const CTTextAlignment ctaligns[] = {
	[uiDrawTextLayoutAlignLeft] = kCTTextAlignmentLeft,
	[uiDrawTextLayoutAlignCenter] = kCTTextAlignmentCenter,
	[uiDrawTextLayoutAlignRight] = kCTTextAlignmentRight,
};

static CTParagraphStyleRef mkParagraphStyle(uiDrawTextLayoutParams *p)
{
	CTParagraphStyleRef ps;
	CTParagraphStyleSetting settings[16];
	size_t nSettings = 0;

	settings[nSettings].spec = kCTParagraphStyleSpecifierAlignment;
	settings[nSettings].valueSize = sizeof (CTTextAlignment);
	settings[nSettings].value = ctaligns + p->Align;
	nSettings++;

	ps = CTParagraphStyleCreate(settings, nSettings);
	if (ps == NULL) {
		// TODO
	}
	return ps;
}

static CFAttributedStringRef attrstrToCoreFoundation(uiDrawTextLayoutParams *p)
{
	CFStringRef cfstr;
	CFMutableDictionaryRef defaultAttrs;
	CTFontRef defaultCTFont;
	CTParagraphStyleRef ps;
	CFAttributedStringRef base;
	CFMutableAttributedStringRef mas;

	cfstr = CFStringCreateWithCharacters(NULL, attrstrUTF16(p->String), attrstrUTF16Len(p->String));
	if (cfstr == NULL) {
		// TODO
	}
	defaultAttrs = CFDictionaryCreateMutable(NULL, 0,
		&kCFCopyStringDictionaryKeyCallBacks,
		&kCFTypeDictionaryValueCallBacks);
	if (defaultAttrs == NULL) {
		// TODO
	}
	defaultCTFont = fontdescToCTFont(p->DefaultFont);
	CFDictionaryAddValue(defaultAttrs, kCTFontAttributeName, defaultCTFont);
	CFRelease(defaultCTFont);
	ps = mkParagraphStyle(p);
	CFDictionaryAddValue(defaultAttrs, kCTParagraphStyleAttributeName, ps);
	CFRelease(ps);

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

// TODO this is wrong for our hit-test example's multiple combining character example
static uiDrawTextLayoutLineMetrics *computeLineMetrics(CTFrameRef frame, CGSize size)
{
	uiDrawTextLayoutLineMetrics *metrics;
	CFArrayRef lines;
	CTLineRef line;
	CFIndex i, n;
	CGFloat ypos;
	CGRect bounds, boundsNoLeading;
	CGFloat ascent, descent, leading;
	CGPoint *origins;

	lines = CTFrameGetLines(frame);
	n = CFArrayGetCount(lines);
	metrics = (uiDrawTextLayoutLineMetrics *) uiAlloc(n * sizeof (uiDrawTextLayoutLineMetrics), "uiDrawTextLayoutLineMetrics[] (text layout)");

	origins = (CGPoint *) uiAlloc(n * sizeof (CGPoint), "CGPoint[] (text layout)");
	CTFrameGetLineOrigins(frame, CFRangeMake(0, n), origins);

	ypos = size.height;
	for (i = 0; i < n; i++) {
		line = (CTLineRef) CFArrayGetValueAtIndex(lines, i);
		bounds = CTLineGetBoundsWithOptions(line, 0);
		boundsNoLeading = CTLineGetBoundsWithOptions(line, kCTLineBoundsExcludeTypographicLeading);

		// this is equivalent to boundsNoLeading.size.height + boundsNoLeading.origin.y (manually verified)
		ascent = bounds.size.height + bounds.origin.y;
		descent = -boundsNoLeading.origin.y;
		// TODO does this preserve leading sign?
		leading = -bounds.origin.y - descent;

		// Core Text always rounds these up for paragraph style calculations; there is a flag to control it but it's inaccessible (and this behavior is turned off for old versions of iPhoto)
		ascent = floor(ascent + 0.5);
		descent = floor(descent + 0.5);
		if (leading > 0)
			leading = floor(leading + 0.5);

		metrics[i].X = origins[i].x;
		metrics[i].Y = origins[i].y - descent - leading;
		metrics[i].Width = bounds.size.width;
		metrics[i].Height = ascent + descent + leading;

		metrics[i].BaselineY = origins[i].y;
		metrics[i].Ascent = ascent;
		metrics[i].Descent = descent;
		metrics[i].Leading = leading;

		// TODO
		metrics[i].ParagraphSpacingBefore = 0;
		metrics[i].LineHeightSpace = 0;
		metrics[i].LineSpacing = 0;
		metrics[i].ParagraphSpacing = 0;

		// and finally advance to the next line
		ypos += metrics[i].Height;
	}

	// okay, but now all these metrics are unflipped
	// we need to flip them
	for (i = 0; i < n; i++) {
		metrics[i].Y = size.height - metrics[i].Y;
		// go from bottom-left corner to top-left
		metrics[i].Y -= metrics[i].Height;
		metrics[i].BaselineY = size.height - metrics[i].BaselineY;
		// TODO also adjust by metrics[i].Height?
	}

	uiFree(origins);
	return metrics;
}

uiDrawTextLayout *uiDrawNewTextLayout(uiDrawTextLayoutParams *p)
{
	uiDrawTextLayout *tl;
	CGFloat cgwidth;
	CFRange range, unused;
	CGRect rect;

	tl = uiNew(uiDrawTextLayout);
	tl->attrstr = attrstrToCoreFoundation(p);
	range.location = 0;
	range.length = CFAttributedStringGetLength(tl->attrstr);
	tl->width = p->Width;

	// TODO CTFrameProgression for RTL/LTR
	// TODO kCTParagraphStyleSpecifierMaximumLineSpacing, kCTParagraphStyleSpecifierMinimumLineSpacing, kCTParagraphStyleSpecifierLineSpacingAdjustment for line spacing
	tl->framesetter = CTFramesetterCreateWithAttributedString(tl->attrstr);
	if (tl->framesetter == NULL) {
		// TODO
	}

	cgwidth = (CGFloat) (tl->width);
	if (cgwidth < 0)
		cgwidth = CGFLOAT_MAX;
	// TODO these seem to be floor()'d or truncated?
	// TODO double check to make sure this TODO was right
	tl->size = CTFramesetterSuggestFrameSizeWithConstraints(tl->framesetter,
		range,
		// TODO kCTFramePathWidthAttributeName?
		NULL,
		CGSizeMake(cgwidth, CGFLOAT_MAX),
		&unused);			// not documented as accepting NULL (TODO really?)

	rect.origin = CGPointZero;
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
	tl->nLines = CFArrayGetCount(tl->lines);
	tl->lineMetrics = computeLineMetrics(tl->frame, tl->size);

	// and finally copy the UTF-8/UTF-16 conversion tables
	tl->u8tou16 = attrstrCopyUTF8ToUTF16(p->String, &(tl->nUTF8));
	tl->u16tou8 = attrstrCopyUTF16ToUTF8(p->String, &(tl->nUTF16));

	return tl;
}

void uiDrawFreeTextLayout(uiDrawTextLayout *tl)
{
	uiFree(tl->u16tou8);
	uiFree(tl->u8tou16);
	uiFree(tl->lineMetrics);
	// TODO release tl->lines?
	CFRelease(tl->frame);
	CFRelease(tl->path);
	CFRelease(tl->framesetter);
	CFRelease(tl->attrstr);
	uiFree(tl);
}

// TODO document that (x,y) is the top-left corner of the *entire frame*
void uiDrawText(uiDrawContext *c, uiDrawTextLayout *tl, double x, double y)
{
	CGContextSaveGState(c->c);

	// Core Text doesn't draw onto a flipped view correctly; we have to pretend it was unflipped
	// see the iOS bits of the first example at https://developer.apple.com/library/mac/documentation/StringsTextFonts/Conceptual/CoreText_Programming/LayoutOperations/LayoutOperations.html#//apple_ref/doc/uid/TP40005533-CH12-SW1 (iOS is naturally flipped)
	// TODO how is this affected by a non-identity CTM?
	CGContextTranslateCTM(c->c, 0, c->height);
	CGContextScaleCTM(c->c, 1.0, -1.0);
	CGContextSetTextMatrix(c->c, CGAffineTransformIdentity);

	// wait, that's not enough; we need to offset y values to account for our new flipping
	// TODO explain this calculation
	y = c->height - tl->size.height - y;

	// CTFrameDraw() draws in the path we specified when creating the frame
	// this means that in our usage, CTFrameDraw() will draw at (0,0)
	// so move the origin to be at (x,y) instead
	// TODO are the signs correct?
	CGContextTranslateCTM(c->c, x, y);

	CTFrameDraw(tl->frame, c->c);

	CGContextRestoreGState(c->c);
}

// TODO document that the width and height of a layout is not necessarily the sum of the widths and heights of its constituent lines; this is definitely untrue on OS X, where lines are placed in such a way that the distance between baselines is always integral
// TODO width doesn't include trailing whitespace...
// TODO figure out how paragraph spacing should play into this
void uiDrawTextLayoutExtents(uiDrawTextLayout *tl, double *width, double *height)
{
	*width = tl->size.width;
	*height = tl->size.height;
}

int uiDrawTextLayoutNumLines(uiDrawTextLayout *tl)
{
	return tl->nLines;
}

void uiDrawTextLayoutLineByteRange(uiDrawTextLayout *tl, int line, size_t *start, size_t *end)
{
	CTLineRef lr;
	CFRange range;

	lr = (CTLineRef) CFArrayGetValueAtIndex(tl->lines, line);
	range = CTLineGetStringRange(lr);
	*start = tl->u16tou8[range.location];
	*end = tl->u16tou8[range.location + range.length];
}

void uiDrawTextLayoutLineGetMetrics(uiDrawTextLayout *tl, int line, uiDrawTextLayoutLineMetrics *m)
{
	*m = tl->lineMetrics[line];
}

// TODO note that in some cases lines can overlap slightly
// in our case, we read lines first to last and use their bottommost point (Y + Height) to determine where the next line should start for hit-testing
void uiDrawTextLayoutHitTest(uiDrawTextLayout *tl, double x, double y, size_t *pos, int *line)
{
	int i;
	CTLineRef ln;
	CFIndex p;

	for (i = 0; i < tl->nLines; i++) {
		double ltop, lbottom;

		ltop = tl->lineMetrics[i].Y;
		lbottom = ltop + tl->lineMetrics[i].Height;
		// y will already >= ltop at this point since the past lbottom should == (or at least >=, see above) ltop
		if (y < lbottom)
			break;
	}
	if (i == tl->nLines)
		i--;
	*line = i;

	ln = (CTLineRef) CFArrayGetValueAtIndex(tl->lines, i);
	// note: according to the docs, we pass a y of 0 for this since the is the baseline of that line (the point is relative to the line)
	// TODO is x relative to the line origin?
	p = CTLineGetStringIndexForPosition(ln, CGPointMake(x, 0));
	if (p == kCFNotFound) {
		// TODO
	}
	*pos = tl->u16tou8[p];
}

double uiDrawTextLayoutByteLocationInLine(uiDrawTextLayout *tl, size_t pos, int line)
{
	CTLineRef lr;
	CFRange range;

	pos = tl->u8tou16[pos];
	if (line < 0 || line >= tl->nLines)
		return -1;
	lr = (CTLineRef) CFArrayGetValueAtIndex(tl->lines, line);
	range = CTLineGetStringRange(lr);
	// note: >, not >=, because the position at end is valid!
	if (pos < range.location || pos > (range.location + range.length))
		return -1;
	// no point in checking the return; we already validated everything and 0 is a valid return for the first index :/
	return CTLineGetOffsetForStringIndex(lr, pos, NULL);
}

void caretDrawParams(uiDrawContext *c, double height, struct caretDrawParams *p)
{
	NSColor *cc;
	CGFloat cr, cg, cb, ca;

	// Interface Builder sets this as the insertion point color for a NSTextView by default
	cc = [NSColor controlTextColor];
	// the given color may not be an RGBA color, which will cause the -getRed:green:blue:alpha: call to throw an exception
	cc = [cc colorUsingColorSpace:[NSColorSpace sRGBColorSpace]];
	[cc getRed:&cr green:&cg blue:&cb alpha:&ca];
	p->r = cr;
	p->g = cg;
	p->b = cb;
	p->a = ca;
	// both cc and the controlTextColor it was made from will be autoreleased since they aren't new or init calls
	// TODO disabled carets have some blending applied...

	// okay there doesn't seem to be any defined metrics for these, argh...
	p->width = 1;
	p->xoff = 0;
}
