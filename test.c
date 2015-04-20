// 6 april 2015
#include "ui.h"
#include <stdio.h>
#include <string.h>

// TODO convert to using the new conversion macros

int onClosing(uiWindow *w, void *data)
{
	printf("in closing!\n");
	uiQuit();
	return 1;
}

uiEntry *e;

static void getWindowText(uiButton *b, void *data)
{
	char *text;

	text = uiWindowTitle(uiWindow(data));
	uiEntrySetText(e, text);
	uiFreeText(text);
}

static void setWindowText(uiButton *b, void *data)
{
	char *text;

	text = uiEntryText(e);
	uiWindowSetTitle(uiWindow(data), text);
	uiFreeText(text);
}

static void getButtonText(uiButton *b, void *data)
{
	char *text;

	text = uiButtonText(uiButton(data));
	uiEntrySetText(e, text);
	uiFreeText(text);
}

static void setButtonText(uiButton *b, void *data)
{
	char *text;

	text = uiEntryText(e);
	uiButtonSetText(uiButton(data), text);
	uiFreeText(text);
}

static void getCheckboxText(uiButton *b, void *data)
{
	char *text;

	text = uiCheckboxText(uiCheckbox(data));
	uiEntrySetText(e, text);
	uiFreeText(text);
}

static void setCheckboxText(uiButton *b, void *data)
{
	char *text;

	text = uiEntryText(e);
	uiCheckboxSetText(uiCheckbox(data), text);
	uiFreeText(text);
}

uiWindow *w;
#define nBoxes 13
uiBox *boxes[nBoxes];
uiCheckbox *spaced;

static void setSpaced(int spaced)
{
	int i;

	uiWindowSetMargined(w, spaced);
	for (i = 0; i < nBoxes; i++)
		uiBoxSetPadded(boxes[i], spaced);
}

static void toggleSpaced(uiCheckbox *c, void *data)
{
	int s;

	s = uiCheckboxChecked(spaced);
	printf("toggled %d\n", s);
	setSpaced(s);
}

// these will also be used to test if setting checks will trigger events
static void forceSpacedOn(uiButton *b, void *data)
{
	uiCheckboxSetChecked(spaced, 1);
}

static void forceSpacedOff(uiButton *b, void *data)
{
	uiCheckboxSetChecked(spaced, 0);
}

static void showSpaced(uiButton *b, void *data)
{
	char msg[] = { 'm', ' ', '0', ' ', 'p', ' ', '0', '\0' };

	if (uiWindowMargined(w))
		msg[2] = '1';
	if (uiBoxPadded(boxes[0]))
		msg[6] = '1';
	uiEntrySetText(e, msg);
}

static void showControl(uiButton *b, void *data)
{
	uiControlShow(uiControl(data));
}

static void hideControl(uiButton *b, void *data)
{
	uiControlHide(uiControl(data));
}

static void enableControl(uiButton *b, void *data)
{
	uiControlEnable(uiControl(data));
}

static void disableControl(uiButton *b, void *data)
{
	uiControlDisable(uiControl(data));
}

static void getLabelText(uiButton *b, void *data)
{
	char *text;

	text = uiLabelText(uiLabel(data));
	uiEntrySetText(e, text);
	uiFreeText(text);
}

static void setLabelText(uiButton *b, void *data)
{
	char *text;

	text = uiEntryText(e);
	uiLabelSetText(uiLabel(data), text);
	uiFreeText(text);
}

uiBox *firstBox;
uiBox *secondBox;
uiLabel *movingLabel;

static void moveToFirst(uiButton *b, void *data)
{
	uiBoxDelete(secondBox, 1);
	uiBoxAppend(firstBox, uiControl(movingLabel), 1);
}

static void moveToSecond(uiButton *b, void *data)
{
	uiBoxDelete(firstBox, 1);
	uiBoxAppend(secondBox, uiControl(movingLabel), 1);
}

uiBox *mainBox;
uiBox *page1box;
uiTab *tab;

void movePage1Out(uiButton *b, void *data)
{
	uiTabDeletePage(tab, 0);
	uiBoxAppend(mainBox, uiControl(page1box), 1);
}

void addPage1Back(uiButton *b, void *data)
{
	uiBoxDelete(mainBox, 1);
	uiTabAddPage(tab, "Page 1", uiControl(page1box));
}

