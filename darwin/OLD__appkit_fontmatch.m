// 3 january 2017
#import "uipriv_darwin.h"

// Stupidity: Core Text requires an **exact match for the entire traits dictionary**, otherwise it will **drop ALL the traits**.
// This seems to be true for every function in Core Text that advertises that it performs font matching; I can confirm at the time of writing this is the case for
// - CTFontDescriptorCreateMatchingFontDescriptors() (though the conditions here seem odd)
// - CTFontCreateWithFontDescriptor()
// - CTFontCreateCopyWithAttributes()
// And as a bonus prize, this also applies to Cocoa's NSFontDescriptor methods as well!
// We have to implement the closest match ourselves.
// This needs to be done early in the matching process; in particular, we have to do this before adding any features (such as small caps), because the matching descriptors won't have those.

struct closeness {
	NSUInteger index;
	double weight;
	double italic;
	double stretch;
	double distance;
};

static double doubleAttr(NSDictionary *traits, NSString *attr)
{
	NSNumber *n;

	n = (NSNumber *) [traits objectForKey:attr];
	return [n doubleValue];
}

struct italicCloseness {
	double normal;
	double oblique;
	double italic;
};

// remember that in closeness, 0 means exact
// in this case, since we define the range, we use 0.5 to mean "close enough" (oblique for italic and italic for oblique) and 1 to mean "not a match"
static const struct italicCloseness italicClosenesses[] = {
	[uiDrawTextItalicNormal] = { 0, 1, 1 },
	[uiDrawTextItalicOblique] = { 1, 0, 0.5 },
	[uiDrawTextItalicItalic] = { 1, 0.5, 0 },
};

// Italics are hard because Core Text does NOT distinguish between italic and oblique.
// All Core Text provides is a slant value and the italic bit of the symbolic traits mask.
// However, Core Text does seem to guarantee (from experimentation; see below) that the slant will be nonzero if and only if the italic bit is set, so we don't need to use the slant value.
// Core Text also seems to guarantee that if a font lists itself as Italic or Oblique by name (font subfamily name, font style name, whatever), it will also have that bit set, so testing this bit does cover all fonts that name themselves as Italic and Oblique. (Again, this is from the below experimentation.)
// TODO there is still one catch that might matter from a user's POV: the reverse is not true — the italic bit can be set even if the style of the font face/subfamily/style isn't named as Italic (for example, script typefaces like Adobe's Palace Script MT Std); I don't know what to do about this... I know how to start: find a script font that has an italic form (Adobe's Palace Script MT Std does not; only Regular and Semibold)
// TODO see if the above applies to Cocoa as well
static double italicCloseness(NSFontDescriptor *desc, NSDictionary *traits, uiDrawTextItalic italic)
{
	const struct italicCloseness *ic = &(italicClosenesses[italic]);
	NSNumber *num;
	NSFontSymbolicTraits symbolic;
	NSString *styleName;
	NSRange range;
	BOOL isOblique;

	num = (NSNumber *) [traits objectForKey:NSFontSymbolicTrait];
	// TODO this should really be a uint32_t-specific one
	symbolic = (NSFontSymbolicTraits) [num unsignedIntegerValue];
	if ((symbolic & NSFontItalicTrait) == 0)
		return ic->normal;

	// Okay, now we know it's either Italic or Oblique
	// Pango's Core Text code just does a g_strrstr() (backwards case-sensitive search) for "Oblique" in the font's style name (see https://git.gnome.org/browse/pango/tree/pango/pangocoretext-fontmap.c); let's do that too I guess
	// note that NSFontFaceAttribute is the Cocoa equivalent of the style name
	isOblique = NO;		// default value
	styleName = (NSString *) [desc objectForKey:NSFontFaceAttribute];
	// TODO is styleName guaranteed?
	// TODO NSLiteralSearch?
	range = [styleName rangeOfString:@"Oblique" options:NSCaseInsensitiveSearch];
	if (range.location != NSNotFound)
		return ic->oblique;
	return ic->italic;
}

