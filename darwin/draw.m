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

void uiDrawPathNewFigureWithArc(uiDrawPath *p, double xCenter, double yCenter, double radius, double startAngle, double sweep, int negative)
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
	uiDrawPathArcTo(p, xCenter, yCenter, radius, startAngle, sweep, negative);
}

void uiDrawPathLineTo(uiDrawPath *p, double x, double y)
{
	if (p->ended)
		complain("attempt to add line to ended path in uiDrawPathLineTo()");
	CGPathAddLineToPoint(p->path, NULL, x, y);
}

void uiDrawPathArcTo(uiDrawPath *p, double xCenter, double yCenter, double radius, double startAngle, double sweep, int negative)
{
	bool cw;

	if (p->ended)
		complain("attempt to add arc to ended path in uiDrawPathArcTo()");
	if (sweep > 2 * M_PI)
		sweep = 2 * M_PI;
	cw = false;
	if (negative)
		cw = true;
	CGPathAddArc(p->path, NULL,
		xCenter, yCenter,
		radius,
		startAngle, startAngle + sweep,
		cw);
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
	CGFloat height;				// needed for text; see below
};

uiDrawContext *newContext(CGContextRef ctxt, CGFloat height)
{
	uiDrawContext *c;

	c = uiNew(uiDrawContext);
	c->c = ctxt;
	c->height = height;
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
	CGPathRef dashPath;
	CGFloat *dashes;
	size_t i;
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
		break;
	case uiDrawLineJoinRound:
		join = kCGLineJoinRound;
		break;
	case uiDrawLineJoinBevel:
		join = kCGLineJoinBevel;
		break;
	}

	// create a temporary path identical to the previous one
	dashPath = (CGPathRef) path->path;
	if (p->NumDashes != 0) {
		dashes = (CGFloat *) uiAlloc(p->NumDashes * sizeof (CGFloat), "CGFloat[]");
		for (i = 0; i < p->NumDashes; i++)
			dashes[i] = p->Dashes[i];
		dashPath = CGPathCreateCopyByDashingPath(path->path,
			NULL,
			p->DashPhase,
			dashes,
			p->NumDashes);
		uiFree(dashes);
	}
	// the documentation is wrong: this produces a path suitable for calling CGPathCreateCopyByStrokingPath(), not for filling directly
	// the cast is safe; we never modify the CGPathRef and always cast it back to a CGPathRef anyway
	p2.path = (CGMutablePathRef) CGPathCreateCopyByStrokingPath(dashPath,
		NULL,
		p->Thickness,
		cap,
		join,
		p->MiterLimit);
	if (p->NumDashes != 0)
		CGPathRelease(dashPath);

	// always draw stroke fills using the winding rule
	// otherwise intersecting figures won't draw correctly
	p2.fillMode = uiDrawFillModeWinding;
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

static void m2c(uiDrawMatrix *m, CGAffineTransform *c)
{
	c->a = m->M11;
	c->b = m->M12;
	c->c = m->M21;
	c->d = m->M22;
	c->tx = m->M31;
	c->ty = m->M32;
}

static void c2m(CGAffineTransform *c, uiDrawMatrix *m)
{
	m->M11 = c->a;
	m->M12 = c->b;
	m->M21 = c->c;
	m->M22 = c->d;
	m->M31 = c->tx;
	m->M32 = c->ty;
}

// TODO get rid of the separate setIdentity()
void uiDrawMatrixSetIdentity(uiDrawMatrix *m)
{
	setIdentity(m);
}

void uiDrawMatrixTranslate(uiDrawMatrix *m, double x, double y)
{
	CGAffineTransform c;

	m2c(m, &c);
	c = CGAffineTransformTranslate(c, x, y);
	c2m(&c, m);
}

