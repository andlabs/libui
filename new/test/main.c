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

int main(int argc, char *argv[])
{
	uiInitOptions o;
	int i;
	const char *err;
	uiWindow *w;

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

	uiWindowShow(newWindow("Second Window", 320, 240, 1));

	uiWindowShow(w);
	uiMain();
	printf("after uiMain()\n");
	return 0;
}
