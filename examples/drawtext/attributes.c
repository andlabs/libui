// 11 february 2017
#include "drawtext.h"

static uiAttributedString *attrstr;

static void setupAttributedString(void)
{
	uiAttributeSpec spec;
	size_t start, end;
	const char *next;

	attrstr = uiNewAttributedString("uiAttributedString isn't just for plain text! It supports ");

	next = "multiple fonts";
	start = uiAttributedStringLen(attrstr);
	end = start + strlen(next);
	uiAttributedStringAppendUnattributed(attrstr, next);
	spec.Type = uiAttributeFamily;
	spec.Value = (uintptr_t) "Courier New";
	uiAttributedStringSetAttribute(attrstr, &spec, start, end);

	uiAttributedStringAppendUnattributed(attrstr, ", ");

	next = "multiple sizes";
	start = uiAttributedStringLen(attrstr);
	end = start + strlen(next);
	uiAttributedStringAppendUnattributed(attrstr, next);
	spec.Type = uiAttributeSize;
	spec.Double = 18;
	uiAttributedStringSetAttribute(attrstr,
		&spec, start, end);

	uiAttributedStringAppendUnattributed(attrstr, ", ");

	next = "multiple weights";
	start = uiAttributedStringLen(attrstr);
	end = start + strlen(next);
	uiAttributedStringAppendUnattributed(attrstr, next);
	spec.Type = uiAttributeWeight;
	spec.Value = (uintptr_t) uiDrawTextWeightBold;
	uiAttributedStringSetAttribute(attrstr, &spec, start, end);

	uiAttributedStringAppendUnattributed(attrstr, ", ");

	next = "multiple italics";
	start = uiAttributedStringLen(attrstr);
	end = start + strlen(next);
	uiAttributedStringAppendUnattributed(attrstr, next);
	spec.Type = uiAttributeItalic;
	spec.Value = (uintptr_t) uiDrawTextItalicItalic;
	uiAttributedStringSetAttribute(attrstr, &spec, start, end);

	uiAttributedStringAppendUnattributed(attrstr, ", ");

	next = "multiple stretches";
	start = uiAttributedStringLen(attrstr);
	end = start + strlen(next);
	uiAttributedStringAppendUnattributed(attrstr, next);
	spec.Type = uiAttributeStretch;
	spec.Value = (uintptr_t) uiDrawTextStretchCondensed;
	uiAttributedStringSetAttribute(attrstr, &spec, start, end);

	uiAttributedStringAppendUnattributed(attrstr, ", ");

	next = "multiple colors";
	start = uiAttributedStringLen(attrstr);
	end = start + strlen(next);
	uiAttributedStringAppendUnattributed(attrstr, next);
	spec.Type = uiAttributeColor;
	// Direct2D "Crimson" (#DC143C)
	spec.R = 0.8627450980392156;
	spec.G = 0.0784313725490196;
	spec.B = 0.2352941176470588;
	spec.A = 0.75;
	uiAttributedStringSetAttribute(attrstr, &spec, start, end);

	uiAttributedStringAppendUnattributed(attrstr, ", ");

	next = "multiple backgrounds";
	start = uiAttributedStringLen(attrstr);
	end = start + strlen(next);
	uiAttributedStringAppendUnattributed(attrstr, next);
	spec.Type = uiAttributeBackground;
	// Direct2D "Peach Puff" (#FFDAB9)
	// TODO choose a darker color
	spec.R = 1.0;
	spec.G = 0.85490196078431372;
	spec.B = 0.7254901960784313;
	spec.A = 0.5;
	uiAttributedStringSetAttribute(attrstr, &spec, start, end);

	uiAttributedStringAppendUnattributed(attrstr, ", ");

	next = "vertical glyph forms";
	start = uiAttributedStringLen(attrstr);
	end = start + strlen(next);
	uiAttributedStringAppendUnattributed(attrstr, next);
	spec.Type = uiAttributeVerticalForms;
	spec.Value = 1;
	uiAttributedStringSetAttribute(attrstr, &spec, start, end);
	uiAttributedStringAppendUnattributed(attrstr, " (which you can draw rotated for proper vertical text)");

	uiAttributedStringAppendUnattributed(attrstr, ", ");

	next = "multiple";
	start = uiAttributedStringLen(attrstr);
	end = start + strlen(next);
	uiAttributedStringAppendUnattributed(attrstr, next);
	spec.Type = uiAttributeUnderline;
	spec.Value = uiDrawUnderlineStyleSingle;
	uiAttributedStringSetAttribute(attrstr, &spec, start, end);
	uiAttributedStringAppendUnattributed(attrstr, " ");
	next = "underlines";
	start = uiAttributedStringLen(attrstr);
	end = start + strlen(next);
	uiAttributedStringAppendUnattributed(attrstr, next);
	spec.Type = uiAttributeUnderline;
	spec.Value = uiDrawUnderlineStyleDouble;
	uiAttributedStringSetAttribute(attrstr, &spec, start, end);
	spec.Type = uiAttributeUnderlineColor;
	spec.Value = uiDrawUnderlineColorCustom;
	spec.R = 0.5;
	spec.G = 0.0;
	spec.B = 1.0;
	spec.A = 1.0;
	uiAttributedStringSetAttribute(attrstr, &spec, start, end);
	uiAttributedStringAppendUnattributed(attrstr, " (");
	next = "including underlines for spelling correction and the like";
	start = uiAttributedStringLen(attrstr);
	end = start + strlen(next);
	uiAttributedStringAppendUnattributed(attrstr, next);
	spec.Type = uiAttributeUnderline;
	spec.Value = uiDrawUnderlineStyleSuggestion;
	uiAttributedStringSetAttribute(attrstr, &spec, start, end);
	spec.Type = uiAttributeUnderlineColor;
	spec.Value = uiDrawUnderlineColorSpelling;
	uiAttributedStringSetAttribute(attrstr, &spec, start, end);
	uiAttributedStringAppendUnattributed(attrstr, ")");

	uiAttributedStringAppendUnattributed(attrstr, ", ");

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

	next = "or any combination of the above";
	// TODO

	uiAttributedStringAppendUnattributed(attrstr, ". In addition, a variety of typographical features are available (depending on the chosen font) that can be switched on (or off, if the font enables them by default): ");

	next = "fi";
	uiAttributedStringAppendUnattributed(attrstr, "standard ligatures like f+i (");
	start = uiAttributedStringLen(attrstr);
	end = start + strlen(next);
	uiAttributedStringAppendUnattributed(attrstr, next);
	spec.Type = uiAttributeStandardLigatures;
	spec.Value = 1;
	uiAttributedStringSetAttribute(attrstr, &spec, start, end);
	uiAttributedStringAppendUnattributed(attrstr, ")");

	uiAttributedStringAppendUnattributed(attrstr, ", ");

	// note the use of LTR marks and RTL embeds to make sure the bidi algorithm doesn't kick in for our demonstration (it will produce incorrect results)
	// see also: https://www.w3.org/International/articles/inline-bidi-markup/#nomarkup
	next = "\xD9\x84\xD8\xA7";
	uiAttributedStringAppendUnattributed(attrstr, "required ligatures like \xE2\x80\xAB\xD9\x84\xE2\x80\xAC+\xE2\x80\xAB\xD8\xA7\xE2\x80\xAC (\xE2\x80\x8E\xE2\x80\xAB");
	start = uiAttributedStringLen(attrstr);
	end = start + strlen(next);
	uiAttributedStringAppendUnattributed(attrstr, next);
	spec.Type = uiAttributeRequiredLigatures;
	spec.Value = 1;
	uiAttributedStringSetAttribute(attrstr, &spec, start, end);
	uiAttributedStringAppendUnattributed(attrstr, "\xE2\x80\xAC)");

	uiAttributedStringAppendUnattributed(attrstr, ", ");

	next = "ct";
	uiAttributedStringAppendUnattributed(attrstr, "discretionary/rare ligatures like c+t (");
	start = uiAttributedStringLen(attrstr);
	end = start + strlen(next);
	uiAttributedStringAppendUnattributed(attrstr, next);
	spec.Type = uiAttributeDiscretionaryLigatures;
	spec.Value = 1;
	uiAttributedStringSetAttribute(attrstr, &spec, start, end);
	uiAttributedStringAppendUnattributed(attrstr, ")");

	uiAttributedStringAppendUnattributed(attrstr, ", ");

	next = "the";
	uiAttributedStringAppendUnattributed(attrstr, "contextual ligatures like h+e in the (");
	start = uiAttributedStringLen(attrstr);
	end = start + strlen(next);
	uiAttributedStringAppendUnattributed(attrstr, next);
	spec.Type = uiAttributeContextualLigatures;
	spec.Value = 1;
	uiAttributedStringSetAttribute(attrstr, &spec, start, end);
	uiAttributedStringAppendUnattributed(attrstr, ")");

	uiAttributedStringAppendUnattributed(attrstr, ", ");

	next = "\xC3\x9F";
	uiAttributedStringAppendUnattributed(attrstr, "historical ligatures like the decomposition of \xC3\x9F (");
	start = uiAttributedStringLen(attrstr);
	end = start + strlen(next);
	uiAttributedStringAppendUnattributed(attrstr, next);
	spec.Type = uiAttributeHistoricalLigatures;
	spec.Value = 1;
	uiAttributedStringSetAttribute(attrstr, &spec, start, end);
	uiAttributedStringAppendUnattributed(attrstr, ")");

	uiAttributedStringAppendUnattributed(attrstr, ", ");

	next = "UnICasE wRITInG";
	start = uiAttributedStringLen(attrstr);
	end = start + strlen(next);
	uiAttributedStringAppendUnattributed(attrstr, next);
	spec.Type = uiAttributeUnicase;
	spec.Value = 1;
	uiAttributedStringSetAttribute(attrstr, &spec, start, end);

	uiAttributedStringAppendUnattributed(attrstr, ", ");

	next = "TODO";
}

