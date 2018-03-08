// 7 march 2018
#import "uipriv_darwin.h"
#import "draw.h"
#import "attrstr.h"

// problem: for a CTFrame made from an empty string, the CTLine array will be empty, and we will crash when gathering metrics or hit-testing
// solution: for those cases, maintain a separate framesetter just for computing those things
// in the usual case, the separate copy will just be identical to the regular one, with extra references to everything within
struct frame {
	CFAttributedStringRef attrstr;
	NSArray *backgroundBlocks;
	CTFramesetterRef framesetter;
	CGSize size;
	CGPathRef path;
	CTFrameRef frame;
};

struct uiDrawTextLayout {
	struct frame forDrawing;
	struct frame forMetrics;
};

static void paramsToFrame(uiDrawTextLayoutParams *params, struct frame *frame)
{
	CFRange range;
	CGFloat width;
	CFRange unused;
	CGRect rect;

	frame->attrstr = uiprivAttributedStringToCFAttributedString(p, &(frame->backgroundBlocks));
	// TODO kCTParagraphStyleSpecifierMaximumLineSpacing, kCTParagraphStyleSpecifierMinimumLineSpacing, kCTParagraphStyleSpecifierLineSpacingAdjustment for line spacing
	frame->framesetter = CTFramesetterCreateWithAttributedString(tl->attrstr);
	if (frame->framesetter == NULL) {
		// TODO
	}

	range.location = 0;
	range.length = CFAttributedStringGetLength(tl->attrstr);

	cgwidth = (CGFloat) (frame->width);
	if (cgwidth < 0)
		cgwidth = CGFLOAT_MAX;
	frame->size = CTFramesetterSuggestFrameSizeWithConstraints(frame->framesetter,
		range,
		// TODO kCTFramePathWidthAttributeName?
		NULL,
		CGSizeMake(cgwidth, CGFLOAT_MAX),
		&unused);			// not documented as accepting NULL (TODO really?)

	rect.origin = CGPointZero;
	rect.size = frame->size;
	frame->path = CGPathCreateWithRect(rect, NULL);
	frame->frame = CTFramesetterCreateFrame(tl->framesetter,
		range,
		tl->path,
		// TODO kCTFramePathWidthAttributeName?
		NULL);
	if (frame->frame == NULL) {
		// TODO
	}
}

static void freeFrame(struct frame *frame)
{
	CFRelease(frame->frame);
	CFRelease(frame->path);
	CFRelease(frame->framesetter);
	[frame->backgroundBlocks release];
	CFRelease(frame->attrstr);
}

static void retainFrameCopy(struct frame *out, const struct frame *frame)
{
	memcpy(out, frame, sizeof (struct frame));
	CFRetain(out->attrstr);
	[out->backgroundBlocks retain];
	CFRetain(out->framesetter);
	CFRetain(out->path);
	CFRetain(out->frame);
}

uiDrawTextLayout *uiDrawNewTextLayout(uiDrawTextLayoutParams *p)
{
	uiDrawTextLayout *tl;

	tl = uiprivNew(uiDrawTextLayout);
	paramsToFrame(p, &(tl->forDrawing));
	if (uiAttributedStringLength(p->String) != 0)
		retainFrameCopy(&(tl->forMetrics), &(tl->forDrawing));
	else {
		uiAttributedString *space;
		uiDrawTextLayoutParams p2;

		space = uiNewAttributedString(" ");
		p2 = *p;
		p2.String = space;
		paramsToFrame(&p2, &(tl->forMetrics));
		uiFreeAttributedString(space);
	}
	return tl;
}

void uiDrawFreeTextLayout(uiDrawTextLayout *tl)
{
	freeFrame(&(tl->forMetrics));
	freeFrame(&(tl->forDrawing));
	uiprivFree(tl);
}

// uiDrawText() draws tl in c with the top-left point of tl at (x, y).
_UI_EXTERN void uiDrawText(uiDrawContext *c, uiDrawTextLayout *tl, double x, double y);

// uiDrawTextLayoutExtents() returns the width and height of tl
// in width and height. The returned width may be smaller than
// the width passed into uiDrawNewTextLayout() depending on
// how the text in tl is wrapped. Therefore, you can use this
// function to get the actual size of the text layout.
_UI_EXTERN void uiDrawTextLayoutExtents(uiDrawTextLayout *tl, double *width, double *height);

// uiDrawTextLayoutNumLines() returns the number of lines in tl.
// This number will always be greater than or equal to 1; a text
// layout with no text only has one line.
_UI_EXTERN int uiDrawTextLayoutNumLines(uiDrawTextLayout *tl);

// uiDrawTextLayoutLineByteRange() returns the byte indices of the
// text that falls into the given line of tl as [start, end).
_UI_EXTERN void uiDrawTextLayoutLineByteRange(uiDrawTextLayout *tl, int line, size_t *start, size_t *end);
