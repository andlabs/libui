// 25 may 2016
#include "uipriv_windows.hpp"
#include "attrstr.hpp"

// We could use CharNextW() to generate grapheme cluster boundaries, but it doesn't handle surrogate pairs properly (see http://archives.miloush.net/michkap/archive/2008/12/16/9223301.html).
// We could also use Uniscribe (see http://archives.miloush.net/michkap/archive/2005/01/14/352802.html, http://www.catch22.net/tuts/uniscribe-mysteries, http://www.catch22.net/tuts/keyboard-navigation, and https://maxradi.us/documents/uniscribe/), but its rules for buffer sizes is convoluted.
// Let's just deal with the CharNextW() bug.

int uiprivGraphemesTakesUTF16(void)
{
	return 1;
}

uiprivGraphemes *uiprivNewGraphemes(void *s, size_t len)
{
	uiprivGraphemes *g;
	WCHAR *str;
	size_t *pPTG, *pGTP;

	g = uiprivNew(uiprivGraphemes);

	g->len = 0;
	str = (WCHAR *) s;
	while (*str != L'\0') {
		g->len++;
		str = CharNextW(str);
		// no need to worry about surrogates if we're just counting
	}

	g->pointsToGraphemes = (size_t *) uiprivAlloc((len + 1) * sizeof (size_t), "size_t[] (graphemes)");
	g->graphemesToPoints = (size_t *) uiprivAlloc((g->len + 1) * sizeof (size_t), "size_t[] (graphemes)");

	pPTG = g->pointsToGraphemes;
	pGTP = g->graphemesToPoints;
	str = (WCHAR *) s;
	while (*str != L'\0') {
		WCHAR *next, *p;
		ptrdiff_t nextoff;

		// as part of the bug, we need to make sure we only call CharNextW() on low halves, otherwise it'll return the same low half forever
		nextoff = 0;
		if (IS_HIGH_SURROGATE(*str))
			nextoff = 1;
		next = CharNextW(str + nextoff);
		if (IS_LOW_SURROGATE(*next))
			next--;

		*pGTP = pPTG - g->pointsToGraphemes;
		for (p = str; p < next; p++)
			*pPTG++ = pGTP - g->graphemesToPoints;
		pGTP++;

		str = next;
	}
	// and handle the last item for the end of the string
	*pGTP = pPTG - g->pointsToGraphemes;
	*pPTG = pGTP - g->graphemesToPoints;

	return g;
}
