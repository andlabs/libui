// 3 december 2016
#import "uipriv_darwin.h"
#import "attrstr.h"

// CFStringGetRangeOfComposedCharactersAtIndex() is the function for grapheme clusters
// https://developer.apple.com/library/mac/documentation/Cocoa/Conceptual/Strings/Articles/stringsClusters.html says that this does work on all multi-codepoint graphemes (despite the name), and that this is the preferred function for this particular job anyway

int uiprivGraphemesTakesUTF16(void)
{
	return 1;
}

uiprivGraphemes *uiprivNewGraphemes(void *s, size_t len)
{
	uiprivGraphemes *g;
	UniChar *str = (UniChar *) s;
	CFStringRef cfstr;
	size_t ppos, gpos;
	CFRange range;
	size_t i;

	g = uiprivNew(uiprivGraphemes);

	cfstr = CFStringCreateWithCharactersNoCopy(NULL, str, len, kCFAllocatorNull);
	if (cfstr == NULL) {
		// TODO
	}

	// first figure out how many graphemes there are
	g->len = 0;
	ppos = 0;
	while (ppos < len) {
		range = CFStringGetRangeOfComposedCharactersAtIndex(cfstr, ppos);
		g->len++;
		ppos = range.location + range.length;
	}

	g->pointsToGraphemes = (size_t *) uiprivAlloc((len + 1) * sizeof (size_t), "size_t[] (graphemes)");
	g->graphemesToPoints = (size_t *) uiprivAlloc((g->len + 1) * sizeof (size_t), "size_t[] (graphemes)");

	// now calculate everything
	// fortunately due to the use of CFRange we can do this in one loop trivially!
	ppos = 0;
	gpos = 0;
	while (ppos < len) {
		range = CFStringGetRangeOfComposedCharactersAtIndex(cfstr, ppos);
		for (i = 0; i < range.length; i++)
			g->pointsToGraphemes[range.location + i] = gpos;
		g->graphemesToPoints[gpos] = range.location;
		gpos++;
		ppos = range.location + range.length;
	}
	// and set the last one
	g->pointsToGraphemes[ppos] = gpos;
	g->graphemesToPoints[gpos] = ppos;

	CFRelease(cfstr);
	return g;
}
