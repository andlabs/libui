// 22 december 2015
#include "test.h"

// TODO draw a rectangle pointing out where (10,10) is both to test initial colors and to figure out what the *real* ascent is

static uiEntry *textString;
static uiEntry *textFont;
static uiEntry *textSize;
static uiCombobox *textWeight;
static uiCombobox *textItalic;
static uiCheckbox *textSmallCaps;
static uiCombobox *textStretch;
static uiCombobox *textGravity;
static uiEntry *textWidth;
static uiButton *textApply;
static uiCheckbox *addLeading;
static uiArea *textArea;
static uiAreaHandler textAreaHandler;

static double entryDouble(uiEntry *e)
{
	char *s;
	double d;

	s = uiEntryText(e);
	d = atof(s);
	uiFreeText(s);
	return d;
}

// TODO this should be altered not to restore all state on exit so default text attributes can be checked
static void drawGuides(uiDrawContext *c, uiDrawTextFontMetrics *m)
{
	uiDrawPath *p;
	uiDrawBrush b;
	uiDrawStrokeParams sp;

	memset(&b, 0, sizeof (uiDrawBrush));
	b.Type = uiDrawBrushTypeSolid;
	memset(&sp, 0, sizeof (uiDrawStrokeParams));
	sp.Cap = uiDrawLineCapFlat;
	sp.Join = uiDrawLineJoinMiter;
	sp.MiterLimit = uiDrawDefaultMiterLimit;
	sp.Thickness = 2;

	uiDrawSave(c);

	p = uiDrawNewPath(uiDrawFillModeWinding);
	uiDrawPathNewFigure(p, 8, 10);
	uiDrawPathLineTo(p, 8, 10 + m->Ascent);
	uiDrawPathEnd(p);
	b.R = 0.94;
	b.G = 0.5;
	b.B = 0.5;
	b.A = 1.0;
	uiDrawStroke(c, p, &b, &sp);
	uiDrawFreePath(p);

	p = uiDrawNewPath(uiDrawFillModeWinding);
	uiDrawPathNewFigure(p, 8, 10 + m->Ascent);
	uiDrawPathLineTo(p, 8, 10 + m->Ascent + m->Descent);
	uiDrawPathEnd(p);
	b.R = 0.12;
	b.G = 0.56;
	b.B = 1.0;
	b.A = 1.0;
	uiDrawStroke(c, p, &b, &sp);
	uiDrawFreePath(p);

	p = uiDrawNewPath(uiDrawFillModeWinding);
	uiDrawPathAddRectangle(p, 0, 0, 10, 10);
	uiDrawPathEnd(p);
	uiDrawClip(c, p);
	b.R = 0.85;
	b.G = 0.65;
	b.B = 0.13;
	b.A = 1.0;
	uiDrawStroke(c, p, &b, &sp);
	uiDrawFreePath(p);

	uiDrawRestore(c);
}

static void handlerDraw(uiAreaHandler *a, uiArea *area, uiAreaDrawParams *dp)
{
	uiDrawTextFontDescriptor desc;
	uiDrawTextFont *font;
	char *s;
	char *family;		// make compiler happy
	uiDrawTextLayout *layout;
	uiDrawTextFontMetrics metrics;
	double ypos;
	double width;
	double height;

	memset(&desc, 0, sizeof (uiDrawTextFontDescriptor));
	family = uiEntryText(textFont);
	desc.Family = family;
	desc.Size = entryDouble(textSize);
	desc.Weight = uiComboboxSelected(textWeight);
	desc.Italic = uiComboboxSelected(textItalic);
	desc.SmallCaps = uiCheckboxChecked(textSmallCaps);
	desc.Stretch = uiComboboxSelected(textStretch);
	desc.Gravity = uiComboboxSelected(textGravity);
	font = uiDrawLoadClosestFont(&desc);
	uiFreeText(family);
	uiDrawTextFontGetMetrics(font, &metrics);

	width = entryDouble(textWidth);

	drawGuides(dp->Context, &metrics);

	s = uiEntryText(textString);
	layout = uiDrawNewTextLayout(s, font, width);
	uiFreeText(s);
	ypos = 10;
	uiDrawText(dp->Context, 10, ypos, layout);
	// TODO make these optional?
	uiDrawTextLayoutExtents(layout, &width, &height);
	uiDrawFreeTextLayout(layout);

	layout = uiDrawNewTextLayout("This is a second line", font, -1);
	if (/*TODO reuse width*/entryDouble(textWidth) < 0) {
		double ad;

		ad = metrics.Ascent + metrics.Descent;
		printf("ad:%g extent:%g\n", ad, height);
	}
	ypos += height;
	if (uiCheckboxChecked(addLeading))
		ypos += metrics.Leading;
	uiDrawText(dp->Context, 10, ypos, layout);
	uiDrawFreeTextLayout(layout);

	uiDrawFreeTextFont(font);
}

static void handlerMouseEvent(uiAreaHandler *a, uiArea *area, uiAreaMouseEvent *e)
{
	// do nothing
}

