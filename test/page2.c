// 29 april 2015
#include "test.h"

static uiLabel *movingLabel;
static uiBox *movingBoxes[2];
static int movingCurrent;

static void moveLabel(uiButton *b, void *data)
{
	int from, to;

	from = movingCurrent;
	to = 0;
	if (from == 0)
		to = 1;
	uiBoxDelete(movingBoxes[from], 0);
	uiBoxAppend(movingBoxes[to], uiControl(movingLabel), 0);
	movingCurrent = to;
}

static int moveBack;
#define moveOutText "Move Page 1 Out"
#define moveBackText "Move Page 1 Back"

static void movePage1(uiButton *b, void *data)
{
	if (moveBack) {
		// TODO
		return;
	}
	uiTabDeletePage(mainTab, 0);
	uiBoxAppend(mainBox, uiControl(page1), 1);
	uiButtonSetText(b, moveBackText);
	moveBack = 1;
}

uiBox *makePage2(void)
{
	uiBox *page2;
	uiBox *hbox;
	uiButton *button;

	page2 = newVerticalBox();

	hbox = newHorizontalBox();
	button = uiNewButton("Move the Label!");
	uiButtonOnClicked(button, moveLabel, NULL);
	uiBoxAppend(hbox, uiControl(button), 1);
	// have a blank label for space
	uiBoxAppend(hbox, uiControl(uiNewLabel("")), 1);
	uiBoxAppend(page2, uiControl(hbox), 0);

	hbox = newHorizontalBox();
	movingBoxes[0] = newVerticalBox();
	uiBoxAppend(hbox, uiControl(movingBoxes[0]), 1);
	movingBoxes[1] = newVerticalBox();
	uiBoxAppend(hbox, uiControl(movingBoxes[1]), 1);
	uiBoxAppend(page2, uiControl(hbox), 0);

	movingCurrent = 0;
	movingLabel = uiNewLabel("This label moves!");
	uiBoxAppend(movingBoxes[movingCurrent], uiControl(movingLabel), 0);

	hbox = newHorizontalBox();
	button = uiNewButton(moveOutText);
	uiButtonOnClicked(button, movePage1, NULL);
	uiBoxAppend(hbox, uiControl(button), 0);
	uiBoxAppend(page2, uiControl(hbox), 0);
	moveBack = 0;

	return page2;
}
