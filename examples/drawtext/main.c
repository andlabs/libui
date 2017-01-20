// 17 january 2017
#include <stdio.h>
#include <string.h>
#include "../../ui.h"

uiWindow *mainwin;
uiArea *area;
uiAreaHandler handler;

const char *text =
	"It is with a kind of fear that I begin to write the history of my life. "
	"I have, as it were, a superstitious hesitation in lifting the veil that "
	"clings about my childhood like a golden mist. The task of writing an "
	"autobiography is a difficult one. When I try to classify my earliest "
	"impressions, I find that fact and fancy look alike across the years that "
	"link the past with the present. The woman paints the child's experiences "
	"in her own fantasy. A few impressions stand out vividly from the first "
	"years of my life; but \"the shadows of the prison-house are on the rest.\" "
	"Besides, many of the joys and sorrows of childhood have lost their "
	"poignancy; and many incidents of vital importance in my early education "
	"have been forgotten in the excitement of great discoveries. In order, "
	"therefore, not to be tedious I shall try to present in a series of "
	"sketches only the episodes that seem to me to be the most interesting "
	"and important."
	"";
char fontFamily[] = "Palatino";
// TODO should be const; look at constructor function
uiDrawFontDescriptor defaultFont = {
	.Family = fontFamily,
	.Size = 18,
	.Weight = uiDrawTextWeightNormal,
	.Italic = uiDrawTextItalicNormal,
	.Stretch = uiDrawTextStretchNormal,
};
uiAttributedString *attrstr;

#define margins 10

static void handlerDraw(uiAreaHandler *a, uiArea *area, uiAreaDrawParams *p)
{
	uiDrawPath *path;
	uiDrawTextLayout *layout;

	path = uiDrawNewPath(uiDrawFillModeWinding);
	uiDrawPathAddRectangle(path, margins, margins,
		p->AreaWidth - 2 * margins,
		p->AreaHeight - 2 * margins);
	uiDrawPathEnd(path);
	uiDrawClip(p->Context, path);
	uiDrawFreePath(path);

	// TODO get rid of this later
	path = uiDrawNewPath(uiDrawFillModeWinding);
	uiDrawPathAddRectangle(path, -100, -100,
		p->AreaWidth * 2,
		p->AreaHeight * 2);
	uiDrawPathEnd(path);
	uiDrawBrush b;
	b.Type = uiDrawBrushTypeSolid;
	b.R = 0.0;
	b.G = 1.0;
	b.B = 0.0;
	b.A = 1.0;
	uiDrawFill(p->Context, path, &b);
	uiDrawFreePath(path);

	layout = uiDrawNewTextLayout(attrstr,
		&defaultFont,
		p->AreaWidth - 2 * margins);
	uiDrawText(p->Context, layout, margins, margins);
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

	attrstr = uiNewAttributedString(text);

	mainwin = uiNewWindow("libui Text-Drawing Example", 640, 480, 1);
	uiWindowOnClosing(mainwin, onClosing, NULL);

	area = uiNewArea(&handler);
	// TODO on GTK+ this doesn't get expand properties set properly?
	uiWindowSetChild(mainwin, uiControl(area));

	uiControlShow(uiControl(mainwin));
	uiMain();
	uiFreeAttributedString(attrstr);
	uiUninit();
	return 0;
}
