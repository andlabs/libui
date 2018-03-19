before "or any combination of the above"

	// thanks to https://twitter.com/codeman38/status/831924064012886017
	next = "\xD0\xB1\xD0\xB3\xD0\xB4\xD0\xBF\xD1\x82";
	uiAttributedStringAppendUnattributed(attrstr, "multiple languages (compare ");
	start = uiAttributedStringLen(attrstr);
	end = start + strlen(next);
	uiAttributedStringAppendUnattributed(attrstr, next);
	spec.Type = uiAttributeItalic;
	spec.Value = uiDrawTextItalicItalic;
	uiAttributedStringSetAttribute(attrstr, &spec, start, end);
	spec.Type = uiAttributeLanguage;
	spec.Value = (uintptr_t) "ru";
	uiAttributedStringSetAttribute(attrstr, &spec, start, end);
	uiAttributedStringAppendUnattributed(attrstr, " to ");
	start = uiAttributedStringLen(attrstr);
	end = start + strlen(next);
	uiAttributedStringAppendUnattributed(attrstr, next);
	spec.Type = uiAttributeItalic;
	spec.Value = uiDrawTextItalicItalic;
	uiAttributedStringSetAttribute(attrstr, &spec, start, end);
	spec.Type = uiAttributeLanguage;
	spec.Value = (uintptr_t) "sr";
	uiAttributedStringSetAttribute(attrstr, &spec, start, end);
	uiAttributedStringAppendUnattributed(attrstr, " \xE2\x80\x94 may require changing the font)");

	uiAttributedStringAppendUnattributed(attrstr, ", ");
