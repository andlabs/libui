// 25 may 2016
#include "uipriv_unix.h"
#include "attrstr.h"

int uiprivGraphemesTakesUTF16(void)
{
	return 0;
}

uiprivGraphemes *uiprivNewGraphemes(void *s, size_t len)
{
	uiprivGraphemes *g;
	char *text = (char *) s;
	size_t lenchars;
	PangoLogAttr *logattrs;
	size_t i;
	size_t *op;

	g = uiprivNew(uiprivGraphemes);

	// TODO see if we can use the utf routines
	lenchars = g_utf8_strlen(text, -1);
	logattrs = (PangoLogAttr *) uiprivAlloc((lenchars + 1) * sizeof (PangoLogAttr), "PangoLogAttr[] (graphemes)");
	pango_get_log_attrs(text, len,
		-1, NULL,
		logattrs, lenchars + 1);

	// first figure out how many graphemes there are
	g->len = 0;
	for (i = 0; i < lenchars; i++)
		if (logattrs[i].is_cursor_position != 0)
			g->len++;

	g->pointsToGraphemes = (size_t *) uiprivAlloc((len + 1) * sizeof (size_t), "size_t[] (graphemes)");
	g->graphemesToPoints = (size_t *) uiprivAlloc((g->len + 1) * sizeof (size_t), "size_t[] (graphemes)");

	// compute the graphemesToPoints array
	// TODO merge with the next for loop somehow?
	op = g->graphemesToPoints;
	for (i = 0; i < lenchars; i++)
		if (logattrs[i].is_cursor_position != 0)
			// TODO optimize this
			*op++ = g_utf8_offset_to_pointer(text, i) - text;
	// and do the last one
	*op++ = len;

	// and finally build the pointsToGraphemes array
	op = g->pointsToGraphemes;
	for (i = 0; i < g->len; i++) {
		size_t j;
		size_t first, last;

		first = g->graphemesToPoints[i];
		last = g->graphemesToPoints[i + 1];
		for (j = first; j < last; j++)
			*op++ = i;
	}
	// and do the last one
	*op++ = i;

	uiprivFree(logattrs);
	return g;
}
