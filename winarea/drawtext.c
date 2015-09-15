// 14 september 2015
#include "area.h"

struct uiDrawFont {
	HFONT font;
};

uiDrawFont *uiDrawPrepareFont(uiDrawFontSpec *spec)
{
	uiDrawFont *df;
	LOGFONTW lf;

	// TODO use uiNew
	df = malloc(sizeof (uiDrawFont));
	ZeroMemory(&lf, sizeof (LOGFONTW));
	// TODO
	df->font = xxxxxxxx(&lf);
	if (df->font == NULL)
		logLastError("error creating font in uiDrawPrepareFont()");
	return df;
}

void uiDrawFreeFont(uiDrawFont *df)
{
	if (DeleteObject(df->font) == 0)
		logLastError("error deleting font in uiDrawFreeFont()");
	// TODO use uiFree
	free(df);
}

/* uniscribe notes:
- https://msdn.microsoft.com/en-us/library/windows/desktop/dd317792%28v=vs.85%29.aspx
- https://maxradi.us/documents/uniscribe/
- http://www.catch22.net/tuts/introduction-uniscribe
- http://www.catch22.net/tuts/uniscribe-mysteries
- http://www.catch22.net/tuts/more-uniscribe-mysteries
- http://www.catch22.net/tuts/drawing-styled-text-uniscribe
- http://www.microsoft.com/msj/1198/multilang/multilang.aspx
Even with all this we're on our own with actual paragraph layout; see the last link for example code (which we don't use here). */

struct uniscribe {
	WCHAR *text;
	size_t len;
	HDC dc;

	SCRIPT_CONTROL control;
	SCRIPT_STATE state;

	SCRIPT_ITEM *items;
	int nItems;

	SCRIPT_CACHE cache;
	WORD **glyphs;
	WORD **clusters;
	SCRIPT_VISATTR **visattrs;
	int *nGlyphs;

	int **advances;
	GOFFSET **gOffsets;
	ABC *abcs;
};

// the step numbers in this function are according to the "Lay Out Text Using Uniscribe" section of https://msdn.microsoft.com/en-us/library/windows/desktop/dd317792%28v=vs.85%29.aspx
void initUniscribe(struct uniscribe *s, WCHAR *text, HDC dc)
{
	HRESULT hr;
	int nAlloc;
	int i;

	ZeroMemory(s, sizeof (struct uniscribe));

	s->text = text;
	s->len = wcslen(s->text);
	s->dc = dc;

	// steps 1 through 3 are irrelevent here

	// step 4
	nAlloc = 32;
	for (;;) {
		// TODO use uiRealloc
		s->items = realloc(s->items, nAlloc * sizeof (SCRIPT_ITEM));
		hr = ScriptItemize(s->text, s->len,
			nAlloc,
			&(s->control), &(s->state),
			s->items, &(s->nItems));
		if (hr == S_OK)
			break;
		if (hr == E_OUTOFMEMORY) {
			nAlloc *= 2;
			continue;
		}
		logHRESULT("error itemizing string in initUniscribe()", hr);
	}

	// step 5
	// we're not using multiple fonts; we have no lists to merge

	// step 6
	s->glyphs = malloc(s->nItems * sizeof (WORD *));
	s->clusters = malloc(s->nItems * sizeof (WORD *));
	s->visattrs = malloc(s->nItems * sizeof (SCRIPT_VISATTR *));
	s->nGlyphs = malloc(s->nItems * sizeof (int));
	for (i = 0; i < s->nItems; i++) {
		WCHAR *pwcChars;
		int nChars;

		pwcChars = s->text + s->items[i].iCharPos;
		nChars = s->items[i + 1].iCharPos - s->items[i].iCharPos;
		nAlloc = 1.5 * nChars + 16;
s->glyphs[i] = NULL;//TODO remove when switched to uiAlloc
s->visattrs[i] = NULL;
		s->clusters[i] = malloc(nChars * sizeof (WORD));
		for (;;) {
			s->glyphs[i] = realloc(s->glyphs[i], nAlloc * sizeof (WORD));
			s->visattrs[i] = realloc(s->visattrs[i], nAlloc * sizeof (SCRIPT_VISATTR));
			hr = ScriptShape(s->dc, &(s->cache),
				pwcChars, nChars,
				nAlloc,
				&(s->items[i].a),
				s->glyphs[i], s->clusters[i],
				s->visattrs[i], &(s->nGlyphs[i]));
			if (hr == S_OK)
				break;
			if (hr == E_OUTOFMEMORY) {
				nAlloc *= 2;
				continue;
			}
			logHRESULT("error shaping glyphs in initUniscribe()", hr);
		}
	}

	// step 7
	// TODO

	// step 8
	s->advances = malloc(s->nItems * sizeof (int *));
	s->goffsets = malloc(s->nItems * sizeof (GOFFSET *));
	s->abcs = malloc(s->nItems * sizeof (ABC));
	for (i = 0; i < s->nItems; i++) {
		s->advances[i] = malloc(s->nGlyphs[i] * sizeof (int));
		s->goffsets[i] = malloc(s->nGlyphs[i] * sizeof (GOFFSET));
		hr = ScriptPlace(s->dc, &(s->cache),
			s->glyphs[i], s->nGlyphs[i], s->visattrs[i],
			&(s->items[i].a),
			s->advances[i], s->goffsets[i], &(s->abcs[i]));
		if (hr != S_OK)
			logHRESULT("error placing glyphs in initUniscribe()", hr);
	}
}

void uninitUniscribe(struct uniscribe *s)
{
	HRESULT hr;
	int i;

	// TODO use uiFree

	free(s->abcs);

	for (i = 0; i < s->goffsets; i++)
		free(s->goffsets[i]);
	free(s->goffsets);

	for (i = 0; i < s->nItems; i++)
		free(s->advances[i]);
	free(s->advances);

	free(s->nGlyphs);

	for (i = 0; i < s->nItems; i++)
		free(s->visattrs[i]);
	free(s->visattrs);

	for (i = 0; i < s->nItems; i++)
		free(s->clusters[i]);
	free(s->clusters);

	for (i = 0; i < s->nItems; i++)
		free(s->glyphs[i]);
	free(s->glyphs);

	hr = ScriptCacheFree(&(s->cache));
	if (hr != S_OK)
		logHRESULT("error freeing Uniscribe cache in uninitUniscribe()");

	free(s->items);
}
