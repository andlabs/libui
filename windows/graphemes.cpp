// 25 may 2016
#include "uipriv_windows.hpp"

// We could use CharNext() to generate grapheme cluster boundaries, but it doesn't handle surrogate pairs properly (see http://archives.miloush.net/michkap/archive/2008/12/16/9223301.html).
// So let's use Uniscribe (see http://archives.miloush.net/michkap/archive/2005/01/14/352802.html)
// See also http://www.catch22.net/tuts/uniscribe-mysteries, http://www.catch22.net/tuts/keyboard-navigation, and https://maxradi.us/documents/uniscribe/ for more details.

// TODO the DirectWrite equivalent appears to be https://msdn.microsoft.com/en-us/library/windows/desktop/dd316625(v=vs.85).aspx but is somehow somewhat more complicated to use than Uniscribe is! maybe the PadWrite sample uses it? or should we just keep using Uniscribe?

int graphemesTakesUTF16(void)
{
	return 1;
}

static HRESULT itemize(WCHAR *s, size_t len, SCRIPT_ITEM **out, int *outn)
{
	SCRIPT_CONTROL sc;
	SCRIPT_STATE ss;
	SCRIPT_ITEM *items;
	size_t maxItems;
	int n;
	HRESULT hr;

	// make sure these are zero-initialized to avoid mangling the text
	ZeroMemory(&sc, sizeof (SCRIPT_CONTROL));
	ZeroMemory(&ss, sizeof (SCRIPT_STATE));

	maxItems = len + 2;
	for (;;) {
		items = new SCRIPT_ITEM[maxItems + 1];
		hr = ScriptItemize(s, len,
			maxItems,
			&sc, &ss,
			items, &n);
		if (hr == S_OK)
			break;
		// otherwise either an error or not enough room
		delete[] items;
		if (hr != E_OUTOFMEMORY)
			return hr;
		maxItems *= 2;		// add some more and try again
	}

	*out = items;
	*outn = n;
	return S_OK;
}

struct graphemes *graphemes(void *s, size_t len)
{
	struct graphemes *g;
	WCHAR *str = (WCHAR *) s;
	SCRIPT_ITEM *items;
	int nItems;
	int curItemIndex;
	int nCharsInCurItem;
	size_t *pPTG, *pGTP;
	HRESULT hr;

	g = uiNew(struct graphemes);

	hr = itemize(str, len, &items, &nItems);
	if (hr != S_OK)
		logHRESULT(L"error itemizing string for finding grapheme cluster boundaries", hr);
	g->len = nItems;
	g->pointsToGraphemes = (size_t *) uiAlloc((len + 1) * sizeof (size_t), "size_t[] (graphemes)");
	// note that there are actually nItems + 1 elements in items
	// items[nItems] is the grapheme one past the end
	g->graphemesToPoints = (size_t *) uiAlloc((g->len + 1) * sizeof (size_t), "size_t[] (graphemes)");

	pPTG = g->pointsToGraphemes;
	pGTP = g->graphemesToPoints;
	for (curItemIndex = 0; curItemIndex < nItems; curItemIndex++) {
		SCRIPT_ITEM *curItem, *nextItem;
		SCRIPT_LOGATTR *logattr;
		size_t *curGTP;
		int i;

		curItem = items + curItemIndex;
		nextItem = curItem + 1;

		nCharsInCurItem = nextItem->iCharPos - curItem->iCharPos;

		logattr = new SCRIPT_LOGATTR[nCharsInCurItem];
		hr = ScriptBreak(str + curItem->iCharPos, nCharsInCurItem,
			&(curItem->a), logattr);
		if (hr != S_OK)
			logHRESULT(L"error breaking string for finding grapheme cluster boundaries", hr);

		// TODO can we merge these loops somehow?
		curGTP = pGTP;
		for (i = 0; i < nCharsInCurItem; i++)
			if (logattr[i].fCharStop != 0)
				*pGTP++ = curItem->iCharPos + i;
		for (i = 0; i < nCharsInCurItem; i++) {
			*pPTG++ = curGTP - g->graphemesToPoints;
			if (logattr[i].fCharStop != 0)
				curGTP++;
		}

		delete[] logattr;
	}
	// and handle the last item for the end of the string
	*pGTP++ = items[nItems].iCharPos;
	*pPTG++ = pGTP - g->graphemesToPoints;

	delete[] items;
	return g;
}
