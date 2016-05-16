// 22 december 2015
#include "test.h"

// TODO figure out how the various backends handle non-BMP characters/surrogate pairs
// use: F0 90 8C 88 (surrogates D800 DF08)

static uiEntry *textString;
static uiFontButton *textFontButton;
static uiColorButton *textColorButton;
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

static void handlerDraw(uiAreaHandler *a, uiArea *area, uiAreaDrawParams *dp)
{
	uiDrawTextFont *font;
	uiDrawTextLayout *layout;
	double r, g, b, al;

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

static void onTextApply(uiButton *b, void *data)
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
	uiButtonOnClicked(textApply, onTextApply, NULL);
	uiBoxAppend(hbox, uiControl(textApply), 1);

	textWidth = uiNewEntry();
	uiEntrySetText(textWidth, "-1");
	uiBoxAppend(hbox, uiControl(textWidth), 1);

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

	// dummy objects to test single-activation
	hbox = newHorizontalBox();
	uiBoxAppend(vbox, uiControl(hbox), 0);
	uiBoxAppend(hbox, uiControl(uiNewFontButton()), 1);
	uiBoxAppend(hbox, uiControl(uiNewColorButton()), 1);

	return page10;
}
