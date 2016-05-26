// 22 december 2015
#include "test.h"

static uiEntry *textString;
static uiFontButton *textFontButton;
static uiColorButton *textColorButton;
static uiEntry *textWidth;
static uiButton *textApply;
static uiCheckbox *noZ;
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

static void handlerDraw(uiAreaHandler *a, uiArea *area, uiAreaDrawParams *dp)
{
	uiDrawTextFont *font;
	uiDrawTextLayout *layout;
	double r, g, b, al;
	char surrogates[1 + 4 + 1 + 1];
	char composed[2 + 2 + 2 + 3 + 2 + 1];
	double width, height;

	font = uiFontButtonFont(textFontButton);

	layout = uiDrawNewTextLayout("One two three four", font, -1);
	uiDrawTextLayoutSetColor(layout,
		4, 7,
		1, 0, 0, 1);
	uiDrawTextLayoutSetColor(layout,
		8, 14,
		1, 0, 0.5, 0.5);
	uiColorButtonColor(textColorButton, &r, &g, &b, &al);
	uiDrawTextLayoutSetColor(layout,
		14, 18,
		r, g, b, al);
	uiDrawText(dp->Context, 10, 10, layout);
	uiDrawTextLayoutExtents(layout, &width, &height);
	uiDrawFreeTextLayout(layout);

	surrogates[0] = 'x';
	surrogates[1] = 0xF0;		// surrogates D800 DF08
	surrogates[2] = 0x90;
	surrogates[3] = 0x8C;
	surrogates[4] = 0x88;
	surrogates[5] = 'y';
	surrogates[6] = '\0';

	layout = uiDrawNewTextLayout(surrogates, font, -1);
	uiDrawTextLayoutSetColor(layout,
		1, 2,
		1, 0, 0.5, 0.5);
	uiDrawText(dp->Context, 10, 10 + height, layout);
	uiDrawFreeTextLayout(layout);

	composed[0] = 'z';
	composed[1] = 'z';
	composed[2] = 0xC3;		// 2
	composed[3] = 0xA9;
	composed[4] = 'z';
	composed[5] = 'z';
	composed[6] = 0x65;		// 5
	composed[7] = 0xCC;
	composed[8] = 0x81;
	composed[9] = 'z';
	composed[10] = 'z';
	composed[11] = '\0';

	layout = uiDrawNewTextLayout(composed, font, -1);
	uiDrawTextLayoutSetColor(layout,
		2, 3,
		1, 0, 0.5, 0.5);
	uiDrawTextLayoutSetColor(layout,
		5, 6,
		1, 0, 0.5, 0.5);
	if (!uiCheckboxChecked(noZ))
		uiDrawTextLayoutSetColor(layout,
			6, 7,
			0.5, 0, 1, 0.5);
	uiDrawText(dp->Context, 10, 10 + height + height, layout);
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

static void onFontChanged(uiFontButton *b, void *data)
{
	uiAreaQueueRedrawAll(textArea);
}

static void onColorChanged(uiColorButton *b, void *data)
{
	uiAreaQueueRedrawAll(textArea);
}

static void onNoZ(uiCheckbox *b, void *data)
{
	uiAreaQueueRedrawAll(textArea);
}

uiBox *makePage10(void)
{
	uiBox *page10;
	uiBox *vbox;
	uiBox *hbox;

	page10 = newVerticalBox();
	vbox = page10;

	hbox = newHorizontalBox();
	uiBoxAppend(vbox, uiControl(hbox), 0);

	textString = uiNewEntry();
	// TODO make it placeholder
	uiEntrySetText(textString, "Enter text here");
	uiBoxAppend(hbox, uiControl(textString), 1);

	textFontButton = uiNewFontButton();
	uiFontButtonOnChanged(textFontButton, onFontChanged, NULL);
	uiBoxAppend(hbox, uiControl(textFontButton), 1);

	textColorButton = uiNewColorButton();
	uiColorButtonOnChanged(textColorButton, onColorChanged, NULL);
	uiBoxAppend(hbox, uiControl(textColorButton), 1);

	hbox = newHorizontalBox();
	uiBoxAppend(vbox, uiControl(hbox), 0);

	textApply = uiNewButton("Apply");
	uiBoxAppend(hbox, uiControl(textApply), 1);

	textWidth = uiNewEntry();
	uiEntrySetText(textWidth, "-1");
	uiBoxAppend(hbox, uiControl(textWidth), 1);

	noZ = uiNewCheckbox("No Z Color");
	uiCheckboxOnToggled(noZ, onNoZ, NULL);
	uiBoxAppend(hbox, uiControl(noZ), 0);

	textAreaHandler.Draw = handlerDraw;
	textAreaHandler.MouseEvent = handlerMouseEvent;
	textAreaHandler.MouseCrossed = handlerMouseCrossed;
	textAreaHandler.DragBroken = handlerDragBroken;
	textAreaHandler.KeyEvent = handlerKeyEvent;
	textArea = uiNewArea(&textAreaHandler);
	uiBoxAppend(vbox, uiControl(textArea), 1);

	// dummy objects to test single-activation
	hbox = newHorizontalBox();
	uiBoxAppend(vbox, uiControl(hbox), 0);
	uiBoxAppend(hbox, uiControl(uiNewFontButton()), 1);
	uiBoxAppend(hbox, uiControl(uiNewColorButton()), 1);

	return page10;
}
