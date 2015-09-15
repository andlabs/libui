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

struct uniitem {
	SCRIPT_ITEM item;

	int nGlyphs;
	WORD *glyphs;
	SCRIPT_VISATTR *visattrs;
	int *advances;
	GOFFSET *goffsets;

	WCHAR *pwcChars;
	int nChars;
	WORD *clusters;

	ABC abc;
};

struct uniscribe {
	WCHAR *text;
	size_t len;
	HDC dc;

	SCRIPT_CONTROL control;
	SCRIPT_STATE state;

	struct uniitem *items;
	int nItems;

	SCRIPT_CACHE cache;
};

// the step numbers in this function are according to the "Lay Out Text Using Uniscribe" section of https://msdn.microsoft.com/en-us/library/windows/desktop/dd317792%28v=vs.85%29.aspx

// this does step 4
// steps 1 through 3 are irrelevent here
static void initUniscribe(struct uniscribe *s, WCHAR *text, HDC dc)
{
	SCRIPT_ITEM *items;
	int nAlloc;
	int i;
	HRESULT hr;

	ZeroMemory(s, sizeof (struct uniscribe));

	s->text = text;
	s->len = wcslen(s->text);
	s->dc = dc;

	items = NULL;
	nAlloc = 32;
	for (;;) {
		// TODO use uiRealloc
		items = realloc(items, nAlloc * sizeof (SCRIPT_ITEM));
		hr = ScriptItemize(s->text, s->len,
			nAlloc,
			&(s->control), &(s->state),
			items, &(s->nItems));
		if (hr == S_OK)
			break;
		if (hr == E_OUTOFMEMORY) {
			nAlloc *= 2;
			continue;
		}
		logHRESULT("error itemizing string in initUniscribe()", hr);
	}

	// this does part of step 6
	s->items = malloc(s->nItems * sizeof (struct uniitem));
ZeroMemory(s->items, s->nItems * sizeof (struct uniitem));//TODO remove when switching to uiAlloc
	for (i = 0; i < s->nItems; i++) {
		s->items[i].pwcChars = s->text + items[i].iCharPos;
		s->items[i].nChars = items[i + 1].iCharPos - items[i].iCharPos;
		s->items[i].item = items[i];
	}
	free(items);
}

static void resetItem(struct uniitem *item)
{
	if (item->nGlyphs == 0)
		return;
	item->nGlyphs = 0;
	// TODO switch to uiFree
	free(item->glyphs);
	item->glyphs = NULL;
	free(item->visattrs);
	item->visattrs = NULL;
	free(item->advances);
	item->advances = NULL;
	free(item->goffsets);
	item->goffsets = NULL;
	free(item->clusters);
	item->clusters = NULL;
}

// step 5
// we're not using multiple fonts; we have no lists to merge

// step 6
static void shapeItem(struct uniscribe *s, int n)
{
	struct uniitem *item;
	int nAlloc;
	HRESULT hr;

	item = &(s->items[n]);
	resetItem(item);
	item->clusters = malloc(nChars * sizeof (WORD));

	// TODO move to uiAlloc
	nAlloc = 1.5 * item->nChars + 16;
	for (;;) {
		item->glyphs = realloc(item->glyphs, nAlloc * sizeof (WORD));
		item->visattrs = realloc(item->visattrs, nAlloc * sizeof (SCRIPT_VISATTR));
		hr = ScriptShape(s->dc, &(s->cache),
			item->pwcChars, item->nChars,
			nAlloc,
			&(item->item.a),
			item->glyphs, item->clusters,
			item->visattrs, &(item->nGlyphs));
		if (hr == S_OK)
			break;
		if (hr == E_OUTOFMEMORY) {
			nAlloc *= 2;
			continue;
		}
		logHRESULT("error shaping glyphs in shapeItem()", hr);
	}
}

// step 7
// TODO

// step 8
static void placeItem(struct uniscribe *s, int n)
{
	struct uniitem *item;
	HRESULT hr;

	item = &(s->items[n]);

	item->advances = malloc(item->nGlyphs * sizeof (int));
	item->goffsets = malloc(item->nGlyphs * sizeof (GOFFSET));
	hr = ScriptPlace(s->dc, &(s->cache),
		item->glyphs, item->nGlyphs, item->visattrs,
		&(item->item.a),
		item->advances, item->goffsets, &(item->abc));
	if (hr != S_OK)
		logHRESULT("error placing glyphs in placeItem()", hr);
}

static void uninitUniscribe(struct uniscribe *s)
{
	HRESULT hr;
	int i;

	// TODO use uiFree

	for (i = 0; i < s->nItems; i++)
		resetItem(&(s->items[i]));
	free(s->items);

	hr = ScriptCacheFree(&(s->cache));
	if (hr != S_OK)
		logHRESULT("error freeing Uniscribe cache in uninitUniscribe()");
}
