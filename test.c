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
	printf("button clicked!\n");
}

int main(void)
{
	uiInitError *err;
	uiWindow *w;
	uiControl *button;

	err = uiInit(NULL);
	if (err != NULL) {
		fprintf(stderr, "error initializing ui: %s\n", uiInitErrorMessage(err));
		uiInitErrorFree(err);
		return 1;
	}

	w = uiNewWindow("Hello", 320, 240);
	uiWindowOnClosing(w, onClosing, NULL);
	uiWindowShow(w);

	button = uiNewButton("Click Me");
	uiButtonOnClicked(button, onClicked, NULL);
	uiWindowSetChild(w, button);

	uiMain();
	printf("after uiMain()\n");
	return 0;
}
