// 10 march 2018
#include <stdio.h>
#include <string.h>
#include "../../ui.h"

uiWindow *mainwin;
uiArea *area;
uiAreaHandler handler;
uiFontButton *fontButton;

uiAttributedString *attrstr;

#define margins 20

static void appendWithAttribute(const char *what, uiAttribute *attr)
{
	size_t start, end;

	start = uiAttributedStringLen(attrstr);
	end = start + strlen(what);
	uiAttributedStringAppendUnattributed(attrstr, what);
	uiAttributedStringSetAttribute(attrstr, attr, start, end);
}

static void makeAttributedString(void)
{
	uiAttribute *attr;

	attrstr = uiNewAttributedString(
		"Drawing strings with libui is done with the uiAttributedString and uiDrawTextLayout obects.\n"
		"uiAttributedString lets you have a variety of attributes: ");

	attr = uiNewFamilyAttribute("Courier New");
	appendWithAttribute("font family", attr);
	uiAttributedStringAppendUnattributed(attrstr, ", ");

	attr = uiNewSizeAttribute(18);
	appendWithAttribute("font size", attr);
	uiAttributedStringAppendUnattributed(attrstr, ", ");

	attr = uiNewWeightAttribute(uiTextWeightBold);
	appendWithAttribute("font weight", attr);
	uiAttributedStringAppendUnattributed(attrstr, ", ");

	attr = uiNewItalicAttribute(uiTextItalicItalic);
	appendWithAttribute("font italicness", attr);
	uiAttributedStringAppendUnattributed(attrstr, ", ");

	attr = uiNewStretchAttribute(uiTextStretchCondensed);
	appendWithAttribute("font stretch", attr);
	uiAttributedStringAppendUnattributed(attrstr, ", ");

	attr = uiNewColorAttribute(0.75, 0.25, 0.5, 0.75);
	appendWithAttribute("text color", attr);
	uiAttributedStringAppendUnattributed(attrstr, ", ");

	attr = uiNewBackgroundAttribute(0.5, 0.5, 0.25, 0.5);
	appendWithAttribute("text background color", attr);
	uiAttributedStringAppendUnattributed(attrstr, ", ");
}

static void handlerDraw(uiAreaHandler *a, uiArea *area, uiAreaDrawParams *p)
{
	uiDrawTextLayout *textLayout;
	uiFontDescriptor fontdesc;
	uiDrawTextLayoutParams params;

	params.String = attrstr;
	uiFontButtonFont(fontButton, &fontdesc);
	params.Width = p->AreaWidth - (2 * margins);
	params.Align = uiDrawTextAlignLeft;
	textLayout = uiDrawNewTextLayout(&params);
	// TODO clip to margins
	uiDrawText(p->Context, textLayout, margins, margins);
	uiDrawFreeTextLayout(textLayout);
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

static int onClosing(uiWindow *w, void *data)
{
	uiControlDestroy(uiControl(mainwin));
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
	uiBox *hbox, *vbox;

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

	makeAttributedString();

	mainwin = uiNewWindow("libui Histogram Example", 640, 480, 1);
	uiWindowSetMargined(mainwin, 1);
	uiWindowOnClosing(mainwin, onClosing, NULL);

	hbox = uiNewHorizontalBox();
	uiBoxSetPadded(hbox, 1);
	uiWindowSetChild(mainwin, uiControl(hbox));

	vbox = uiNewVerticalBox();
	uiBoxSetPadded(vbox, 1);
	uiBoxAppend(hbox, uiControl(vbox), 0);

	fontButton = uiNewFontButton();
	uiBoxAppend(vbox, uiControl(fontButton), 0);

	area = uiNewArea(&handler);
	uiBoxAppend(hbox, uiControl(area), 1);

	uiControlShow(uiControl(mainwin));
	uiMain();
	uiUninit();
	return 0;
}