static char fontFamily[] = "Times New Roman";
// TODO should be const; look at constructor function?
static uiDrawFontDescriptor defaultFont = {
	.Family = fontFamily,
	.Size = 12,
	.Weight = uiDrawTextWeightNormal,
	.Italic = uiDrawTextItalicNormal,
	.Stretch = uiDrawTextStretchNormal,
};
static uiDrawTextLayoutParams params;

#define margins 10

static uiBox *panel;
static uiCheckbox *showLineBounds;
static uiFontButton *fontButton;

// TODO should be const?
static uiDrawBrush fillBrushes[4] = {
	{
		.Type = uiDrawBrushTypeSolid,
		.R = 1.0,
		.G = 0.0,
		.B = 0.0,
		.A = 0.5,
	},
	{
		.Type = uiDrawBrushTypeSolid,
		.R = 0.0,
		.G = 1.0,
		.B = 0.0,
		.A = 0.5,
	},
	{
		.Type = uiDrawBrushTypeSolid,
		.R = 0.0,
		.G = 0.0,
		.B = 1.0,
		.A = 0.5,
	},
	{
		.Type = uiDrawBrushTypeSolid,
		.R = 0.0,
		.G = 1.0,
		.B = 1.0,
		.A = 0.5,
	},
};

static void draw(uiAreaDrawParams *p)
{
	uiDrawPath *path;
	uiDrawTextLayout *layout;
	uiDrawBrush b;

	b.Type = uiDrawBrushTypeSolid;

	// only clip the text, not the guides
	uiDrawSave(p->Context);

	path = uiDrawNewPath(uiDrawFillModeWinding);
	uiDrawPathAddRectangle(path, margins, margins,
		p->AreaWidth - 2 * margins,
		p->AreaHeight - 2 * margins);
	uiDrawPathEnd(path);
	uiDrawClip(p->Context, path);
	uiDrawFreePath(path);

	params.Width = p->AreaWidth - 2 * margins;
	layout = uiDrawNewTextLayout(&params);
	uiDrawText(p->Context, layout, margins, margins);

	uiDrawRestore(p->Context);

	if (uiCheckboxChecked(showLineBounds)) {
		uiDrawTextLayoutLineMetrics m;
		int i, n;
		int fill = 0;

		n = uiDrawTextLayoutNumLines(layout);
		for (i = 0; i < n; i++) {
			uiDrawTextLayoutLineGetMetrics(layout, i, &m);

			path = uiDrawNewPath(uiDrawFillModeWinding);
			uiDrawPathAddRectangle(path, margins + m.X, margins + m.Y,
				m.Width, m.Height);
			uiDrawPathEnd(path);
			uiDrawFill(p->Context, path, fillBrushes + fill);
			uiDrawFreePath(path);
			fill = (fill + 1) % 4;
		}
	}

	uiDrawFreeTextLayout(layout);
}