int main(int argc, char *argv[])
{
	uiInitOptions o;
	int i;
	const char *err;
	uiButton *getButton, *setButton;
	uiLabel *label;
	int page2box;

	memset(&o, 0, sizeof (uiInitOptions));
	for (i = 1; i < argc; i++)
		if (strcmp(argv[i], "leaks") == 0)
			o.debugLogAllocations = 1;
		else {
			fprintf(stderr, "%s: unrecognized option %s\n", argv[0], argv[i]);
			return 1;
		}

	err = uiInit(&o);
	if (err != NULL) {
		fprintf(stderr, "error initializing ui: %s\n", err);
		uiFreeInitError(err);
		return 1;
	}

	w = uiNewWindow("Hello", 320, 240);
	uiWindowOnClosing(w, onClosing, NULL);

	boxes[0] = uiNewVerticalBox();
	page1box = boxes[0];

	e = uiNewEntry();
	uiBoxAppend(boxes[0], uiControl(e), 0);

	i = 1;

	boxes[i] = uiNewHorizontalBox();
	getButton = uiNewButton("Get Window Text");
	uiButtonOnClicked(getButton, getWindowText, w);
	setButton = uiNewButton("Set Window Text");
	uiButtonOnClicked(setButton, setWindowText, w);
	uiBoxAppend(boxes[i], uiControl(getButton), 1);
	uiBoxAppend(boxes[i], uiControl(setButton), 1);
	uiBoxAppend(boxes[0], uiControl(boxes[i]), 0);
	i++;

	boxes[i] = uiNewHorizontalBox();
	getButton = uiNewButton("Get Button Text");
	uiButtonOnClicked(getButton, getButtonText, getButton);
	setButton = uiNewButton("Set Button Text");
	uiButtonOnClicked(setButton, setButtonText, getButton);
	uiBoxAppend(boxes[i], uiControl(getButton), 1);
	uiBoxAppend(boxes[i], uiControl(setButton), 1);
	uiBoxAppend(boxes[0], uiControl(boxes[i]), 0);
	i++;

	// this will also be used to make sure tab stops work properly when inserted out of creation order, especially on Windows
	spaced = uiNewCheckbox("Spaced");
	uiCheckboxOnToggled(spaced, toggleSpaced, NULL);

	boxes[i] = uiNewHorizontalBox();
	getButton = uiNewButton("Get Checkbox Text");
	uiButtonOnClicked(getButton, getCheckboxText, spaced);
	setButton = uiNewButton("Set Checkbox Text");
	uiButtonOnClicked(setButton, setCheckboxText, spaced);
	uiBoxAppend(boxes[i], uiControl(getButton), 1);
	uiBoxAppend(boxes[i], uiControl(setButton), 1);
	uiBoxAppend(boxes[0], uiControl(boxes[i]), 0);
	i++;

	label = uiNewLabel("Label");

	boxes[i] = uiNewHorizontalBox();
	getButton = uiNewButton("Get Label Text");
	uiButtonOnClicked(getButton, getLabelText, label);
	setButton = uiNewButton("Set Label Text");
	uiButtonOnClicked(setButton, setLabelText, label);
	uiBoxAppend(boxes[i], uiControl(getButton), 1);
	uiBoxAppend(boxes[i], uiControl(setButton), 1);
	uiBoxAppend(boxes[0], uiControl(boxes[i]), 0);
	i++;

	boxes[i] = uiNewHorizontalBox();
	uiBoxAppend(boxes[i], uiControl(spaced), 1);
	getButton = uiNewButton("On");
	uiButtonOnClicked(getButton, forceSpacedOn, NULL);
	setButton = uiNewButton("Off");
	uiButtonOnClicked(setButton, forceSpacedOff, NULL);
	uiBoxAppend(boxes[i], uiControl(getButton), 0);
	uiBoxAppend(boxes[i], uiControl(setButton), 0);
	setButton = uiNewButton("Show");
	uiButtonOnClicked(setButton, showSpaced, NULL);
	uiBoxAppend(boxes[i], uiControl(setButton), 0);
	uiBoxAppend(boxes[0], uiControl(boxes[i]), 0);
	i++;

	boxes[i] = uiNewHorizontalBox();
	getButton = uiNewButton("Button");
	uiBoxAppend(boxes[i], uiControl(getButton), 1);
	setButton = uiNewButton("Show");
	uiButtonOnClicked(setButton, showControl, getButton);
	uiBoxAppend(boxes[i], uiControl(setButton), 0);
	setButton = uiNewButton("Hide");
	uiButtonOnClicked(setButton, hideControl, getButton);
	uiBoxAppend(boxes[i], uiControl(setButton), 0);
	setButton = uiNewButton("Enable");
	uiButtonOnClicked(setButton, enableControl, getButton);
	uiBoxAppend(boxes[i], uiControl(setButton), 0);
	setButton = uiNewButton("Disable");
	uiButtonOnClicked(setButton, disableControl, getButton);
	uiBoxAppend(boxes[i], uiControl(setButton), 0);
	uiBoxAppend(boxes[0], uiControl(boxes[i]), 0);
	i++;

	boxes[i] = uiNewHorizontalBox();
	setButton = uiNewButton("Show Box");
	uiButtonOnClicked(setButton, showControl, boxes[i - 1]);
	uiBoxAppend(boxes[i], uiControl(setButton), 1);
	setButton = uiNewButton("Hide Box");
	uiButtonOnClicked(setButton, hideControl, boxes[i - 1]);
	uiBoxAppend(boxes[i], uiControl(setButton), 1);
	setButton = uiNewButton("Enable Box");
	uiButtonOnClicked(setButton, enableControl, boxes[i - 1]);
	uiBoxAppend(boxes[i], uiControl(setButton), 1);
	setButton = uiNewButton("Disable Box");
	uiButtonOnClicked(setButton, disableControl, boxes[i - 1]);
	uiBoxAppend(boxes[i], uiControl(setButton), 1);
	uiBoxAppend(boxes[0], uiControl(boxes[i]), 0);
	i++;

	uiBoxAppend(boxes[0], uiControl(label), 0);

	tab = uiNewTab();
	uiTabAddPage(tab, "Page 1", uiControl(boxes[0]));
//TODO	uiTabAddPage(tab, "Page 1", uiControl(uiNewVerticalBox()));

	page2box = i;
	boxes[i] = uiNewVerticalBox();
	uiTabAddPage(tab, "Page 2", uiControl(boxes[i]));
	i++;

	boxes[i] = uiNewHorizontalBox();
	firstBox = boxes[i];
	getButton = uiNewButton("Move Here");
	uiButtonOnClicked(getButton, moveToFirst, NULL);
	uiBoxAppend(boxes[i], uiControl(getButton), 0);
	movingLabel = uiNewLabel("This label moves!");
	uiBoxAppend(boxes[i], uiControl(movingLabel), 1);
	uiBoxAppend(boxes[page2box], uiControl(boxes[i]), 0);
	i++;

	boxes[i] = uiNewHorizontalBox();
	secondBox = boxes[i];
	getButton = uiNewButton("Move Here");
	uiButtonOnClicked(getButton, moveToSecond, NULL);
	uiBoxAppend(boxes[i], uiControl(getButton), 0);
	uiBoxAppend(boxes[page2box], uiControl(boxes[i]), 0);
	i++;

	boxes[i] = uiNewHorizontalBox();
	getButton = uiNewButton("Move Page 1 Out");
	uiButtonOnClicked(getButton, movePage1Out, NULL);
	setButton = uiNewButton("Add Page 1 Back");
	uiButtonOnClicked(setButton, addPage1Back, NULL);
	uiBoxAppend(boxes[i], uiControl(getButton), 0);
	uiBoxAppend(boxes[i], uiControl(setButton), 0);
	uiBoxAppend(boxes[page2box], uiControl(boxes[i]), 0);
	i++;

	boxes[i] = uiNewHorizontalBox();
	mainBox = boxes[i];
	uiBoxAppend(boxes[i], uiControl(tab), 1);
	uiWindowSetChild(w, uiControl(mainBox));
	i++;

	if (i != nBoxes) {
		fprintf(stderr, "forgot to update nBoxes (expected %d)\n", i);
		return 1;
	}
	uiWindowShow(w);
	uiMain();
	printf("after uiMain()\n");
	return 0;
}