static NSFontDescriptor *matchTraits(NSFontDescriptor *against, double targetWeight, uiDrawTextItalic targetItalic, double targetStretch)
{
	NSArray<NSFontDescriptor *> *matching;
	NSUInteger i, n;
	struct closeness *closeness;
	NSFontDescriptor *current;
	NSFontDescriptor *out;

	matching = [against matchingFontDescriptorsWithMandatoryKeys:nil];
	if (matching == nil)
		// no matches; give the original back and hope for the best
		return against;
	n = [matching count];
	if (n == 0) {
		// likewise
//TODO		[matching release];
		return against;
	}

	closeness = (struct closeness *) uiAlloc(n * sizeof (struct closeness), "struct closeness[]");
	for (i = 0; i < n; i++) {
		NSDictionary *traits;

		closeness[i].index = i;
		current = (NSFontDescriptor *) [matching objectAtIndex:i];
		traits = (NSDictionary *) [current objectForKey:NSFontTraitsAttribute];
		if (traits == nil) {
			// couldn't get traits; be safe by ranking it lowest
			// LONGTERM figure out what the longest possible distances are
			closeness[i].weight = 3;
			closeness[i].italic = 2;
			closeness[i].stretch = 3;
			continue;
		}
		closeness[i].weight = doubleAttr(traits, NSFontWeightTrait) - targetWeight;
		closeness[i].italic = italicCloseness(current, traits, targetItalic);
		closeness[i].stretch = doubleAttr(traits, NSFontWidthTrait) - targetStretch;
		// TODO release traits?
	}

	// now figure out the 3-space difference between the three and sort by that
	// TODO merge this loop with the previous loop?
	for (i = 0; i < n; i++) {
		double weight, italic, stretch;

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
		// LONGTERM is this really the best way? isn't it the same as if (*a < *b) return -1; if (*a > *b) return 1; return 0; ?
		return (a->distance > b->distance) - (a->distance < b->distance);
	});
	// and the first element of the sorted array is what we want
	out = (NSFontDescriptor *) [matching objectAtIndex:closeness[0].index];
	// TODO is this correct?
	[out retain];			// get rule

	// release everything
	uiFree(closeness);
//TODO	[matching release];
	// and release the original descriptor since we no longer need it
	[against release];

	return out;
}

// since uiDrawTextWeight effectively corresponds to OS/2 weights (which roughly correspond to GDI, Pango, and DirectWrite weights, and to a lesser(? TODO) degree, CSS weights), let's just do what Core Text does with OS/2 weights
// TODO this will not be correct for system fonts, which use cached values that have no relation to the OS/2 weights; we need to figure out how to reconcile these
// for more information, see https://bugzilla.gnome.org/show_bug.cgi?id=766148 and TODO_put_blog_post_here_once_I_write_it (TODO keep this line when resolving the above TODO)
static const double weightsToCTWeights[] = {
	-1.0,		// 0..99
	-0.7,		// 100..199
	-0.5,		// 200..299
	-0.23,	// 300..399
	0.0,		// 400..499
	0.2,		// 500..599
	0.3,		// 600..699
	0.4,		// 700..799
	0.6,		// 800..899
	0.8,		// 900..999
	1.0,		// 1000
};

static double weightToCTWeight(uiDrawTextWeight weight)
{
	int weightClass;
	double ctclass;
	double rest, weightFloor, nextFloor;

	if (weight <= 0)
		return -1.0;
	if (weight >= 1000)
		return 1.0;

	weightClass = weight / 100;
	rest = (double) weight;
	weightFloor = (double) (weightClass * 100);
	nextFloor = (double) ((weightClass + 1) * 100);
	rest = (rest - weightFloor) / (nextFloor - weightFloor);

	ctclass = weightsToCTWeights[weightClass];
	return fma(rest,
		weightsToCTWeights[weightClass + 1] - ctclass,
		ctclass);
}

// based on what Core Text says about actual fonts (system fonts, system fonts in another folder to avoid using cached values, Adobe Font Folio 11, Google Fonts archive, fonts in Windows 7/8.1/10)
static const double stretchesToCTWidths[] = {
	[uiDrawTextStretchUltraCondensed] = -0.400000,
	[uiDrawTextStretchExtraCondensed] = -0.300000,
	[uiDrawTextStretchCondensed] = -0.200000,
	[uiDrawTextStretchSemiCondensed] = -0.100000,
	[uiDrawTextStretchNormal] = 0.000000,
	[uiDrawTextStretchSemiExpanded] = 0.100000,
	[uiDrawTextStretchExpanded] = 0.200000,
	[uiDrawTextStretchExtraExpanded] = 0.300000,
	// this one isn't present in any of the fonts I tested, but it follows naturally from the pattern of the rest, so... (TODO verify by checking the font files directly)
	[uiDrawTextStretchUltraExpanded] = 0.400000,
};

NSFontDescriptor *fontdescToNSFontDescriptor(uiDrawFontDescriptor *fd)
{
	NSMutableDictionary *attrs;
	NSFontDescriptor *basedesc;

	attrs = [NSMutableDictionary new];
	[attrs setObject:[NSString stringWithUTF8String:fd->Family]
		forKey:NSFontFamilyAttribute];
	[attrs setObject:[NSNumber numberWithDouble:fd->Size]
		forKey:NSFontSizeAttribute];

	basedesc = [[NSFontDescriptor alloc] initWithFontAttributes:attrs];
	[attrs release];
	return matchTraits(basedesc,
		weightToCTWeight(fd->Weight),
		fd->Italic,
		stretchesToCTWidths[fd->Stretch]);
}