static struct example attributesExample;

static void changeFont(uiFontButton *b, void *data)
{
	if (defaultFont.Family != fontFamily)
		uiFreeText(defaultFont.Family);
	// TODO rename defaultFont
	uiFontButtonFont(fontButton, &defaultFont);
	redraw();
}

// TODO share?
static void checkboxChecked(uiCheckbox *c, void *data)
{
	redraw();
}

static uiCheckbox *newCheckbox(const char *text)
{
	uiCheckbox *c;

	c = uiNewCheckbox(text);
	uiCheckboxOnToggled(c, checkboxChecked, NULL);
	uiBoxAppend(panel, uiControl(c), 0);
	return c;
}

struct example *mkAttributesExample(void)
{
	panel = uiNewVerticalBox();
	showLineBounds = newCheckbox("Show Line Bounds");
	fontButton = uiNewFontButton();
	uiFontButtonOnChanged(fontButton, changeFont, NULL);
	// TODO set the font button to the current defaultFont
	uiBoxAppend(panel, uiControl(fontButton), 0);

	attributesExample.name = "Attributed Text";
	attributesExample.panel = uiControl(panel);
	attributesExample.draw = draw;
	attributesExample.mouse = NULL;
	attributesExample.key = NULL;

	setupAttributedString();
	params.String = attrstr;
	params.DefaultFont = &defaultFont;
	params.Align = uiDrawTextLayoutAlignLeft;

	return &attributesExample;
}