static void handlerMouseCrossed(uiAreaHandler *ah, uiArea *a, int left)
{
	// do nothing
}

static void handlerDragBroken(uiAreaHandler *ah, uiArea *a)
{
	// do nothing
}

static int handlerKeyEvent(uiAreaHandler *ah, uiArea *a, uiAreaKeyEvent *e)
{
	// do nothing
	return 0;
}

static void onTextApply(uiButton *b, void *data)
{
	uiAreaQueueRedrawAll(textArea);
}

uiBox *makePage9(void)
{
	uiBox *page9;
	uiBox *vbox;
	uiBox *hbox;

	page9 = newVerticalBox();
	vbox = page9;

	hbox = newHorizontalBox();
	uiBoxAppend(vbox, uiControl(hbox), 0);

	textString = uiNewEntry();
	// TODO make it placeholder
	uiEntrySetText(textString, "Enter text here");
	uiBoxAppend(hbox, uiControl(textString), 1);

	textFont = uiNewEntry();
	uiEntrySetText(textFont, "Arial");
	uiBoxAppend(hbox, uiControl(textFont), 1);

	textSize = uiNewEntry();
	uiEntrySetText(textSize, "10");
	uiBoxAppend(hbox, uiControl(textSize), 1);

	hbox = newHorizontalBox();
	uiBoxAppend(vbox, uiControl(hbox), 0);

	textWeight = uiNewCombobox();
	uiComboboxAppend(textWeight, "Thin");
	uiComboboxAppend(textWeight, "Ultra Light");
	uiComboboxAppend(textWeight, "Light");
	uiComboboxAppend(textWeight, "Book");
	uiComboboxAppend(textWeight, "Normal");
	uiComboboxAppend(textWeight, "Medium");
	uiComboboxAppend(textWeight, "Semi Bold");
	uiComboboxAppend(textWeight, "Bold");
	uiComboboxAppend(textWeight, "Utra Bold");
	uiComboboxAppend(textWeight, "Heavy");
	uiComboboxAppend(textWeight, "Ultra Heavy");
	uiComboboxSetSelected(textWeight, uiDrawTextWeightNormal);
	uiBoxAppend(hbox, uiControl(textWeight), 1);

	textItalic = uiNewCombobox();
	uiComboboxAppend(textItalic, "Normal");
	uiComboboxAppend(textItalic, "Oblique");
	uiComboboxAppend(textItalic, "Italic");
	uiComboboxSetSelected(textItalic, uiDrawTextItalicNormal);
	uiBoxAppend(hbox, uiControl(textItalic), 1);

	textSmallCaps = uiNewCheckbox("Small Caps");
	uiBoxAppend(hbox, uiControl(textSmallCaps), 1);

	hbox = newHorizontalBox();
	uiBoxAppend(vbox, uiControl(hbox), 0);

	textStretch = uiNewCombobox();
	uiComboboxAppend(textStretch, "Ultra Condensed");
	uiComboboxAppend(textStretch, "Extra Condensed");
	uiComboboxAppend(textStretch, "Condensed");
	uiComboboxAppend(textStretch, "Semi Condensed");
	uiComboboxAppend(textStretch, "Normal");
	uiComboboxAppend(textStretch, "Semi Expanded");
	uiComboboxAppend(textStretch, "Expanded");
	uiComboboxAppend(textStretch, "Extra Expanded");
	uiComboboxAppend(textStretch, "Ultra Expanded");
	uiComboboxSetSelected(textStretch, uiDrawTextStretchNormal);
	uiBoxAppend(hbox, uiControl(textStretch), 1);

	textGravity = uiNewCombobox();
	uiComboboxAppend(textGravity, "South");
	uiComboboxAppend(textGravity, "East");
	uiComboboxAppend(textGravity, "North");
	uiComboboxAppend(textGravity, "West");
	uiComboboxAppend(textGravity, "Auto");
	uiComboboxSetSelected(textGravity, uiDrawTextGravitySouth);
	uiBoxAppend(hbox, uiControl(textGravity), 1);

	textWidth = uiNewEntry();
	uiEntrySetText(textWidth, "-1");
	uiBoxAppend(hbox, uiControl(textWidth), 1);

	hbox = newHorizontalBox();
	uiBoxAppend(vbox, uiControl(hbox), 0);

	textApply = uiNewButton("Apply");
	uiButtonOnClicked(textApply, onTextApply, NULL);
	uiBoxAppend(hbox, uiControl(textApply), 1);

	addLeading = uiNewCheckbox("Add Leading");
	uiCheckboxSetChecked(addLeading, 1);
	uiBoxAppend(hbox, uiControl(addLeading), 0);

	textAreaHandler.Draw = handlerDraw;
	textAreaHandler.MouseEvent = handlerMouseEvent;
	textAreaHandler.MouseCrossed = handlerMouseCrossed;
	textAreaHandler.DragBroken = handlerDragBroken;
	textAreaHandler.KeyEvent = handlerKeyEvent;
	textArea = uiNewArea(&textAreaHandler);
	uiBoxAppend(vbox, uiControl(textArea), 1);

	return page9;
}
