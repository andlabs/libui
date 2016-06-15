// 15 june 2016
#include "test.h"

void moveX(uiSpinbox *s, void *data)
{
	uiWindow *w = uiWindow(data);
	int x, y;

	uiWindowPosition(w, &x, &y);
	x = uiSpinboxValue(s);
	uiWindowSetPosition(w, x, y);
}

void moveX(uiSpinbox *s, void *data)
{
	uiWindow *w = uiWindow(data);
	int x, y;

	uiWindowPosition(w, &x, &y);
	y = uiSpinboxValue(s);
	uiWindowSetPosition(w, x, y);
}

// TODO onMove

uiBox *makePage15(uiWindow *w)
{
	uiBox *page15;
	uiBox *hbox;
	uiSpinbox *x;
	uiSpinbox *y;
	int curx, cury;

	page15 = newVerticalBox();

	hbox = newHorizontalBox();
	uiBoxAppend(page15, uiControl(hbox), 1);

	uiBoxAppend(hbox, uiControl(uiNewLabel("Position")), 0);
	x = uiNewSpinbox(INT_MIN, INT_MAX);
	uiBoxAppend(hbox, uiControl(x), 1);
	y = uiNewSpinbox(INT_MIN, INT_MAX);
	uiBoxAppend(hbox, uiControl(y), 1);

	uiSpinboxOnChanged(x, moveX, w);
	uiSpinboxOnChanged(y, moveY, w);
	uiWindowPosition(w, &curX, &curY);
	uiSpinboxSetValue(x, curX);
	uiSpinboxSetValue(y, curY);

	return page15;
}
