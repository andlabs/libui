// 6 april 2015
#include "ui.h"
#include <stdio.h>

// #qo pkg-config: gtk+-3.0

int onClosing(uiWindow *w, void *data)
{
	printf("in closing!\n");
	uiQuit();
	return 1;
}

int main(void)
{
	uiInitError *err;
	uiWindow *w;

	err = uiInit(NULL);
	if (err != NULL) {
		fprintf(stderr, "error initializing ui: %s\n", uiInitErrorMessage(err));
		uiInitErrorFree(err);
		return 1;
	}

	w = uiNewWindow("Hello", 320, 240);
	uiWindowOnClosing(w, onClosing, NULL);
	uiWindowShow(w);

	uiMain();
	return 0;
}