void uiDrawMatrixScale(uiDrawMatrix *m, double xCenter, double yCenter, double x, double y)
{
	CGAffineTransform c;
	double xt, yt;

	m2c(m, &c);
	// TODO explain why the translation must come first
	xt = x;
	yt = y;
	scaleCenter(xCenter, yCenter, &xt, &yt);
	c = CGAffineTransformTranslate(c, xt, yt);
	c = CGAffineTransformScale(c, x, y);
	// TODO undo the translation?
	c2m(&c, m);
}

void uiDrawMatrixRotate(uiDrawMatrix *m, double x, double y, double amount)
{
	CGAffineTransform c;

	m2c(m, &c);
	c = CGAffineTransformTranslate(c, x, y);
	c = CGAffineTransformRotate(c, amount);
	c = CGAffineTransformTranslate(c, -x, -y);
	c2m(&c, m);
}

void uiDrawMatrixSkew(uiDrawMatrix *m, double x, double y, double xamount, double yamount)
{
	fallbackSkew(m, x, y, xamount, yamount);
}

void uiDrawMatrixMultiply(uiDrawMatrix *dest, uiDrawMatrix *src)
{
	CGAffineTransform c;
	CGAffineTransform d;

	m2c(dest, &c);
	m2c(src, &d);
	c = CGAffineTransformConcat(c, d);
	c2m(&c, dest);
}

// there is no test for invertibility; CGAffineTransformInvert() is merely documented as returning the matrix unchanged if it isn't invertible
// therefore, special care must be taken to catch matrices who are their own inverses
// TODO figure out which matrices these are and do so
int uiDrawMatrixInvertible(uiDrawMatrix *m)
{
	CGAffineTransform c, d;

	m2c(m, &c);
	d = CGAffineTransformInvert(c);
	return CGAffineTransformEqualToTransform(c, d) == false;
}

int uiDrawMatrixInvert(uiDrawMatrix *m)
{
	CGAffineTransform c, d;

	m2c(m, &c);
	d = CGAffineTransformInvert(c);
	if (CGAffineTransformEqualToTransform(c, d))
		return 0;
	c2m(&d, m);
	return 1;
}

void uiDrawMatrixTransformPoint(uiDrawMatrix *m, double *x, double *y)
{
	CGAffineTransform c;
	CGPoint p;

	m2c(m, &c);
	p = CGPointApplyAffineTransform(CGPointMake(*x, *y), c);
	*x = p.x;
	*y = p.y;
}

void uiDrawMatrixTransformSize(uiDrawMatrix *m, double *x, double *y)
{
	CGAffineTransform c;
	CGSize s;

	m2c(m, &c);
	s = CGSizeApplyAffineTransform(CGSizeMake(*x, *y), c);
	*x = s.width;
	*y = s.height;
}

void uiDrawTransform(uiDrawContext *c, uiDrawMatrix *m)
{
	CGAffineTransform cm;

	m2c(m, &cm);
	CGContextConcatCTM(c->c, cm);
}

void uiDrawClip(uiDrawContext *c, uiDrawPath *path)
{
	if (!path->ended)
		complain("path not ended in uiDrawClip()");
	CGContextAddPath(c->c, (CGPathRef) (path->path));
	switch (path->fillMode) {
	case uiDrawFillModeWinding:
		CGContextClip(c->c);
		break;
	case uiDrawFillModeAlternate:
		CGContextEOClip(c->c);
		break;
	}
}

// TODO figure out what besides transforms these save/restore on all platforms
void uiDrawSave(uiDrawContext *c)
{
	CGContextSaveGState(c->c);
}

void uiDrawRestore(uiDrawContext *c)
{
	CGContextRestoreGState(c->c);
}

// TODO for all relevant routines, make sure we are freeing memory correctly
// TODO make sure allocation failures throw exceptions?
struct uiDrawFontFamilies {
	CFArrayRef fonts;
};

