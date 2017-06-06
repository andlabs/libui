// 20 january 2017
#include "drawtext.h"

// TODO double-check ligatures on all platforms to make sure we can place the cursor at the right place
// TODO using the arrow keys allows us to walk back to the end of the line on some platforms (TODO which?); IIRC arrow keys shouldn't do that
// TODO make sure to check the cursor positions of RTL on all platforms

static const char *text =
	"Each of the glyphs an end user interacts with are called graphemes. "
	"If you enter a byte range in the text boxes below and click the button, you can see the blue box move to surround that byte range, as well as what the actual byte range necessary is. "
	// TODO rephrase this; I don't think this code will use those grapheme functions...
	"You'll also see the index of the first grapheme; uiAttributedString has facilities for converting between UTF-8 code points and grapheme indices. "
	"Additionally, click on the string to move the caret. Watch the status text at the bottom change too. "
	"That being said: "
	"\xC3\x93O\xCC\x81 (combining accents) "
	"A\xCC\xAA\xEF\xB8\xA0 (multiple combining characters) "
	"\xE2\x80\xAE#\xE2\x80\xAC (RTL glyph) "
	"\xF0\x9F\x92\xBB (non-BMP character) "
	"\xF0\x9F\x92\xBB\xCC\x80 (combined non-BMP character; may render strangely) "
	"";
static char fontFamily[] = "Helvetica";
static uiDrawFontDescriptor defaultFont = {
	.Family = fontFamily,
	.Size = 14,
	.Weight = uiDrawTextWeightNormal,
	.Italic = uiDrawTextItalicNormal,
	.Stretch = uiDrawTextStretchNormal,
};
static uiAttributedString *attrstr;
static uiDrawTextLayoutParams params;

#define margins 10

static uiBox *panel;
static uiBox *vbox;
static uiLabel *caretLabel;
static uiCheckbox *showLineBounds;
static uiFontButton *fontButton;
static uiCombobox *textAlign;

static int caretLine = -1;
static size_t caretPos;

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
	uiDrawTextLayoutLineMetrics m;

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

	if (caretLine == -1) {
		caretLine = uiDrawTextLayoutNumLines(layout) - 1;
		caretPos = uiAttributedStringLen(attrstr);
	}
	uiDrawCaret(p->Context, margins, margins,
		layout, caretPos, &caretLine);

	if (uiCheckboxChecked(showLineBounds)) {
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

static void mouse(uiAreaMouseEvent *e)
{
	uiDrawTextLayout *layout;
	char labelText[128];

	if (e->Down != 1)
		return;

	params.Width = e->AreaWidth - 2 * margins;
	layout = uiDrawNewTextLayout(&params);
	uiDrawTextLayoutHitTest(layout,
		e->X - margins, e->Y - margins,
		&caretPos, &caretLine);
	uiDrawFreeTextLayout(layout);

	// TODO move this into the draw handler so it is reflected by keyboard-based position changes
	// urgh %zd is not supported by MSVC with sprintf()
	// TODO get that part in test/ about having no other option
	sprintf(labelText, "pos %d line %d",
		(int) caretPos, caretLine);
	uiLabelSetText(caretLabel, labelText);

	redraw();
}

static int key(uiAreaKeyEvent *e)
{
	size_t grapheme;

	if (e->Up)
		return 0;
	if (e->Key != 0)
		return 0;
	switch (e->ExtKey) {
	case uiExtKeyUp:
		// TODO
		return 1;
	case uiExtKeyDown:
		// TODO
		return 1;
	case uiExtKeyLeft:
		grapheme = uiAttributedStringByteIndexToGrapheme(attrstr, caretPos);
		if (grapheme == 0)
			return 0;
		grapheme--;
		caretPos = uiAttributedStringGraphemeToByteIndex(attrstr, grapheme);
		redraw();
		return 1;
	case uiExtKeyRight:
		grapheme = uiAttributedStringByteIndexToGrapheme(attrstr, caretPos);
		if (grapheme == uiAttributedStringNumGraphemes(attrstr))
			return 0;
		grapheme++;
		caretPos = uiAttributedStringGraphemeToByteIndex(attrstr, grapheme);
		redraw();
		return 1;
	}
	return 0;
}

static struct example hitTestExample;

// TODO share?
static void checkboxChecked(uiCheckbox *c, void *data)
{
	redraw();
}

static void changeFont(uiFontButton *b, void *data)
{
	if (defaultFont.Family != fontFamily)
		uiFreeText(defaultFont.Family);
	// TODO rename defaultFont
	uiFontButtonFont(fontButton, &defaultFont);
	printf("{\n\tfamily: %s\n\tsize: %g\n\tweight: %d\n\titalic: %d\n\tstretch: %d\n}\n",
		defaultFont.Family,
		defaultFont.Size,
		(int) (defaultFont.Weight),
		(int) (defaultFont.Italic),
		(int) (defaultFont.Stretch));
	redraw();
}

static void changeTextAlign(uiCombobox *c, void *data)
{
	// note the order of the items added below
	params.Align = (uiDrawTextAlign) uiComboboxSelected(textAlign);
	redraw();
}

// TODO share?
static uiCheckbox *newCheckbox(uiBox *box, const char *text)
{
	uiCheckbox *c;

	c = uiNewCheckbox(text);
	uiCheckboxOnToggled(c, checkboxChecked, NULL);
	uiBoxAppend(box, uiControl(c), 0);
	return c;
}

struct example *mkHitTestExample(void)
{
	panel = uiNewHorizontalBox();
	vbox = uiNewVerticalBox();
	// TODO the second vbox causes this not to stretch at least on OS X
	uiBoxAppend(panel, uiControl(vbox), 1);
	caretLabel = uiNewLabel("Caret information is shown here");
	uiBoxAppend(vbox, uiControl(caretLabel), 0);
	showLineBounds = newCheckbox(vbox, "Show Line Bounds (for debugging metrics)");
	vbox = uiNewVerticalBox();
	uiBoxAppend(panel, uiControl(vbox), 0);
	fontButton = uiNewFontButton();
	uiFontButtonOnChanged(fontButton, changeFont, NULL);
	// TODO set the font button to the current defaultFont
	uiBoxAppend(vbox, uiControl(fontButton), 0);
	textAlign = uiNewCombobox();
	// note that these are in the order in the enum
	uiComboboxAppend(textAlign, "Left");
	uiComboboxAppend(textAlign, "Center");
	uiComboboxAppend(textAlign, "Right");
	uiComboboxOnSelected(textAlign, changeTextAlign, NULL);
	uiBoxAppend(vbox, uiControl(textAlign), 0);

	hitTestExample.name = "Hit-Testing and Grapheme Boundaries";
	hitTestExample.panel = uiControl(panel);
	hitTestExample.draw = draw;
	hitTestExample.mouse = mouse;
	hitTestExample.key = key;

	attrstr = uiNewAttributedString(text);
	params.String = attrstr;
	params.DefaultFont = &defaultFont;
	params.Align = uiDrawTextAlignLeft;

	return &hitTestExample;
}
