#include <stdio.h>
#include <string.h>
#include "../../ui.h"

static uiWindow *mainwin;

static int onClosing(uiWindow *w, void *data)
{
	uiControlDestroy(uiControl(mainwin));
	uiQuit();
	return 0;
}

static int shouldQuit(void *data)
{
	uiControlDestroy(uiControl(mainwin));
	return 1;
}

int main(void)
{
	uiInitOptions o;
	const char *err;
	uiMenu *menu;
	uiMenuItem *item;
	uiBox *box;
	uiLabel *label;

	memset(&o, 0, sizeof (uiInitOptions));
	err = uiInit(&o);
	if (err != NULL) {
		fprintf(stderr, "error initializing ui: %s\n", err);
		uiFreeInitError(err);
		return 1;
	}

	menu = uiNewMenu("File");
	item = uiMenuAppendItem(menu, "Item");
	item = uiMenuAppendQuitItem(menu);
	uiOnShouldQuit(shouldQuit, NULL);

	mainwin = uiNewWindow("Window", 640, 480, 1);
	uiWindowSetMargined(mainwin, 1);
	uiWindowOnClosing(mainwin, onClosing, NULL);

	box = uiNewVerticalBox();
	uiBoxSetPadded(box, 1);
	uiWindowSetChild(mainwin, uiControl(box));
	
	label = uiNewLabel("Hello, World!");
	uiBoxAppend(box, uiControl(label), 0);

	uiControlShow(uiControl(mainwin));
	uiMain();
	uiUninit();
	return 0;
}
