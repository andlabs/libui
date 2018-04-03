	PangoLanguage *lang;

	// language strings are specified as BCP 47: https://developer.gnome.org/pango/1.30/pango-Scripts-and-Languages.html#pango-language-from-string https://www.ietf.org/rfc/rfc3066.txt
	case uiAttributeLanguage:
		lang = pango_language_from_string((const char *) (spec->Value));
		addattr(p, start, end,
			pango_attr_language_new(lang));
		// lang *cannot* be freed
		break;
