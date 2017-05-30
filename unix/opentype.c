// 11 may 2017
#include "uipriv_unix.h"

// TODO switch from GINT_TO_POINTER() and so to a fake GUINT_TO_POINTER()?

struct uiOpenTypeFeatures {
	GHashTable *tags;
};

uiOpenTypeFeatures *uiNewOpenTypeFeatures(void)
{
	uiOpenTypeFeatures *otf;

	otf = uiNew(uiOpenTypeFeatures);
	otf->tags = g_hash_table_new(g_direct_hash, g_direct_equal);
	return otf;
}

void uiFreeOpenTypeFeatures(uiOpenTypeFeatures *otf)
{
	g_hash_table_destroy(otf->tags);
	uiFree(otf);
}

static void cloneTags(gpointer key, gpointer value, gpointer data)
{
	// TODO is there a G_HASH_TABLE()?
	g_hash_table_replace((GHashTable *) data, key, value);
}

uiOpenTypeFeatures *uiOpenTypeFeaturesClone(const uiOpenTypeFeatures *otf)
{
	uiOpenTypeFeatures *out;

	// TODO switch the windows one to use this
	out = uiNewOpenTypeFeatures();
	g_hash_table_foreach(otf->tags, cloneTags, out->tags);
	return out;
}

static gpointer mkTag(char a, char b, char c, char d)
{
	uint32_t tag;

	tag = (((uint32_t) a) & 0xFF) << 24;
	tag |= (((uint32_t) b) & 0xFF) << 16;
	tag |= (((uint32_t) c) & 0xFF) << 8;
	tag |= ((uint32_t) d) & 0xFF;
	return GINT_TO_POINTER(tag);
}

void uiOpenTypeFeaturesAdd(uiOpenTypeFeatures *otf, char a, char b, char c, char d, uint32_t value)
{
	g_hash_table_replace(otf->tags, mkTag(a, b, c, d), GINT_TO_POINTER(value));
}

void uiOpenTypeFeaturesRemove(uiOpenTypeFeatures *otf, char a, char b, char c, char d)
{
	g_hash_table_remove(otf->tags, mkTag(a, b, c, d));
}

// TODO should this be before Add and Remove?
// TODO better name than Get?
int uiOpenTypeFeaturesGet(uiOpenTypeFeatures *otf, char a, char b, char c, char d, uint32_t *value)
{
	gboolean found;
	gpointer gv;

	found = g_hash_table_lookup_extended(otf->tags,
		mkTag(a, b, c, d),
		NULL, &gv);
	if (!found)
		return 0;
	*value = GPOINTER_TO_INT(gv);
	return 1;
}

struct otfForEach {
	uiOpenTypeFeaturesForEachFunc f;
	void *data;
	// TODO store continuation status here
};

static void foreach(gpointer key, gpointer value, gpointer data)
{
	struct otfForEach *ofe = (struct otfForEach *) data;
	uint32_t tag;
	uint8_t a, b, c, d;

	tag = GPOINTER_TO_INT(key);
	a = (uint8_t) ((tag >> 24) & 0xFF);
	b = (uint8_t) ((tag >> 16) & 0xFF);
	c = (uint8_t) ((tag >> 8) & 0xFF);
	d = (uint8_t) (tag & 0xFF);
	// TODO handle return value
	(*(ofe->f))((char) a, (char) b, (char) c, (char) d, GPOINTER_TO_INT(value), ofe->data);
}

void uiOpenTypeFeaturesForEach(const uiOpenTypeFeatures *otf, uiOpenTypeFeaturesForEachFunc f, void *data)
{
	struct otfForEach ofe;

	memset(&ofe, 0, sizeof (struct otfForEach));
	ofe.f = f;
	ofe.data = data;
	g_hash_table_foreach(otf->tags, foreach, &ofe);
}

static gint tagcmp(gconstpointer a, gconstpointer b)
{
	return GPOINTER_TO_INT(a) - GPOINTER_TO_INT(b);
}

static GList *copySortedKeys(GHashTable *tags)
{
	GList *k, *copy;

	k = g_hash_table_get_keys(tags);
	copy = g_list_copy(k);
	copy = g_list_sort(copy, tagcmp);
	// TODO do we free k? the docs contradict themselves
	// TODO I already forgot, does g_list_sort() copy, or just change the head?
	return copy;
}

int uiOpenTypeFeaturesEqual(const uiOpenTypeFeatures *a, const uiOpenTypeFeatures *b)
{
	GList *ak, *bk;
	GList *ai, *bi;
	guint na, nb;
	guint i;
	int equal = 0;

	ak = copySortedKeys(a->tags);
	bk = copySortedKeys(b->tags);

	na = g_list_length(ak);
	nb = g_list_length(bk);
	if (na != nb) {
		equal = 0;
		goto out;
	}

	// TODO use GPOINTER_TO_INT() in these?
	ai = ak;
	bi = bk;
	for (i = 0; i < na; i++) {
		gpointer av, bv;

		// compare keys
		// this is why we needed to sort earlier
		if (ai->data != bi->data) {
			equal = 0;
			goto out;
		}
		// and compare values
		av = g_hash_table_lookup(a->tags, ai->data);
		bv = g_hash_table_lookup(b->tags, bi->data);
		if (av != bv) {
			equal = 0;
			goto out;
		}
		ai = ai->next;
		bi = bi->next;
	}

	// all good
	equal = 1;

out:
	g_list_free(bk);
	g_list_free(ak);
	return equal;
}

// see https://developer.mozilla.org/en/docs/Web/CSS/font-feature-settings
// TODO make this a g_hash_table_foreach() function (which requires duplicating code)?
static int toCSS(char a, char b, char c, char d, uint32_t value, void *data)
{
	// TODO is there a G_STRING()?
	GString *s = (GString *) data;

	// the last trailing comma is removed after foreach is done
	g_string_append_printf(s, "\"%c%c%c%c\" %" PRIu32 ", ",
		a, b, c, d, value);
	// TODO use this
	return 0;
}

GString *otfToPangoCSSString(uiOpenTypeFeatures *otf)
{
	GString *s;

	s = g_string_new("");
	uiOpenTypeFeaturesForEach(otf, toCSS, s);
	if (s->len != 0)
		// and remove the last comma
		g_string_truncate(s, s->len - 2);
	return s;
}
