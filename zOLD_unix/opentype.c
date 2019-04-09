// 11 may 2017
#include "uipriv_unix.h"
#include "attrstr.h"

// see https://developer.mozilla.org/en/docs/Web/CSS/font-feature-settings
static uiForEach toCSS(const uiOpenTypeFeatures *otf, char a, char b, char c, char d, uint32_t value, void *data)
{
	GString *s = (GString *) data;

	// the last trailing comma is removed after foreach is done
	g_string_append_printf(s, "\"%c%c%c%c\" %" PRIu32 ", ",
		a, b, c, d, value);
	return uiForEachContinue;
}

GString *uiprivOpenTypeFeaturesToPangoCSSFeaturesString(const uiOpenTypeFeatures *otf)
{
	GString *s;

	s = g_string_new("");
	uiOpenTypeFeaturesForEach(otf, toCSS, s);
	if (s->len != 0)
		// and remove the last comma
		g_string_truncate(s, s->len - 2);
	return s;
}
