// 15 june 2016
#include "test.h"

static uiSpinbox *width, *height;
static uiCheckbox *fullscreen;

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
