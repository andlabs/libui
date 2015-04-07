// 6 april 2015
#include "ui.h"
#include <stdio.h>

int onClosing(uiWindow *w, void *data)
{
	printf("in closing!\n");
	uiQuit();
	return 1;
}

void onClicked(uiControl *b, void *data)
{
	// TODO
}

void onClicked2(uiControl *b, void *data)
{
	printf("button clicked!\n");
}

int main(int argc, char *argv[])
{
	uiInitError *err;
	uiWindow *w;
	uiControl *stack;
	uiControl *button, *button2;

	err = uiInit(NULL);
	if (err != NULL) {
		fprintf(stderr, "error initializing ui: %s\n", uiInitErrorMessage(err));
		uiInitErrorFree(err);
		return 1;
	}

	w = uiNewWindow("Hello", 320, 240);
	uiWindowOnClosing(w, onClosing, NULL);

	if (argc > 1)
		stack = uiNewHorizontalStack();
	else
		stack = uiNewVerticalStack();
	uiWindowSetChild(w, stack);

	button2 = uiNewButton("Change Me");
	uiButtonOnClicked(button, onClicked2, NULL);

	button = uiNewButton("Click Me");
	uiButtonOnClicked(button, onClicked, button2);
	uiStackAdd(stack, button, 1);
	uiStackAdd(stack, button2, 0);

	uiWindowShow(w);
	uiMain();
	printf("after uiMain()\n");
	return 0;
}
