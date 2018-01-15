// 25 may 2016
#include "uipriv_unix.h"

int graphemesTakesUTF16(void)
{
	return 0;
}

struct graphemes *graphemes(void *s, size_t len)
{
	struct graphemes *g;
	char *text = (char *) s;
	size_t lenchars;
	PangoLogAttr *logattrs;
	size_t i;
	size_t *op;

	g = uiNew(struct graphemes);

	// TODO see if we can use the utf routines
	lenchars = g_utf8_strlen(text, -1);
	logattrs = (PangoLogAttr *) uiAlloc((lenchars + 1) * sizeof (PangoLogAttr), "PangoLogAttr[] (graphemes)");
	pango_get_log_attrs(text, len,
		-1, NULL,
		logattrs, lenchars + 1);

	// first figure out how many graphemes there are
	g->len = 0;
	for (i = 0; i < lenchars; i++)
		if (logattrs[i].is_cursor_position != 0)
			g->len++;

	g->pointsToGraphemes = (size_t *) uiAlloc((len + 1) * sizeof (size_t), "size_t[] (graphemes)");
	g->graphemesToPoints = (size_t *) uiAlloc((g->len + 1) * sizeof (size_t), "size_t[] (graphemes)");

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

	uiFree(logattrs);
	return g;
}
