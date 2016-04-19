// 22 december 2015
#include "test.h"

// TODO draw a rectangle pointing out where (10,10) is both to test initial colors and to figure out what the *real* ascent is

static uiEntry *textString;
static uiFontButton *textFontButton;
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
	uiDrawTextFontDescriptor desc;
	uiDrawTextFont *font;
	uiDrawTextLayout *layout;

	// TODO
	memset(&desc, 0, sizeof (uiDrawTextFontDescriptor));
	family = uiEntryText(textFont);
	desc.Family = "Arial";
	desc.Size = 36;
	desc.Weight = uiDrawTextWeightNormal;
	desc.Italic = uiDrawTextItalicNormal;
	desc.Stretch = uiDrawTextStretchNormal;
	font = uiDrawLoadClosestFont(&desc);

	layout = uiDrawNewTextLayout(s, "One two three four", -1);
	uiDrawTextLayoutSetForegroundColor(layout,
		4, 7,
		1, 0, 0, 1);
	uiDrawTextLayoutSetForegroundColor(layout,
		8, 14,
		1, 0, 0.5, 0.5);
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
	uiBoxAppend(hbox, uiControl(textFontButton), 1);

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

	return page10;
}