uiDrawFontFamilies *uiDrawListFontFamilies(void)
{
	uiDrawFontFamilies *ff;

	ff = uiNew(uiDrawFontFamilies);
	// TODO is there a way to get an error reason?
	ff->fonts = CTFontManagerCopyAvailableFontFamilyNames();
	if (ff->fonts == NULL)
		complain("error getting available font names (no reason specified)");
	return ff;
}

uintmax_t uiDrawFontFamiliesNumFamilies(uiDrawFontFamilies *ff)
{
	return CFArrayGetCount(ff->fonts);
}

char *uiDrawFontFamiliesFamily(uiDrawFontFamilies *ff, uintmax_t n)
{
	CFStringRef familystr;
	char *family;

	familystr = (CFStringRef) CFArrayGetValueAtIndex(ff->fonts, n);
	// TODO create a uiDarwinCFStringToText()?
	family = uiDarwinNSStringToText((NSString *) familystr);
	// Get Rule means we do not free familystr
	return family;
}

void uiDrawFreeFontFamilies(uiDrawFontFamilies *ff)
{
	CFRelease(ff->fonts);
	uiFree(ff);
}

double uiDrawTextSizeToPoints(double textSize)
{
	// TODO
	return 0;
}

double uiDrawPointsToTextSize(double points)
{
	// TODO
	return 0;
}

struct uiDrawTextLayout {
	CFMutableAttributedStringRef mas;
	intmax_t *bytesToCharacters;
};

// TODO this is *really* iffy, but we need to know character offsets...
// TODO clean up the local variable names and improve documentation
static intmax_t *strToCFStrOffsetList(const char *str, CFMutableStringRef *cfstr)
{
	intmax_t *bytesToCharacters;
	intmax_t i, len;

	len = strlen(str);
	bytesToCharacters = (intmax_t *) uiAlloc(len * sizeof (intmax_t), "intmax_t[]");

	*cfstr = CFStringCreateMutable(NULL, 0);
	if (*cfstr == NULL)
		complain("error creating CFMutableStringRef for storing string in uiDrawNewTextLayout()");

	i = 0;
	while (i < len) {
		CFStringRef substr;
		intmax_t n;
		intmax_t j;
		intmax_t pos;

		// figure out how many characters to convert and convert them
		for (n = 1; (i + n - 1) < len; n++) {
			substr = CFStringCreateWithBytes(NULL, (const UInt8 *) (str + i), n, kCFStringEncodingUTF8, false);
			if (substr != NULL)		// found a full character
				break;
		}
		// if this test passes we either:
		// - reached the end of the string without a successful conversion (invalid string)
		// - ran into allocation issues
		if (substr == NULL)
			complain("something bad happened when trying to prepare string in uiDrawNewTextLayout()");

		// now save the character offsets for those bytes
		pos = CFStringGetLength(*cfstr);
		for (j = 0; j < n; j++)
			bytesToCharacters[j] = pos;

		// and add the characters that we converted
		CFStringAppend(*cfstr, substr);
		CFRelease(substr);			// TODO correct?

		// and go to the next
		i += n;
	}

	return bytesToCharacters;
}

