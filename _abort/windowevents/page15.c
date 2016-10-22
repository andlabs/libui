static uiSpinbox *x, *y;

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

uiBox *makePage15(uiWindow *w)
{
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
}
