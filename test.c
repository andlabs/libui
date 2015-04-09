// 6 april 2015
#include "ui.h"
#include <stdio.h>

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

uiControl *stacks[5];
uiControl *spaced;

static void setSpaced(uiControl *c, void *data)
{
	// TODO
	printf("toggled %d\n", uiCheckboxChecked(spaced));
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

int main(int argc, char *argv[])
{
	uiInitError *err;
	uiWindow *w;
	uiControl *getButton, *setButton;

	err = uiInit(NULL);
	if (err != NULL) {
		fprintf(stderr, "error initializing ui: %s\n", uiInitErrorMessage(err));
		uiInitErrorFree(err);
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
	uiCheckboxOnToggled(spaced, setSpaced, NULL);

	stacks[3] = uiNewHorizontalStack();
	getButton = uiNewButton("Get Checkbox Text");
	uiButtonOnClicked(getButton, getCheckboxText, spaced);
	setButton = uiNewButton("Set Checkbox Text");
	uiButtonOnClicked(setButton, setCheckboxText, spaced);
	uiStackAdd(stacks[3], getButton, 1);
	uiStackAdd(stacks[3], setButton, 1);
	uiStackAdd(stacks[0], stacks[3], 0);

	stacks[4] = uiNewHorizontalStack();
	uiStackAdd(stacks[4], spaced, 1);
	getButton = uiNewButton("On");
	uiButtonOnClicked(getButton, forceSpacedOn, NULL);
	setButton = uiNewButton("Off");
	uiButtonOnClicked(setButton, forceSpacedOff, NULL);
	uiStackAdd(stacks[4], getButton, 0);
	uiStackAdd(stacks[4], setButton, 0);
	uiStackAdd(stacks[0], stacks[4], 0);

	uiWindowShow(w);
	uiMain();
	printf("after uiMain()\n");
	return 0;
}
