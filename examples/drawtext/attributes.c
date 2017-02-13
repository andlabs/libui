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

	next = "multiple TODO";
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
static uiCheckbox *showExtents;
static uiCheckbox *showLineBounds;
static uiCheckbox *showLineGuides;

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
