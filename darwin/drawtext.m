// 2 january 2017
#import "uipriv_darwin.h"

struct uiDrawTextLayout {
	CFAttributedStringRef attrstr;
	double width;
	CTFramesetterRef framesetter;
	CGSize size;
	CGPathRef path;
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
	return tl;
}

void uiDrawFreeTextLayout(uiDrawTextLayout *tl)
{
	CFRelease(tl->path);
	CFRelease(tl->framesetter);
	CFRelease(tl->attrstr);
	uiFree(tl);
}

void uiDrawText(uiDrawContext *c, uiDrawTextLayout *tl, double x, double y)
{
}

void uiDrawTextLayoutExtents(uiDrawTextLayout *tl, double *width, double *height)
{
}

int uiDrawTextLayoutNumLines(uiDrawTextLayout *tl)
{
}

void uiDrawTextLayoutLineByteRange(uiDrawTextLayout *tl, int line, size_t *start, size_t *end)
{
}

void uiDrawTextLayoutLineGetMetrics(uiDrawTextLayout *tl, int line, uiDrawTextLayoutLineMetrics *m)
{
}

void uiDrawTextLayoutByteIndexToGraphemeRect(uiDrawTextLayout *tl, size_t pos, int *line, double *x, double *y, double *width, double *height)
{
}

uiDrawTextLayoutHitTestResult uiDrawTextLayoutHitTest(uiDrawTextLayout *tl, double x, double y, size_t *byteIndex, int *line)
{
}

void uiDrawTextLayoutByteRangeToRectangle(uiDrawTextLayout *tl, size_t start, size_t end, uiDrawTextLayoutByteRangeRectangle *r)
{
}
