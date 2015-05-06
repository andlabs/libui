// 22 april 2015
#include "test.h"

void die(const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	fprintf(stderr, "[test program] ");
	vfprintf(stderr, fmt, ap);
	fprintf(stderr, "\n");
	va_end(ap);
	abort();
}

int onClosing(uiWindow *w, void *data)
{
	printf("in onClosing()\n");
	uiQuit();
	return 1;
}

uiBox *mainBox;
uiTab *mainTab;

int main(int argc, char *argv[])
{
	uiInitOptions o;
	int i;
	const char *err;
	uiWindow *w;
	uiBox *page2;
	int nomenus = 0;

	memset(&o, 0, sizeof (uiInitOptions));
	for (i = 1; i < argc; i++)
		if (strcmp(argv[i], "nomenus") == 0)
			nomenus = 1;
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

	if (!nomenus)
		initMenus();

	w = newWindow("Main Window", 320, 240, 1);
	uiWindowOnClosing(w, onClosing, NULL);
	printf("main window %p\n", w);

	mainBox = newHorizontalBox();
	uiWindowSetChild(w, uiControl(mainBox));

	mainTab = newTab();
	uiBoxAppend(mainBox, uiControl(mainTab), 1);

	makePage1(w);
	uiTabAppendPage(mainTab, "Page 1", uiControl(page1));

	page2 = makePage2();
	uiTabAppendPage(mainTab, "Page 2", uiControl(page2));

	uiTabAppendPage(mainTab, "Empty Page", uiControl(uiNewHorizontalBox()));

	uiControlShow(uiControl(w));
	uiMain();
	printf("after uiMain()\n");
	return 0;
}
