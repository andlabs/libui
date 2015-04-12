// 6 april 2015
#include "ui.h"
#include <stdio.h>
#include <string.h>

int onClosing(uiWindow *w, void *data)
{
	printf("in closing!\n");
	uiQuit();
	return 1;
}

uiControl *e;

static void getWindowText(uiControl *b, void *data)
{
	char *text;

	text = uiWindowTitle((uiWindow *) data);
	uiEntrySetText(e, text);
	uiFreeText(text);
}

static void setWindowText(uiControl *b, void *data)
{
	char *text;

	text = uiEntryText(e);
	uiWindowSetTitle((uiWindow *) data, text);
	uiFreeText(text);
}

static void getButtonText(uiControl *b, void *data)
{
	char *text;

	text = uiButtonText((uiControl *) data);
	uiEntrySetText(e, text);
	uiFreeText(text);
}

static void setButtonText(uiControl *b, void *data)
{
	char *text;

	text = uiEntryText(e);
	uiButtonSetText((uiControl *) data, text);
	uiFreeText(text);
}

static void getCheckboxText(uiControl *b, void *data)
{
	char *text;

	text = uiCheckboxText((uiControl *) data);
	uiEntrySetText(e, text);
	uiFreeText(text);
}

static void setCheckboxText(uiControl *b, void *data)
{
	char *text;

	text = uiEntryText(e);
	uiCheckboxSetText((uiControl *) data, text);
	uiFreeText(text);
}

uiWindow *w;
#define nStacks 7
uiControl *stacks[nStacks];
uiControl *spaced;

static void setSpaced(int spaced)
{
	int i;

	uiWindowSetMargined(w, spaced);
	for (i = 0; i < nStacks; i++)
		uiStackSetPadded(stacks[i], spaced);
}

static void toggleSpaced(uiControl *c, void *data)
{
	int s = uiCheckboxChecked(spaced);

	printf("toggled %d\n", s);
	setSpaced(s);
}

// these will also be used to test if setting checks will trigger events
static void forceSpacedOn(uiControl *c, void *data)
{
	uiCheckboxSetChecked(spaced, 1);
}

static void forceSpacedOff(uiControl *c, void *data)
{
	uiCheckboxSetChecked(spaced, 0);
}

static void showSpaced(uiControl *c, void *data)
{
	char msg[] = { 'm', ' ', '0', ' ', 'p', ' ', '0', '\0' };

	if (uiWindowMargined(w))
		msg[2] = '1';
	if (uiStackPadded(stacks[0]))
		msg[6] = '1';
	uiEntrySetText(e, msg);
}

static void showControl(uiControl *c, void *data)
{
	uiControlShow((uiControl *) data);
}

static void hideControl(uiControl *c, void *data)
{
	uiControlHide((uiControl *) data);
}

static void enableControl(uiControl *c, void *data)
{
	uiControlEnable((uiControl *) data);
}

static void disableControl(uiControl *c, void *data)
{
	uiControlDisable((uiControl *) data);
}

static void getLabelText(uiControl *b, void *data)
{
	char *text;

	text = uiLabelText((uiControl *) data);
	uiEntrySetText(e, text);
	uiFreeText(text);
}

static void setLabelText(uiControl *b, void *data)
{
	char *text;

	text = uiEntryText(e);
	uiLabelSetText((uiControl *) data, text);
	uiFreeText(text);
}

