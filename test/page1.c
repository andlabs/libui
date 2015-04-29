// 29 april 2015
#include "test.h"

static uiEntry *entry;
static uiCheckbox *spaced;

uiBox *makePage1(void)
{
	uiBox *page1;
	uiButton *getButton, *setButton;
	uiBox *hbox;
	uiBox *testBox;
	uiLabel *label;

	page1 = newVerticalBox();

	entry = uiNewEntry();
	uiBoxAppend(page1, uiControl(entry), 0);

	spaced = uiNewCheckbox("Spaced");
	label = uiNewLabel("Label");

	hbox = newHorizontalBox();
	getButton = uiNewButton("Get Window Text");
	setButton = uiNewButton("Set Window Text");
	uiBoxAppend(hbox, uiControl(getButton), 1);
	uiBoxAppend(hbox, uiControl(setButton), 1);
	uiBoxAppend(page1, uiControl(hbox), 0);

	hbox = newHorizontalBox();
	getButton = uiNewButton("Get Button Text");
	setButton = uiNewButton("Set Button Text");
	uiBoxAppend(hbox, uiControl(getButton), 1);
	uiBoxAppend(hbox, uiControl(setButton), 1);
	uiBoxAppend(page1, uiControl(hbox), 0);

	hbox = newHorizontalBox();
	getButton = uiNewButton("Get Checkbox Text");
	setButton = uiNewButton("Set Checkbox Text");
	uiBoxAppend(hbox, uiControl(getButton), 1);
	uiBoxAppend(hbox, uiControl(setButton), 1);
	uiBoxAppend(page1, uiControl(hbox), 0);

	hbox = newHorizontalBox();
	getButton = uiNewButton("Get Label Text");
	setButton = uiNewButton("Set Label Text");
	uiBoxAppend(hbox, uiControl(getButton), 1);
	uiBoxAppend(hbox, uiControl(setButton), 1);
	uiBoxAppend(page1, uiControl(hbox), 0);

	hbox = newHorizontalBox();
	uiBoxAppend(hbox, uiControl(spaced), 1);
	getButton = uiNewButton("On");
	uiBoxAppend(hbox, uiControl(getButton), 0);
	getButton = uiNewButton("Off");
	uiBoxAppend(hbox, uiControl(getButton), 0);
	getButton = uiNewButton("Show");
	uiBoxAppend(hbox, uiControl(getButton), 0);
	uiBoxAppend(page1, uiControl(hbox), 0);

	testBox = newHorizontalBox();
	setButton = uiNewButton("Button");
	uiBoxAppend(testBox, uiControl(setButton), 1);
	getButton = uiNewButton("Show");
	uiBoxAppend(testBox, uiControl(getButton), 0);
	getButton = uiNewButton("Hide");
	uiBoxAppend(testBox, uiControl(getButton), 0);
	getButton = uiNewButton("Enable");
	uiBoxAppend(testBox, uiControl(getButton), 0);
	getButton = uiNewButton("Disable");
	uiBoxAppend(testBox, uiControl(getButton), 0);
	uiBoxAppend(page1, uiControl(testBox), 0);

	hbox = newHorizontalBox();
	getButton = uiNewButton("Show Box");
	uiBoxAppend(hbox, uiControl(getButton), 1);
	getButton = uiNewButton("Hide Box");
	uiBoxAppend(hbox, uiControl(getButton), 1);
	getButton = uiNewButton("Enable Box");
	uiBoxAppend(hbox, uiControl(getButton), 1);
	getButton = uiNewButton("Disable Box");
	uiBoxAppend(hbox, uiControl(getButton), 1);
	uiBoxAppend(page1, uiControl(hbox), 0);

	uiBoxAppend(page1, uiControl(label), 0);

	return page1;
}
