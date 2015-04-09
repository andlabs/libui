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

/*TODO
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
*/

int main(int argc, char *argv[])
{
	uiInitError *err;
	uiWindow *w;
	uiControl *mainStack;
	uiControl *buttonStack;
	uiControl *getButton, *setButton;

	err = uiInit(NULL);
	if (err != NULL) {
		fprintf(stderr, "error initializing ui: %s\n", uiInitErrorMessage(err));
		uiInitErrorFree(err);
		return 1;
	}

	w = uiNewWindow("Hello", 320, 240);
	uiWindowOnClosing(w, onClosing, NULL);

	mainStack = uiNewVerticalStack();
	uiWindowSetChild(w, mainStack);

	e = uiTextEntryNew();
	uiStackAdd(mainStack, e, 0);

	buttonStack = uiNewHorizontalStack();
	getButton = uiNewButton("Get Window Text");
//TODO	uiButtonOnClicked(getButton, getWindowText, w);
	setButton = uiNewButton("Set Window Text");
//TODO	uiButtonOnClicked(setButton, setWindowText, w);
	uiStackAdd(buttonStack, getButton, 1);
	uiStackAdd(buttonStack, setButton, 1);
	uiStackAdd(mainStack, buttonStack, 0);

	buttonStack = uiNewHorizontalStack();
//TODO	getButton = uiNewButton("Get Button Text");
	uiButtonOnClicked(getButton, getButtonText, getButton);
//TODO	setButton = uiNewButton("Set Button Text");
	uiButtonOnClicked(setButton, setButtonText, getButton);
	uiStackAdd(buttonStack, getButton, 1);
	uiStackAdd(buttonStack, setButton, 1);
	uiStackAdd(mainStack, buttonStack, 0);

	uiWindowShow(w);
	uiMain();
	printf("after uiMain()\n");
	return 0;
}
