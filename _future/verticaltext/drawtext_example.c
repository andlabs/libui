	next = "vertical glyph forms";
	start = uiAttributedStringLen(attrstr);
	end = start + strlen(next);
	uiAttributedStringAppendUnattributed(attrstr, next);
	spec.Type = uiAttributeVerticalForms;
	spec.Value = 1;
	uiAttributedStringSetAttribute(attrstr, &spec, start, end);
	uiAttributedStringAppendUnattributed(attrstr, " (which you can draw rotated for proper vertical text; for instance, ");
	next = "\xE3\x81\x82\xE3\x81\x84\xE3\x81\x86\xE3\x81\x88\xE3\x81\x8A";
	start = uiAttributedStringLen(attrstr);
	end = start + strlen(next);
	uiAttributedStringAppendUnattributed(attrstr, next);
	spec.Type = uiAttributeVerticalForms;
	spec.Value = 1;
	uiAttributedStringSetAttribute(attrstr, &spec, start, end);
	uiAttributedStringAppendUnattributed(attrstr, ")");

	uiAttributedStringAppendUnattributed(attrstr, ", ");
