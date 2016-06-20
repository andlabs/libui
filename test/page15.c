// 15 june 2016
#include "test.h"

static uiSpinbox *x, *y;
static uiSpinbox *width, *height;
static uiCheckbox *fullscreen;

static void moveX(uiSpinbox *s, void *data)
{
	uiWindow *w = uiWindow(data);
	int xp, yp;

	uiWindowPosition(w, &xp, &yp);
	xp = uiSpinboxValue(x);
	uiWindowSetPosition(w, xp, yp);
}

static void moveY(uiSpinbox *s, void *data)
{
	uiWindow *w = uiWindow(data);
	int xp, yp;

	uiWindowPosition(w, &xp, &yp);
	yp = uiSpinboxValue(y);
	uiWindowSetPosition(w, xp, yp);
}

static void updatepos(uiWindow *w)
{
	int xp, yp;

	uiWindowPosition(w, &xp, &yp);
	uiSpinboxSetValue(x, xp);
	uiSpinboxSetValue(y, yp);
}

static void center(uiButton *b, void *data)
{
	uiWindow *w = uiWindow(data);

	uiWindowCenter(w);
	updatepos(w);
}

void onMove(uiWindow *w, void *data)
{
	printf("move\n");
	updatepos(w);
}

static void sizeWidth(uiSpinbox *s, void *data)
{
	uiWindow *w = uiWindow(data);
	int xp, yp;

	uiWindowContentSize(w, &xp, &yp);
	xp = uiSpinboxValue(width);
	uiWindowSetContentSize(w, xp, yp);
}

static void sizeHeight(uiSpinbox *s, void *data)
{
	uiWindow *w = uiWindow(data);
	int xp, yp;

	uiWindowContentSize(w, &xp, &yp);
	yp = uiSpinboxValue(height);
	uiWindowSetContentSize(w, xp, yp);
}

static void updatesize(uiWindow *w)
{
	int xp, yp;

	uiWindowContentSize(w, &xp, &yp);
	uiSpinboxSetValue(width, xp);
	uiSpinboxSetValue(height, yp);
	// TODO on OS X this is updated AFTER sending the size change, not before
	uiCheckboxSetChecked(fullscreen, uiWindowFullscreen(w));
}

void onSize(uiWindow *w, void *data)
{
	printf("size\n");
	updatesize(w);
}

void setFullscreen(uiCheckbox *cb, void *data)
{
	uiWindow *w = uiWindow(data);

	uiWindowSetFullscreen(w, uiCheckboxChecked(fullscreen));
	updatesize(w);
}

static void borderless(uiCheckbox *c, void *data)
{
	uiWindow *w = uiWindow(data);

	uiWindowSetBorderless(w, uiCheckboxChecked(c));
}

uiBox *makePage15(uiWindow *w)
{
	uiBox *page15;
	uiBox *hbox;
	uiButton *button;
	uiCheckbox *checkbox;

	page15 = newVerticalBox();

	hbox = newHorizontalBox();
	// TODO if I make this 1 and not add anything else AND not call uiWindowOnPositionChanged(), on OS X the box won't be able to grow vertically
	uiBoxAppend(page15, uiControl(hbox), 0);

	uiBoxAppend(hbox, uiControl(uiNewLabel("Position")), 0);
	x = uiNewSpinbox(INT_MIN, INT_MAX);
	uiBoxAppend(hbox, uiControl(x), 1);
	y = uiNewSpinbox(INT_MIN, INT_MAX);
	uiBoxAppend(hbox, uiControl(y), 1);
	button = uiNewButton("Center");
	uiBoxAppend(hbox, uiControl(button), 0);

	uiSpinboxOnChanged(x, moveX, w);
	uiSpinboxOnChanged(y, moveY, w);
	uiButtonOnClicked(button, center, w);
	uiWindowOnPositionChanged(w, onMove, NULL);
	updatepos(w);

	hbox = newHorizontalBox();
	uiBoxAppend(page15, uiControl(hbox), 0);

	uiBoxAppend(hbox, uiControl(uiNewLabel("Size")), 0);
	width = uiNewSpinbox(INT_MIN, INT_MAX);
	uiBoxAppend(hbox, uiControl(width), 1);
	height = uiNewSpinbox(INT_MIN, INT_MAX);
	uiBoxAppend(hbox, uiControl(height), 1);
	fullscreen = uiNewCheckbox("Fullscreen");
	uiBoxAppend(hbox, uiControl(fullscreen), 0);

	uiSpinboxOnChanged(width, sizeWidth, w);
	uiSpinboxOnChanged(height, sizeHeight, w);
	uiCheckboxOnToggled(fullscreen, setFullscreen, w);
	uiWindowOnContentSizeChanged(w, onSize, NULL);
	updatesize(w);

	checkbox = uiNewCheckbox("Borderless");
	uiCheckboxOnToggled(checkbox, borderless, w);
	uiBoxAppend(page15, uiControl(checkbox), 0);

	hbox = newHorizontalBox();
	uiBoxAppend(page15, uiControl(hbox), 1);

	uiBoxAppend(hbox, uiControl(uiNewVerticalSeparator()), 0);

	return page15;
}