int main(int argc, char *argv[])
{
	uiInitOptions o;
	int i;
	const char *err;
	uiControl *getButton, *setButton;
	uiControl *label;

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

	stacks[0] = uiNewVerticalStack();
	uiWindowSetChild(w, stacks[0]);

	e = uiNewEntry();
	uiStackAdd(stacks[0], e, 0);

	stacks[1] = uiNewHorizontalStack();
	getButton = uiNewButton("Get Window Text");
	uiButtonOnClicked(getButton, getWindowText, w);
	setButton = uiNewButton("Set Window Text");
	uiButtonOnClicked(setButton, setWindowText, w);
	uiStackAdd(stacks[1], getButton, 1);
	uiStackAdd(stacks[1], setButton, 1);
	uiStackAdd(stacks[0], stacks[1], 0);

	stacks[2] = uiNewHorizontalStack();
	getButton = uiNewButton("Get Button Text");
	uiButtonOnClicked(getButton, getButtonText, getButton);
	setButton = uiNewButton("Set Button Text");
	uiButtonOnClicked(setButton, setButtonText, getButton);
	uiStackAdd(stacks[2], getButton, 1);
	uiStackAdd(stacks[2], setButton, 1);
	uiStackAdd(stacks[0], stacks[2], 0);

	// this will also be used to make sure tab stops work properly when inserted out of creation order, especially on Windows
	spaced = uiNewCheckbox("Spaced");
	uiCheckboxOnToggled(spaced, toggleSpaced, NULL);

	stacks[3] = uiNewHorizontalStack();
	getButton = uiNewButton("Get Checkbox Text");
	uiButtonOnClicked(getButton, getCheckboxText, spaced);
	setButton = uiNewButton("Set Checkbox Text");
	uiButtonOnClicked(setButton, setCheckboxText, spaced);
	uiStackAdd(stacks[3], getButton, 1);
	uiStackAdd(stacks[3], setButton, 1);
	uiStackAdd(stacks[0], stacks[3], 0);

	label = uiNewLabel("Label");
/*
	stacks[i] = uiNewHorizontalStack();
	getButton = uiNewButton("Get Label Text");
	uiButtonOnClicked(getButton, getLabelText, label);
	setButton = uiNewButton("Set Label Text");
	uiButtonOnClicked(setButton, setLabelText, label);
	uiStackAdd(stacks[i], getButton, 1);
	uiStackAdd(stacks[i], setButton, 1);
	uiStackAdd(stacks[0], stacks[i], 0);
	i++;
*/
	stacks[4] = uiNewHorizontalStack();
	uiStackAdd(stacks[4], spaced, 1);
	getButton = uiNewButton("On");
	uiButtonOnClicked(getButton, forceSpacedOn, NULL);
	setButton = uiNewButton("Off");
	uiButtonOnClicked(setButton, forceSpacedOff, NULL);
	uiStackAdd(stacks[4], getButton, 0);
	uiStackAdd(stacks[4], setButton, 0);
	setButton = uiNewButton("Show");
	uiButtonOnClicked(setButton, showSpaced, NULL);
	uiStackAdd(stacks[4], setButton, 0);
	uiStackAdd(stacks[0], stacks[4], 0);

	stacks[5] = uiNewHorizontalStack();
	getButton = uiNewButton("Button");
	uiStackAdd(stacks[5], getButton, 1);
	setButton = uiNewButton("Show");
	uiButtonOnClicked(setButton, showControl, getButton);
	uiStackAdd(stacks[5], setButton, 0);
	setButton = uiNewButton("Hide");
	uiButtonOnClicked(setButton, hideControl, getButton);
	uiStackAdd(stacks[5], setButton, 0);
	setButton = uiNewButton("Enable");
	uiButtonOnClicked(setButton, enableControl, getButton);
	uiStackAdd(stacks[5], setButton, 0);
	setButton = uiNewButton("Disable");
	uiButtonOnClicked(setButton, disableControl, getButton);
	uiStackAdd(stacks[5], setButton, 0);
	uiStackAdd(stacks[0], stacks[5], 0);

	stacks[6] = uiNewHorizontalStack();
	setButton = uiNewButton("Show Stack");
	uiButtonOnClicked(setButton, showControl, stacks[5]);
	uiStackAdd(stacks[6], setButton, 1);
	setButton = uiNewButton("Hide Stack");
	uiButtonOnClicked(setButton, hideControl, stacks[5]);
	uiStackAdd(stacks[6], setButton, 1);
	setButton = uiNewButton("Enable Stack");
	uiButtonOnClicked(setButton, enableControl, stacks[5]);
	uiStackAdd(stacks[6], setButton, 1);
	setButton = uiNewButton("Disable Stack");
	uiButtonOnClicked(setButton, disableControl, stacks[5]);
	uiStackAdd(stacks[6], setButton, 1);
	uiStackAdd(stacks[0], stacks[6], 0);

	uiStackAdd(stacks[0], label, 0);

	uiWindowShow(w);
	uiMain();
	printf("after uiMain()\n");
	return 0;
}
