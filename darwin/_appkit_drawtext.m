// 2 january 2017
#import "uipriv_darwin.h"
#import "draw.h"

@interface lineInfo : NSObject
@property NSRange glyphRange;
@property NSRange characterRange;
@property NSRect lineRect;
@property NSRect lineUsedRect;
@property NSRect glyphBoundingRect;
@property CGFloat baselineOffset;
@property double ascent;
@property double descent;
@property double leading;
@end

@implementation lineInfo
@end

struct uiDrawTextLayout {
	// NSTextStorage is subclassed from NSMutableAttributedString
	NSTextStorage *attrstr;
	NSTextContainer *container;
	NSLayoutManager *layoutManager;

	// the width as passed into uiDrawTextLayout constructors
	double width;

#if 0 /* TODO */
	// the *actual* size of the frame
	// note: technically, metrics returned from frame are relative to CGPathGetPathBoundingBox(tl->path)
	// however, from what I can gather, for a path created by CGPathCreateWithRect(), like we do (with a NULL transform), CGPathGetPathBoundingBox() seems to just return the standardized form of the rect used to create the path
	// (this I confirmed through experimentation)
	// so we can just use tl->size for adjustments
	// we don't need to adjust coordinates by any origin since our rect origin is (0, 0)
	CGSize size;
#endif

	NSMutableArray<lineInfo *> *lineInfo;

	// for converting CFAttributedString indices to byte offsets
	size_t *u16tou8;
	size_t nu16tou8;		// TODO I don't like the casing of this name
};

static NSFont *fontdescToNSFont(uiDrawFontDescriptor *fd)
{
	NSFontDescriptor *desc;
	NSFont *font;

	desc = fontdescToNSFontDescriptor(fd);
	font = [NSFont fontWithDescriptor:desc size:fd->Size];
	[desc release];
	return font;
}

static NSTextStorage *attrstrToTextStorage(uiAttributedString *s, uiDrawFontDescriptor *defaultFont)
{
	NSString *nsstr;
	NSMutableDictionary *defaultAttrs;
	NSTextStorage *attrstr;

	nsstr = [[NSString alloc] initWithCharacters:attrstrUTF16(s)
		length:attrstrUTF16Len(s)];

	defaultAttrs = [NSMutableDictionary new];
	[defaultAttrs setObject:fontdescToNSFont(defaultFont)
		forKey:NSFontAttributeName];

	attrstr = [[NSTextStorage alloc] initWithString:nsstr
		attributes:defaultAttrs];
	[defaultAttrs release];
	[nsstr release];

	[attrstr beginEditing];
	// TODO copy in the attributes
	[attrstr endEditing];

	return attrstr;
}

