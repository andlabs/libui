// 25 may 2016
#include "uipriv_unix.H"

// TODO this breaks for non-BMP characters?

struct gparam {
	const char *text;
	PangoGlyphString *glyphString;
};

static void runItem(gpointer it, gpointer data)
{
	PangoItem *item = (PangoItem *) it;
	struct gparam *gparam = (struct gparam *) data;

	pango_shape(gparam->text + item->offset,
		item->length,
		&(item->analysis),
		gparam->glyphString);
	pango_item_free(item);
}

PangoGlyphString *graphemes(const char *text, PangoContext *context)
{
	size_t len;
	GList *list;
	struct gparam gparam;

	len = strlen(text);
	list = pango_itemize(context,
		text, 0, len,
		NULL, NULL);

	gparam.text = text;
	gparam.glyphString = pango_glyph_string_new();

	g_list_foreach(list, runItem, &gparam);

	g_list_free(list);
	return gparam.glyphString;
}
