// 10 june 2017
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include "../../ui.h"

// TODO the grid simply flat out does not work on OS X

uiWindow *mainwin;
uiFontButton *fontButton;
uiEntry *textEntry;
uiCheckbox *nullFeatures;
uiArea *area;

uiAttributedString *attrstr = NULL;

static void remakeAttrStr(void)
{
	char *text;
	uiOpenTypeFeatures *otf;
	uiAttributeSpec spec;

	if (attrstr != NULL)
		uiFreeAttributedString(attrstr);

	text = uiEntryText(textEntry);
	attrstr = uiNewAttributedString(text);
	uiFreeText(text);

	if (!uiCheckboxChecked(nullFeatures)) {
		otf = uiNewOpenTypeFeatures();
		// TODO
		spec.Type = uiAttributeFeatures;
		spec.Features = otf;
		uiAttributedStringSetAttribute(attrstr, &spec,
			0, uiAttributedStringLen(attrstr));
		// and uiAttributedString copied otf
		uiFreeOpenTypeFeatures(otf);
	}

	uiAreaQueueRedrawAll(area);
}

// TODO make a variable of main()? in all programs?
static uiAreaHandler handler;

static void handlerDraw(uiAreaHandler *a, uiArea *area, uiAreaDrawParams *p)
{
	uiDrawTextLayout *layout;
	uiDrawTextLayoutParams lp;
	uiDrawFontDescriptor desc;

	memset(&lp, 0, sizeof (uiDrawTextLayoutParams));
	lp.String = attrstr;
	uiFontButtonFont(fontButton, &desc);
	lp.DefaultFont = &desc;
	lp.Width = p->AreaWidth;
	lp.Align = uiDrawTextAlignLeft;
	layout = uiDrawNewTextLayout(&lp);
	uiDrawText(p->Context, layout, 0, 0);
	uiDrawFreeTextLayout(layout);
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
	// reject all keys
	return 0;
}

static void onFontChanged(uiFontButton *b, void *data)
{
	remakeAttrStr();
}

static void onTextChanged(uiEntry *e, void *data)
{
	remakeAttrStr();
}

static void onNULLToggled(uiCheckbox *c, void *data)
{
	remakeAttrStr();
}

static int onClosing(uiWindow *w, void *data)
{
	// TODO change the others to be like this? (the others destroy here rather than later)
	// TODO move this below uiQuit()?
	uiControlHide(uiControl(w));
	uiQuit();
	return 0;
}

static int shouldQuit(void *data)
{
	uiControlDestroy(uiControl(mainwin));
	return 1;
}

int main(void)
{
	uiInitOptions o;
	const char *err;
	uiGrid *grid;
	uiBox *vbox;

	handler.Draw = handlerDraw;
	handler.MouseEvent = handlerMouseEvent;
	handler.MouseCrossed = handlerMouseCrossed;
	handler.DragBroken = handlerDragBroken;
	handler.KeyEvent = handlerKeyEvent;

	memset(&o, 0, sizeof (uiInitOptions));
	err = uiInit(&o);
	if (err != NULL) {
		fprintf(stderr, "error initializing ui: %s\n", err);
		uiFreeInitError(err);
		return 1;
	}

	uiOnShouldQuit(shouldQuit, NULL);

	// TODO 800x600? the size of the GTK+ example?
	mainwin = uiNewWindow("libui OpenType Features Example", 640, 480, 1);
	uiWindowSetMargined(mainwin, 1);
	uiWindowOnClosing(mainwin, onClosing, NULL);

	grid = uiNewGrid();
	uiGridSetPadded(grid, 1);
	uiWindowSetChild(mainwin, uiControl(grid));

	fontButton = uiNewFontButton();
	uiFontButtonOnChanged(fontButton, onFontChanged, NULL);
	uiGridAppend(grid, uiControl(fontButton),
		0, 0, 1, 1,
		// TODO are these Y values correct?
		0, uiAlignFill, 0, uiAlignCenter);

	textEntry = uiNewEntry();
	uiEntrySetText(textEntry, "afford afire aflight");
	uiEntryOnChanged(textEntry, onTextChanged, NULL);
	uiGridAppend(grid, uiControl(textEntry),
		1, 0, 1, 1,
		// TODO are these Y values correct too?
		// TODO add a baseline align? or a form align?
		1, uiAlignFill, 0, uiAlignCenter);

	vbox = uiNewVerticalBox();
	uiBoxSetPadded(vbox, 1);
	uiGridAppend(grid, uiControl(vbox),
		0, 1, 1, 1,
		0, uiAlignFill, 1, uiAlignFill);

	nullFeatures = uiNewCheckbox("NULL uiOpenTypeFeatures");
	uiCheckboxOnToggled(nullFeatures, onNULLToggled, NULL);
	uiBoxAppend(vbox, uiControl(nullFeatures), 0);

	// TODO separator (if other stuff isn't a tab)

	// TODO needed for this to be testable on os x without rewriting everything again
	{
		int x;

		for (x = 0; x < 10; x++)
			uiBoxAppend(vbox, uiControl(uiNewEntry()), 0);
	}

	// TODO other stuff

	area = uiNewArea(&handler);
	uiGridAppend(grid, uiControl(area),
		1, 1, 1, 1,
		1, uiAlignFill, 1, uiAlignFill);

	// and set up the initial draw
	remakeAttrStr();

	uiControlShow(uiControl(mainwin));
	uiMain();

	uiControlDestroy(uiControl(mainwin));
	uiFreeAttributedString(attrstr);
	uiUninit();
	return 0;
}