// TODO fine-tune all the properties
uiDrawTextLayout *uiDrawNewTextLayout(uiAttributedString *s, uiDrawFontDescriptor *defaultFont, double width)
{
	uiDrawTextLayout *tl;
	CGFloat cgwidth;
	// TODO correct type?
	NSUInteger index;

	tl = uiNew(uiDrawTextLayout);
	tl->attrstr = attrstrToTextStorage(s, defaultFont);
	tl->width = width;

	// TODO the documentation on the size property implies this might not be necessary?
	cgwidth = (CGFloat) width;
	if (cgwidth < 0)
		cgwidth = CGFLOAT_MAX;
	// TODO rename to tl->textContainer
	tl->container = [[NSTextContainer alloc] initWithSize:NSMakeSize(cgwidth, CGFLOAT_MAX)];
	// TODO pull the reference for this
	[tl->container setLineFragmentPadding:0];

	tl->layoutManager = [[NSLayoutManager alloc] init];
	[tl->layoutManager setTypesetterBehavior:NSTypesetterLatestBehavior];

	[tl->layoutManager addTextContainer:tl->container];
	[tl->attrstr addLayoutManager:tl->layoutManager];
	// and force a re-layout (TODO get source
	[tl->layoutManager glyphRangeForTextContainer:tl->container];

	// TODO equivalent of CTFrameProgression for RTL/LTR?

	// now collect line information; see https://developer.apple.com/library/content/documentation/Cocoa/Conceptual/TextLayout/Tasks/CountLines.html
	tl->lineInfo = [NSMutableArray<lineInfo *> new];
	index = 0;
	while (index < [tl->layoutManager numberOfGlyphs]) {
		NSRange glyphRange;
		__block lineInfo *li;

		li = [lineInfo new];
		li.lineRect = [tl->layoutManager lineFragmentRectForGlyphAtIndex:index effectiveRange:&glyphRange];
		li.glyphRange = glyphRange;
		li.characterRange = [tl->layoutManager characterRangeForGlyphRange:li.glyphRange actualGlyphRange:NULL];
		li.lineUsedRect = [tl->layoutManager lineFragmentUsedRectForGlyphAtIndex:index effectiveRange:NULL];
		li.glyphBoundingRect = [tl->layoutManager boundingRectForGlyphRange:li.glyphRange inTextContainer:tl->container];
		// and this is from http://www.cocoabuilder.com/archive/cocoa/308568-how-to-get-baseline-info.html and http://www.cocoabuilder.com/archive/cocoa/199283-height-and-location-of-text-within-line-in-nslayoutmanager-ignoring-spacing.html
		li.baselineOffset = [[tl->layoutManager typesetter] baselineOffsetInLayoutManager:tl->layoutManager glyphIndex:index];
		li.ascent = 0;
		li.descent = 0;
		li.leading = 0;
		// imitate what AppKit actually does (or seems to)
		[tl->attrstr enumerateAttributesInRange:li.characterRange options:0 usingBlock:^(NSDictionary<NSString *,id> *attrs, NSRange range, BOOL *stop) {
			NSFont *f;
			NSRect boundingRect;
			double v, realAscent, realDescent, realLeading;
			BOOL skipAdjust, doAdjust;

			f = (NSFont *) [attrs objectForKey:NSFontAttributeName];
			if (f == nil) {
				f = [NSFont fontWithName:@"Helvetica" size:12.0];
				if (f == nil)
					f = [NSFont systemFontOfSize:12.0];
			}

			boundingRect = [f boundingRectForFont];
			realAscent = [f ascender];
			realDescent = -[f descender];
			realLeading = [f leading];
			skipAdjust = NO;
			doAdjust = NO;
			if (NSMaxY(boundingRect) <= realAscent) {
				// ascent entirely within bounding box
				// don't do anything if there's leading; I'm guessing it's a combination of both of the reasons to skip below... (least sure of this one)
				if (realLeading != 0)
					skipAdjust = YES;
				// does the descent slip outside the bounding box?
				if (-realDescent <= NSMinY(boundingRect))
					// yes — I guess we should assume accents don't collide with the previous line's descent, though I'm not as sure of that as I am about the else clause below...
					skipAdjust = YES;
			} else {
				// ascent outside bounding box — ascent does not include accents
				// only skip adjustment if there isn't leading (apparently some fonts use the previous line's leading for accents? :/ )
				if (realLeading != 0)
					skipAdjust = YES;
			}
			if (!skipAdjust) {
				UniChar ch = 0xC0;
				CGGlyph glyph;

				// there does not seem to be an AppKit API for this...
				if (CTFontGetGlyphsForCharacters((CTFontRef) f, &ch, &glyph, 1) != false) {
					NSRect bbox;

					bbox = [f boundingRectForGlyph:glyph];
					if (NSMaxY(bbox) > realAscent)
						doAdjust = YES;
					if (-realDescent > NSMinY(bbox))
						doAdjust = YES;
				}
			}
			// TODO vertical

			v = [f ascender];
			// TODO get this one back out
			if (doAdjust)
				v += 0.2 * ([f ascender] + [f descender]);
			//v = floor(v + 0.5);
			if (li.ascent < v)
				li.ascent = v;

			v = -[f descender];// floor(-[f descender] + 0.5);
			if (li.descent < v)
				li.descent = v;

			v = [f leading];//floor([f leading] + 0.5);
			if (li.leading < v)
				li.leading = v;
		}];
		li.ascent = floor(li.ascent + 0.5);
		li.descent = floor(li.descent + 0.5);
		li.leading = floor(li.leading + 0.5);
		[tl->lineInfo addObject:li];
		[li release];
		index = glyphRange.location + glyphRange.length;
	}

	// and finally copy the UTF-16 to UTF-8 index conversion table
	tl->u16tou8 = attrstrCopyUTF16ToUTF8(s, &(tl->nu16tou8));

	return tl;
}

void uiDrawFreeTextLayout(uiDrawTextLayout *tl)
{
	uiFree(tl->u16tou8);
	[tl->lineInfo release];
	[tl->layoutManager release];
	[tl->container release];
	[tl->attrstr release];
	uiFree(tl);
}

