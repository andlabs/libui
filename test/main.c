// 22 april 2015
#include "test.h"

void die(const char *fmt, ...)
{
	// TODO
	abort();
}

int onClosing(uiWindow *w, void *data)
{
	printf("in onClosing()\n");
	uiQuit();
	return 1;
}

uiBox *mainBox;

int main(int argc, char *argv[])
{
	uiInitOptions o;
	int i;
	const char *err;
	uiWindow *w;
	uiTab *tab;
	uiBox *page2;

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

	initMenus();

	w = newWindow("Main Window", 320, 240, 1);
	uiWindowOnClosing(w, onClosing, NULL);

	mainBox = newHorizontalBox();
	uiWindowSetChild(w, uiControl(mainBox));

	tab = newTab();
	uiBoxAppend(mainBox, uiControl(tab), 1);

	makePage1(w);
	uiTabAppendPage(tab, "Page 1", uiControl(page1));

	page2 = makePage2();
	uiTabAppendPage(tab, "Page 2", uiControl(page2));

	uiControlShow(uiControl(w));
	uiMain();
	printf("after uiMain()\n");
	return 0;
}
