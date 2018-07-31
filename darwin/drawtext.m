// 7 march 2018
#import "uipriv_darwin.h"
#import "draw.h"
#import "attrstr.h"

#define inRange(min, x, max) ((min) <= (x) && (x) <= (max))

// problem: for a CTFrame made from an empty string, the CTLine array will be empty, and we will crash when doing anything requiring a CTLine
// solution: for those cases, maintain a separate framesetter just for computing those things
// in the usual case, the separate copy will just be identical to the regular one, with extra references to everything within
@interface uiprivTextFrame : NSObject {
	CFAttributedStringRef attrstr;
	NSArray *backgroundParams;
	CTFramesetterRef framesetter;
	CGSize size;
	CGPathRef path;
	CTFrameRef frame;
}
- (id)initWithLayoutParams:(uiDrawTextLayoutParams *)p;
- (void)draw:(uiDrawContext *)c textLayout:(uiDrawTextLayout *)tl at:(double)x y:(double)y;
- (void)returnWidth:(double *)width height:(double *)height;
- (CFArrayRef)lines;
- (void)lineOrigins:(CFRange)range origins:(CGPoint*)origins;
@end

struct uiDrawTextLayout {
	uiprivTextFrame *frame;
	uiprivTextFrame *forLines;
	BOOL empty;

	// for converting CFAttributedString indices from/to byte offsets
	size_t *u8tou16;
	size_t nUTF8;
	size_t *u16tou8;
	size_t nUTF16;
};

CGColorRef mkcolor(double r, double g, double b, double a)
{
	CGColorSpaceRef colorspace;
	CGColorRef color;
	CGFloat components[4];

	// TODO we should probably just create this once and recycle it throughout program execution...
	colorspace = CGColorSpaceCreateWithName(kCGColorSpaceSRGB);
	if (colorspace == NULL) {
		// TODO
	}
	components[0] = r;
	components[1] = g;
	components[2] = b;
	components[3] = a;
	color = CGColorCreate(colorspace, components);
	CFRelease(colorspace);
	return color;
}

@implementation uiprivDrawTextBackgroundParams

- (id)initWithStart:(size_t)s end:(size_t)e r:(double)red g:(double)green b:(double)blue a:(double)alpha
{
	self = [super init];
	if (self) {
		self->start = s;
		self->end = e;
		self->r = red;
		self->g = green;
		self->b = blue;
		self->a = alpha;
	}
	return self;
}

- (void)draw:(CGContextRef)c layout:(uiDrawTextLayout *)layout at:(double)x y:(double)y utf8Mapping:(const size_t *)u16tou8
{
	long startI = u16tou8[self->start];
	long endI = u16tou8[self->end];

	CGColorRef color = mkcolor(self->r, self->g, self->b, self->a);

	double frameHeight;
	[layout->frame returnWidth:NULL height:&frameHeight];

	CFArrayRef lines = [layout->frame lines];
	size_t numOfLines = CFArrayGetCount(lines);
	CGPoint lineOrigins[numOfLines];

	[layout->frame lineOrigins:CFRangeMake(0, numOfLines) origins:lineOrigins];
	for (size_t i = 0; i < numOfLines; i++) {
		CTLineRef line = CFArrayGetValueAtIndex(lines, i);
		CFRange lr = CTLineGetStringRange(line);
		CFIndex lineStart = lr.location;
		CFIndex lineEnd = lineStart + lr.length;
		if (inRange(startI, lineStart, endI) ||
			inRange(startI, lineEnd, endI) || 
			inRange(lineStart, startI, lineEnd) ||
			inRange(lineStart, endI, lineEnd)) {

			CFArrayRef glyphruns = CTLineGetGlyphRuns(line);
			size_t numOfRuns = CFArrayGetCount(glyphruns);
			for (size_t j = 0; j < numOfRuns; j++) {
				// from https://stackoverflow.com/a/5341819/2352201
				CTRunRef run = CFArrayGetValueAtIndex(glyphruns, j);
				CFRange runRange = CTRunGetStringRange(run);
				CFIndex runStart = runRange.location;
				CFIndex runEnd = runStart + runRange.length;
				if (startI <= runStart && runEnd <= endI) {
					CGRect rect;

					CGFloat ascent; //height above the baseline
					CGFloat descent; //height below the baseline
					rect.size.width = CTRunGetTypographicBounds(run, CFRangeMake(0, 0), &ascent, &descent, NULL);
					rect.size.height = ascent + descent;

					rect.origin.x = lineOrigins[i].x + x + CTLineGetOffsetForStringIndex(line, runRange.location, NULL);
					rect.origin.y = (frameHeight - lineOrigins[i].y) + y - ascent + descent;
					rect.origin.y -= descent;

					CGContextAddRect(c, rect);
					CGContextSetFillColorWithColor(c, color);
					CGContextDrawPath(c, kCGPathFill);
				}
			}
		}
	}

	CFRelease(color);
}

@end

@implementation uiprivTextFrame