static CFMutableDictionaryRef newAttrList(void)
{
	CFMutableDictionaryRef attr;

	attr = CFDictionaryCreateMutable(NULL, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
	if (attr == NULL)
		complain("error creating attribute dictionary in newAttrList()()");
	return attr;
}

static void addFontFamilyAttr(CFMutableDictionaryRef attr, const char *family)
{
	CFStringRef cfstr;

	cfstr = CFStringCreateWithCString(NULL, family, kCFStringEncodingUTF8);
	if (cfstr == NULL)
		complain("error creating font family name CFStringRef in addFontFamilyAttr()");
	CFDictionaryAddValue(attr, kCTFontFamilyNameAttribute, cfstr);
	CFRelease(cfstr);			// dictionary holds its own reference
}

static void addFontSizeAttr(CFMutableDictionaryRef attr, double size)
{
	CFNumberRef n;

	n = CFNumberCreate(NULL, kCFNumberDoubleType, &size);
	CFDictionaryAddValue(attr, kCTFontSizeAttribute, n);
	CFRelease(n);
}

// see http://stackoverflow.com/questions/4810409/does-coretext-support-small-caps/4811371#4811371 and https://git.gnome.org/browse/pango/tree/pango/pangocoretext-fontmap.c
static void addFontSmallCapsAttr(CFMutableDictionaryRef attr)
{
	CFMutableArrayRef outerArray;
	CFMutableDictionaryRef innerDict;
	CFNumberRef numType, numSelector;
	int num;

	outerArray = CFArrayCreateMutable(NULL, 0, &kCFTypeArrayCallBacks);
	if (outerArray == NULL)
		complain("error creating outer CFArray for adding small caps attributes in addFontSmallCapsAttr()");

	// TODO Apple's headers say these values are deprecated, but I'm not sure what they should be replaced with (or whether they should be deleted outright or used concurrently with their replacements); the other answers of the Stack Overflow question has hints though (and TODO inform Pango of this)
	num = kLetterCaseType;
	numType = CFNumberCreate(NULL, kCFNumberIntType, &num);
	num = kSmallCapsSelector;
	numSelector = CFNumberCreate(NULL, kCFNumberIntType, &num);
	innerDict = newAttrList();
	CFDictionaryAddValue(innerDict, kCTFontFeatureTypeIdentifierKey, numType);
	CFRelease(numType);
	CFDictionaryAddValue(innerDict, kCTFontFeatureSelectorIdentifierKey, numSelector);
	CFRelease(numSelector);
	CFArrayAppendValue(outerArray, innerDict);
	CFRelease(innerDict);		// and likewise for CFArray

	CFDictionaryAddValue(attr, kCTFontFeatureSettingsAttribute, outerArray);
	CFRelease(outerArray);
}

static void addFontGravityAttr(CFMutableDictionaryRef dict, uiDrawTextGravity gravity)
{
	// TODO: matrix setting? kCTFontOrientationAttribute? or is it a kCTVerticalFormsAttributeName of the CFAttributedString attributes and thus not part of the CTFontDescriptor?
}

// Named constants for these were NOT added until 10.11, and even then they were added as external symbols instead of macros, so we can't use them directly :(
// kode54 got these for me before I had access to El Capitan; thanks to him.
#define ourNSFontWeightUltraLight -0.800000
#define ourNSFontWeightThin -0.600000
#define ourNSFontWeightLight -0.400000
#define ourNSFontWeightRegular 0.000000
#define ourNSFontWeightMedium 0.230000
#define ourNSFontWeightSemibold 0.300000
#define ourNSFontWeightBold 0.400000
#define ourNSFontWeightHeavy 0.560000
#define ourNSFontWeightBlack 0.620000
static const CGFloat ctWeights[] = {
	// yeah these two have their names swapped; blame Pango
	// TODO note that these names do not necessarily line up with their OS names
	[uiDrawTextWeightThin] = ourNSFontWeightUltraLight,
	[uiDrawTextWeightUltraLight] = ourNSFontWeightThin,
	[uiDrawTextWeightLight] = ourNSFontWeightLight,
	// for this one let's go between Light and Regular
	// TODO figure out if we can rely on the order for these (and the one below)
	[uiDrawTextWeightBook] = ourNSFontWeightLight + ((ourNSFontWeightRegular - ourNSFontWeightLight) / 2),
	[uiDrawTextWeightNormal] = ourNSFontWeightRegular,
	[uiDrawTextWeightMedium] = ourNSFontWeightMedium,
	[uiDrawTextWeightSemiBold] = ourNSFontWeightSemibold,
	[uiDrawTextWeightBold] = ourNSFontWeightBold,
	// for this one let's go between Bold and Heavy
	[uiDrawTextWeightUtraBold] = ourNSFontWeightBold + ((ourNSFontWeightHeavy - ourNSFontWeightBold) / 2),
	[uiDrawTextWeightHeavy] = ourNSFontWeightHeavy,
	[uiDrawTextWeightUltraHeavy] = ourNSFontWeightBlack,
};

// Unfortunately there are still no named constants for these.
// Let's just use normalized widths.
// As far as I can tell (OS X only has condensed fonts, not expanded fonts; TODO), regardless of condensed or expanded, negative means condensed and positive means expanded.
// TODO verify this is correct
static const CGFloat ctStretches[] = {
	[uiDrawTextStretchUltraCondensed] = -1.0,
	[uiDrawTextStretchExtraCondensed] = -0.75,
	[uiDrawTextStretchCondensed] = -0.5,
	[uiDrawTextStretchSemiCondensed] = -0.25,
	[uiDrawTextStretchNormal] = 0.0,
	[uiDrawTextStretchSemiExpanded] = 0.25,
	[uiDrawTextStretchExpanded] = 0.5,
	[uiDrawTextStretchExtraExpanded] = 0.75,
	[uiDrawTextStretchUltraExpanded] = 1.0,
};

struct closeness {
	CFIndex index;
	CGFloat weight;
	CGFloat italic;
	CGFloat stretch;
	CGFloat distance;
};

// see below for details
// TODO document that font matching is closest match but the search method is OS defined
CTFontDescriptorRef matchTraits(CTFontDescriptorRef against, uiDrawTextWeight weight, uiDrawTextItalic italic, uiDrawTextStretch stretch)
{
	CGFloat targetWeight;
	CGFloat italicCloseness, obliqueCloseness, normalCloseness;
	CGFloat targetStretch;
	CFArrayRef matching;
	CFIndex i, n;
	struct closeness *closeness;
	CTFontDescriptorRef current;
	CTFontDescriptorRef out;

	targetWeight = ctWeights[weight];
	switch (italic) {
	case uiDrawTextItalicNormal:
		italicCloseness = 1;
		obliqueCloseness = 1;
		normalCloseness = 0;
		break;
	case uiDrawTextItalicOblique:
		italicCloseness = 0.5;
		obliqueCloseness = 0;
		normalCloseness = 1;
		break;
	case uiDrawTextItalicItalic:
		italicCloseness = 0;
		obliqueCloseness = 0.5;
		normalCloseness = 1;
		break;
	}
	targetStretch = ctStretches[stretch];

	matching = CTFontDescriptorCreateMatchingFontDescriptors(against, NULL);
	if (matching == NULL)
		// no matches; give the original back and hope for the best
		return against;
	n = CFArrayGetCount(matching);
	if (n == 0) {
		// likewise
		CFRelease(matching);
		return against;
	}

	closeness = (struct closeness *) uiAlloc(n * sizeof (struct closeness), "struct closeness[]");
	for (i = 0; i < n; i++) {
		CFDictionaryRef traits;
		CFNumberRef cfnum;
		CTFontSymbolicTraits symbolic;

		closeness[i].index = i;

		current = CFArrayGetValueAtIndex(matching, i);
		traits = CTFontDescriptorCopyAttribute(current, kCTFontTraitsAttribute);
		if (traits == NULL) {
			// couldn't get traits; be safe by ranking it lowest
			// TODO figure out what the longest possible distances are
			closeness[i].weight = 3;
			closeness[i].italic = 2;
			closeness[i].stretch = 3;
			continue;
		}

		symbolic = 0;			// assume no symbolic traits if none are listed
		cfnum = CFDictionaryGetValue(traits, kCTFontSymbolicTrait);
		if (cfnum != NULL) {
			SInt32 s;

			if (CFNumberGetValue(cfnum, kCFNumberSInt32Type, &s) == false)
				complain("error getting symbolic traits in matchTraits()");
			symbolic = (CTFontSymbolicTraits) s;
			// Get rule; do not release cfnum
		}

		// now try weight
		cfnum = CFDictionaryGetValue(traits, kCTFontWeightTrait);
		if (cfnum != NULL) {
			CGFloat val;

			// TODO instead of complaining for this and width, should we just fall through to the default?
			if (CFNumberGetValue(cfnum, kCFNumberCGFloatType, &val) == false)
				complain("error getting weight value in matchTraits()");
			closeness[i].weight = val - targetWeight;
		} else
			// okay there's no weight key; let's try the literal meaning of the symbolic constant
			// TODO is the weight key guaranteed?
			if ((symbolic & kCTFontBoldTrait) != 0)
				closeness[i].weight = ourNSFontWeightBold - targetWeight;
			else
				closeness[i].weight = ourNSFontWeightRegular - targetWeight;

		// italics is a bit harder because Core Text doesn't expose a concept of obliqueness
		// Pango just does a g_strrstr() (backwards case-sensitive search) for "Oblique" in the font's style name (see https://git.gnome.org/browse/pango/tree/pango/pangocoretext-fontmap.c); let's do that too I guess
		if ((symbolic & kCTFontItalicTrait) != 0)
			closeness[i].italic = italicCloseness;
		else {
			CFStringRef styleName;
			BOOL isOblique;

			isOblique = NO;		// default value
			styleName = CTFontDescriptorCopyAttribute(current, kCTFontStyleNameAttribute);
			if (styleName != NULL) {
				CFRange range;

				// note the use of the toll-free bridge for the string literal, since CFSTR() *can* return NULL
				range = CFStringFind(styleName, (CFStringRef) @"Oblique", kCFCompareBackwards);
				if (range.location != kCFNotFound)
					isOblique = YES;
				CFRelease(styleName);
			}
			if (isOblique)
				closeness[i].italic = obliqueCloseness;
			else
				closeness[i].italic = normalCloseness;
		}

		// now try width
		cfnum = CFDictionaryGetValue(traits, kCTFontWidthTrait);
		if (cfnum != NULL) {
			CGFloat val;

			if (CFNumberGetValue(cfnum, kCFNumberCGFloatType, &val) == false)
				complain("error getting width value in matchTraits()");
			closeness[i].stretch = val - targetStretch;
		} else
			// okay there's no width key; let's try the literal meaning of the symbolic constant
			// TODO is the width key guaranteed?
			if ((symbolic & kCTFontExpandedTrait) != 0)
				closeness[i].stretch = 1.0 - targetStretch;
			else if ((symbolic & kCTFontCondensedTrait) != 0)
				closeness[i].stretch = -1.0 - targetStretch;
			else
				closeness[i].stretch = 0.0 - targetStretch;

		CFRelease(traits);
	}

	// now figure out the 3-space difference between the three and sort by that
	for (i = 0; i < n; i++) {
		CGFloat weight, italic, stretch;

		weight = closeness[i].weight;
		weight *= weight;
		italic = closeness[i].italic;
		italic *= italic;
		stretch = closeness[i].stretch;
		stretch *= stretch;
		closeness[i].distance = sqrt(weight + italic + stretch);
	}
	qsort_b(closeness, n, sizeof (struct closeness), ^(const void *aa, const void *bb) {
		const struct closeness *a = (const struct closeness *) aa;
		const struct closeness *b = (const struct closeness *) bb;

		// via http://www.gnu.org/software/libc/manual/html_node/Comparison-Functions.html#Comparison-Functions
		// TODO is this really the best way? isn't it the same as if (*a < *b) return -1; if (*a > *b) return 1; return 0; ?
		return (a->distance > b->distance) - (a->distance < b->distance);
	});
	// and the first element of the sorted array is what we want
	out = CFArrayGetValueAtIndex(matching, closeness[0].index);
	CFRetain(out);			// get rule

	// release everything
	uiFree(closeness);
	CFRelease(matching);
	// and release the original descriptor since we no longer need it
	CFRelease(against);

	return out;
}

uiDrawTextLayout *uiDrawNewTextLayout(const char *str, const uiDrawInitialTextStyle *initialStyle)
{
	uiDrawTextLayout *layout;
	CFMutableStringRef cfstr;
	CFMutableDictionaryRef attr;
	CTFontDescriptorRef desc;
	CTFontRef font;
	CFAttributedStringRef immutable;

	layout = uiNew(uiDrawTextLayout);

	layout->bytesToCharacters = strToCFStrOffsetList(str, &cfstr);

	attr = newAttrList();
	addFontFamilyAttr(attr, initialStyle->Family);
	addFontSizeAttr(attr, initialStyle->Size);
	if (initialStyle->SmallCaps)
		addFontSmallCapsAttr(attr);
	addFontGravityAttr(attr, initialStyle->Gravity);

	desc = CTFontDescriptorCreateWithAttributes(attr);
	// TODO release attr?

	// unfortunately OS X requires an EXACT MATCH for the traits, otherwise it will *drop all the traits*
	// we want a nearest match, so we have to do it ourselves
	// TODO this does not preserve small caps
	desc = matchTraits(desc, initialStyle->Weight, initialStyle->Italic, initialStyle->Stretch);

	// specify the initial size again just to be safe
	font = CTFontCreateWithFontDescriptor(desc, initialStyle->Size, NULL);
	// TODO release desc?

	attr = newAttrList();
	CFDictionaryAddValue(attr, kCTFontAttributeName, font);
	CFRelease(font);

	immutable = CFAttributedStringCreate(NULL, cfstr, attr);
	if (immutable == NULL)
		complain("error creating immutable attributed string in uiDrawNewTextLayout()");
	CFRelease(cfstr);
	CFRelease(attr);

	layout->mas = CFAttributedStringCreateMutableCopy(NULL, 0, immutable);
	if (layout->mas == NULL)
		complain("error creating attributed string in uiDrawNewTextLayout()");
	CFRelease(immutable);

	return layout;
}

void uiDrawFreeTextLayout(uiDrawTextLayout *layout)
{
	CFRelease(layout->mas);
	uiFree(layout->bytesToCharacters);
	uiFree(layout);
}

// Core Text doesn't draw onto a flipped view correctly; we have to do this
// see the iOS bits of the first example at https://developer.apple.com/library/mac/documentation/StringsTextFonts/Conceptual/CoreText_Programming/LayoutOperations/LayoutOperations.html#//apple_ref/doc/uid/TP40005533-CH12-SW1 (iOS is naturally flipped)
// TODO how is this affected by the CTM?
static void prepareContextForText(uiDrawContext *c, double *y)
{
	CGContextSaveGState(c->c);
	CGContextTranslateCTM(c->c, 0, c->height);
	CGContextScaleCTM(c->c, 1.0, -1.0);
	CGContextSetTextMatrix(c->c, CGAffineTransformIdentity);

	// wait, that's not enough; we need to offset y values to account for our new flipping
	*y = c->height - *y;
}

// TODO how does this behave with multiple lines? on other platforms? is there a generic way to draw this unbounded?
void uiDrawText(uiDrawContext *c, double x, double y, uiDrawTextLayout *layout)
{
	CTLineRef line;
	CGRect bounds;

	prepareContextForText(c, &y);

	line = CTLineCreateWithAttributedString(layout->mas);
	if (line == NULL)
		complain("error creating CTLine object in uiDrawText()");

	// oh, and (x, y) is the bottom-left corner; we need the top-left
	// remember that we're flipped, so we subtract
	bounds = CTLineGetImageBounds(line, c->c);
	// though CTLineGetImageBounds() returns CGRectNull on error, it also returns CGRectNull on an empty string, so we can't reasonably check for error
	y -= bounds.size.height;
	CGContextSetTextPosition(c->c, x, y);

	// and now we can FINALLY draw the line
	CTLineDraw(line, c->c);
	CFRelease(line);

	CGContextRestoreGState(c->c);
}