// TODO document that (x,y) is the top-left corner of the *entire frame*
void uiDrawText(uiDrawContext *c, uiDrawTextLayout *tl, double x, double y)
{
	NSGraphicsContext *gc;

	CGContextFlush(c->c);		// just to be safe
	[NSGraphicsContext saveGraphicsState];
	gc = [NSGraphicsContext graphicsContextWithGraphicsPort:c->c flipped:YES];
	[NSGraphicsContext setCurrentContext:gc];

	// TODO is this the right point?
	// TODO does this draw with the proper default styles?
	[tl->layoutManager drawGlyphsForGlyphRange:[tl->layoutManager glyphRangeForTextContainer:tl->container]
		atPoint:NSMakePoint(x, y)];

	[gc flushGraphics];		// just to be safe
	[NSGraphicsContext restoreGraphicsState];
	// TODO release gc?
}

// TODO update all of these {
// TODO document that the width and height of a layout is not necessarily the sum of the widths and heights of its constituent lines; this is definitely untrue on OS X, where lines are placed in such a way that the distance between baselines is always integral
// TODO width doesn't include trailing whitespace...
// TODO figure out how paragraph spacing should play into this
// }
void uiDrawTextLayoutExtents(uiDrawTextLayout *tl, double *width, double *height)
{
	NSRect r;

	// see https://developer.apple.com/library/content/documentation/Cocoa/Conceptual/TextLayout/Tasks/StringHeight.html
	r = [tl->layoutManager usedRectForTextContainer:tl->container];
	*width = r.size.width;
	*height = r.size.height;
}

int uiDrawTextLayoutNumLines(uiDrawTextLayout *tl)
{
	return [tl->lineInfo count];
}

void uiDrawTextLayoutLineByteRange(uiDrawTextLayout *tl, int line, size_t *start, size_t *end)
{
	lineInfo *li;

	li = (lineInfo *) [tl->lineInfo objectAtIndex:line];
	*start = tl->u16tou8[li.characterRange.location];
	*end = tl->u16tou8[li.characterRange.location + li.characterRange.length];
}

void uiDrawTextLayoutLineGetMetrics(uiDrawTextLayout *tl, int line, uiDrawTextLayoutLineMetrics *m)
{
	lineInfo *li;

	li = (lineInfo *) [tl->lineInfo objectAtIndex:line];

	m->X = li.lineRect.origin.x;
	m->Y = li.lineRect.origin.y;
	// if we use li.lineRect here we get the whole line, not just the part with stuff in it
	m->Width = li.lineUsedRect.size.width;
	m->Height = li.lineRect.size.height;

	// TODO is this correct?
	m->BaselineY = (m->Y + m->Height) - li.baselineOffset;
	m->Ascent = li.ascent;
	m->Descent = li.descent;
	m->Leading = li.leading;

	// TODO
	m->ParagraphSpacingBefore = 0;
	m->LineHeightSpace = 0;
	m->LineSpacing = 0;
	m->ParagraphSpacing = 0;
}

void uiDrawTextLayoutHitTest(uiDrawTextLayout *tl, double x, double y, uiDrawTextLayoutHitTestResult *result)
{
#if 0 /* TODO */
	CFIndex i;
	CTLineRef line;
	CFIndex pos;

	if (y >= 0) {
		for (i = 0; i < tl->nLines; i++) {
			double ltop, lbottom;

			ltop = tl->lineMetrics[i].Y;
			lbottom = ltop + tl->lineMetrics[i].Height;
			if (y >= ltop && y < lbottom)
				break;
		}
		result->YPosition = uiDrawTextLayoutHitTestPositionInside;
		if (i == tl->nLines) {
			i--;
			result->YPosition = uiDrawTextLayoutHitTestPositionAfter;
		}
	} else {
		i = 0;
		result->YPosition = uiDrawTextLayoutHitTestPositionBefore;
	}
	result->Line = i;

	result->XPosition = uiDrawTextLayoutHitTestPositionInside;
	if (x < tl->lineMetrics[i].X) {
		result->XPosition = uiDrawTextLayoutHitTestPositionBefore;
		// and forcibly return the first character
		x = tl->lineMetrics[i].X;
	} else if (x > (tl->lineMetrics[i].X + tl->lineMetrics[i].Width)) {
		result->XPosition = uiDrawTextLayoutHitTestPositionAfter;
		// and forcibly return the last character
		x = tl->lineMetrics[i].X + tl->lineMetrics[i].Width;
	}

	line = (CTLineRef) CFArrayGetValueAtIndex(tl->lines, i);
	// TODO copy the part from the docs about this point
	pos = CTLineGetStringIndexForPosition(line, CGPointMake(x, 0));
	if (pos == kCFNotFound) {
		// TODO
	}
	result->Pos = tl->u16tou8[pos];
#endif
}

void uiDrawTextLayoutByteRangeToRectangle(uiDrawTextLayout *tl, size_t start, size_t end, uiDrawTextLayoutByteRangeRectangle *r)
{
}
