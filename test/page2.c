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
		uiBoxDelete(mainBox, 1);
		uiTabInsertPageBefore(mainTab, 0, uiControl(page1));
		uiButtonSetText(b, moveOutText);
		return;
	}
	uiTabDeletePage(mainTab, 0);
	uiBoxAppend(mainBox, uiControl(page1), 1);
	uiButtonSetText(b, moveBackText);
	moveBack = 1;
}

static void openAnotherWindow(uiButton *b, void *data)
{
	uiControlShow(uiControl(uiNewWindow("Another Window", 100, 100, data != NULL)));
}

#define SHED(method, Method) \
	static void method ## Control(uiButton *b, void *data) \
	{ \
		uiControl ## Method(uiControl(data)); \
	}
SHED(enable, Enable)
SHED(disable, Disable)

uiBox *makePage2(void)
{
	uiBox *page2;
	uiBox *hbox;
	uiButton *button;
	uiBox *nestedBox;
	uiBox *innerhbox;
	uiBox *innerhbox2;
	uiBox *innerhbox3;
	uiTab *disabledTab;

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

	hbox = newHorizontalBox();
	uiBoxAppend(hbox, uiControl(uiNewLabel("Label Alignment Test")), 0);
	button = uiNewButton("Open Menued Window");
	uiButtonOnClicked(button, openAnotherWindow, button);
	uiBoxAppend(hbox, uiControl(button), 0);
	button = uiNewButton("Open Menuless Window");
	uiButtonOnClicked(button, openAnotherWindow, NULL);
	uiBoxAppend(hbox, uiControl(button), 0);
	uiBoxAppend(page2, uiControl(hbox), 0);

	nestedBox = newHorizontalBox();
	innerhbox = newHorizontalBox();
	uiBoxAppend(innerhbox, uiControl(uiNewButton("These")), 0);
	button = uiNewButton("buttons");
	uiControlDisable(uiControl(button));
	uiBoxAppend(innerhbox, uiControl(button), 0);
	uiBoxAppend(nestedBox, uiControl(innerhbox), 0);
	innerhbox = newHorizontalBox();
	uiBoxAppend(innerhbox, uiControl(uiNewButton("are")), 0);
	innerhbox2 = newHorizontalBox();
	button = uiNewButton("in");
	uiControlDisable(uiControl(button));
	uiBoxAppend(innerhbox2, uiControl(button), 0);
	uiBoxAppend(innerhbox, uiControl(innerhbox2), 0);
	uiBoxAppend(nestedBox, uiControl(innerhbox), 0);
	innerhbox = newHorizontalBox();
	innerhbox2 = newHorizontalBox();
	uiBoxAppend(innerhbox2, uiControl(uiNewButton("nested")), 0);
	innerhbox3 = newHorizontalBox();
	button = uiNewButton("boxes");
	uiControlDisable(uiControl(button));
	uiBoxAppend(innerhbox3, uiControl(button), 0);
	uiBoxAppend(innerhbox2, uiControl(innerhbox3), 0);
	uiBoxAppend(innerhbox, uiControl(innerhbox2), 0);
	uiBoxAppend(nestedBox, uiControl(innerhbox), 0);
	uiBoxAppend(page2, uiControl(nestedBox), 0);

	hbox = newHorizontalBox();
	button = uiNewButton("Enable Nested Box");
	uiButtonOnClicked(button, enableControl, nestedBox);
	uiBoxAppend(hbox, uiControl(button), 0);
	button = uiNewButton("Disable Nested Box");
	uiButtonOnClicked(button, disableControl, nestedBox);
	uiBoxAppend(hbox, uiControl(button), 0);
	uiBoxAppend(page2, uiControl(hbox), 0);

	disabledTab = newTab();
	uiTabAppendPage(disabledTab, "Disabled", uiControl(uiNewButton("Button")));
	uiTabAppendPage(disabledTab, "Tab", uiControl(uiNewLabel("Label")));
	uiControlDisable(uiControl(disabledTab));
	uiBoxAppend(page2, uiControl(disabledTab), 1);

	return page2;
}