- (id)initWithLayoutParams:(uiDrawTextLayoutParams *)p
{
	CFRange range;
	CGFloat cgwidth;
	CFRange unused;
	CGRect rect;

	self = [super init];
	if (self) {
		self->attrstr = uiprivAttributedStringToCFAttributedString(p, &(self->backgroundParams));
		// TODO kCTParagraphStyleSpecifierMaximumLineSpacing, kCTParagraphStyleSpecifierMinimumLineSpacing, kCTParagraphStyleSpecifierLineSpacingAdjustment for line spacing
		self->framesetter = CTFramesetterCreateWithAttributedString(self->attrstr);
		if (self->framesetter == NULL) {
			// TODO
		}

		range.location = 0;
		range.length = CFAttributedStringGetLength(self->attrstr);

		cgwidth = (CGFloat) (p->Width);
		if (cgwidth < 0)
			cgwidth = CGFLOAT_MAX;
		self->size = CTFramesetterSuggestFrameSizeWithConstraints(self->framesetter,
			range,
			// TODO kCTFramePathWidthAttributeName?
			NULL,
			CGSizeMake(cgwidth, CGFLOAT_MAX),
			&unused);			// not documented as accepting NULL (TODO really?)

		rect.origin = CGPointZero;
		rect.size = self->size;
		self->path = CGPathCreateWithRect(rect, NULL);
		self->frame = CTFramesetterCreateFrame(self->framesetter,
			range,
			self->path,
			// TODO kCTFramePathWidthAttributeName?
			NULL);
		if (self->frame == NULL) {
			// TODO
		}
	}
	return self;
}

- (void)dealloc
{
	CFRelease(self->frame);
	CFRelease(self->path);
	CFRelease(self->framesetter);
	[self->backgroundParams release];
	CFRelease(self->attrstr);
	[super dealloc];
}

- (void)draw:(uiDrawContext *)c textLayout:(uiDrawTextLayout *)tl at:(double)x y:(double)y
{
	uiprivDrawTextBackgroundParams *dtb;
	CGAffineTransform textMatrix;

	CGContextSaveGState(c->c);
	// save the text matrix because it's not part of the graphics state
	textMatrix = CGContextGetTextMatrix(c->c);

	for (dtb in self->backgroundParams)
		[dtb draw:c->c layout:tl at:x y:y utf8Mapping:tl->u16tou8];

	// Core Text doesn't draw onto a flipped view correctly; we have to pretend it was unflipped
	// see the iOS bits of the first example at https://developer.apple.com/library/mac/documentation/StringsTextFonts/Conceptual/CoreText_Programming/LayoutOperations/LayoutOperations.html#//apple_ref/doc/uid/TP40005533-CH12-SW1 (iOS is naturally flipped)
	// TODO how is this affected by a non-identity CTM?
	CGContextTranslateCTM(c->c, 0, c->height);
	CGContextScaleCTM(c->c, 1.0, -1.0);
	CGContextSetTextMatrix(c->c, CGAffineTransformIdentity);

	// wait, that's not enough; we need to offset y values to account for our new flipping
	// TODO explain this calculation
	y = c->height - self->size.height - y;

	// CTFrameDraw() draws in the path we specified when creating the frame
	// this means that in our usage, CTFrameDraw() will draw at (0,0)
	// so move the origin to be at (x,y) instead
	// TODO are the signs correct?
	CGContextTranslateCTM(c->c, x, y);

	CTFrameDraw(self->frame, c->c);

	CGContextSetTextMatrix(c->c, textMatrix);
	CGContextRestoreGState(c->c);
}

- (void)returnWidth:(double *)width height:(double *)height
{
	if (width != NULL)
		*width = self->size.width;
	if (height != NULL)
		*height = self->size.height;
}

- (CFArrayRef)lines
{
	return CTFrameGetLines(self->frame);
}

- (void)lineOrigins:(CFRange)range origins:(CGPoint*)origins
{
	CTFrameGetLineOrigins(self->frame, range, origins);
}

@end

uiDrawTextLayout *uiDrawNewTextLayout(uiDrawTextLayoutParams *p)
{
	uiDrawTextLayout *tl;

	tl = uiprivNew(uiDrawTextLayout);
	tl->frame = [[uiprivTextFrame alloc] initWithLayoutParams:p];
	if (uiAttributedStringLen(p->String) != 0)
		tl->forLines = [tl->frame retain];
	else {
		uiAttributedString *space;
		uiDrawTextLayoutParams p2;

		tl->empty = YES;
		space = uiNewAttributedString(" ");
		p2 = *p;
		p2.String = space;
		tl->forLines = [[uiprivTextFrame alloc] initWithLayoutParams:&p2];
		uiFreeAttributedString(space);
	}

	// and finally copy the UTF-8/UTF-16 conversion tables
	tl->u8tou16 = uiprivAttributedStringCopyUTF8ToUTF16Table(p->String, &(tl->nUTF8));
	tl->u16tou8 = uiprivAttributedStringCopyUTF16ToUTF8Table(p->String, &(tl->nUTF16));
	return tl;
}

void uiDrawFreeTextLayout(uiDrawTextLayout *tl)
{
	uiprivFree(tl->u16tou8);
	uiprivFree(tl->u8tou16);
	[tl->forLines release];
	[tl->frame release];
	uiprivFree(tl);
}

// TODO document that (x,y) is the top-left corner of the *entire frame*
void uiDrawText(uiDrawContext *c, uiDrawTextLayout *tl, double x, double y)
{
	[tl->frame draw:c textLayout:tl at:x y:y];
}

// TODO document that the width and height of a layout is not necessarily the sum of the widths and heights of its constituent lines
// TODO width doesn't include trailing whitespace...
// TODO figure out how paragraph spacing should play into this
// TODO standardize and document the behavior of this on an empty layout
void uiDrawTextLayoutExtents(uiDrawTextLayout *tl, double *width, double *height)
{
	// TODO explain this, given the above
	[tl->frame returnWidth:width height:NULL];
	[tl->forLines returnWidth:NULL height:height];
}
