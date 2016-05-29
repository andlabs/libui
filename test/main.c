// 22 april 2015
#include "test.h"

// TODOs
// - blank page affects menus negatively on Windows

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

int onShouldQuit(void *data)
{
	printf("in onShouldQuit()\n");
	if (uiMenuItemChecked(shouldQuitItem)) {
		uiControlDestroy(uiControl(data));
		return 1;
	}
	return 0;
}

uiBox *mainBox;
uiTab *mainTab;

uiBox *(*newhbox)(void);
uiBox *(*newvbox)(void);

int main(int argc, char *argv[])
{
	uiInitOptions o;
	int i;
	const char *err;
	uiWindow *w;
	uiBox *page2, *page3, *page4, *page5;
	uiBox *page6, *page7, *page8, *page9, *page10;
	uiBox *page11, *page12, *page13;
	uiTab *outerTab;
	uiTab *innerTab;
	int nomenus = 0;
	int startspaced = 0;

	newhbox = uiNewHorizontalBox;
	newvbox = uiNewVerticalBox;

	memset(&o, 0, sizeof (uiInitOptions));
	for (i = 1; i < argc; i++)
		if (strcmp(argv[i], "nomenus") == 0)
			nomenus = 1;
		else if (strcmp(argv[i], "startspaced") == 0)
			startspaced = 1;
		else if (strcmp(argv[i], "swaphv") == 0) {
			newhbox = uiNewVerticalBox;
			newvbox = uiNewHorizontalBox;
		} else {
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

	uiOnShouldQuit(onShouldQuit, w);

	mainBox = newHorizontalBox();
	uiWindowSetChild(w, uiControl(mainBox));

	outerTab = newTab();
	uiBoxAppend(mainBox, uiControl(outerTab), 1);

	mainTab = newTab();
	uiTabAppend(outerTab, "Pages 1-5", uiControl(mainTab));

	// page 1 uses page 2's uiGroup
	page2 = makePage2();

	makePage1(w);
	uiTabAppend(mainTab, "Page 1", uiControl(page1));

	uiTabAppend(mainTab, "Page 2", uiControl(page2));

	uiTabAppend(mainTab, "Empty Page", uiControl(uiNewHorizontalBox()));

	page3 = makePage3();
	uiTabAppend(mainTab, "Page 3", uiControl(page3));

	page4 = makePage4();
	uiTabAppend(mainTab, "Page 4", uiControl(page4));

	page5 = makePage5(w);
	uiTabAppend(mainTab, "Page 5", uiControl(page5));

	innerTab = newTab();
	uiTabAppend(outerTab, "Pages 6-10", uiControl(innerTab));

	page6 = makePage6();
	uiTabAppend(innerTab, "Page 6", uiControl(page6));

	page7 = makePage7();
	uiTabAppend(innerTab, "Page 7", uiControl(page7));

	page8 = makePage8();
	uiTabAppend(innerTab, "Page 8", uiControl(page8));

	page9 = makePage9();
	uiTabAppend(innerTab, "Page 9", uiControl(page9));

	page10 = makePage10();
	uiTabAppend(innerTab, "Page 10", uiControl(page10));

	innerTab = newTab();
	uiTabAppend(outerTab, "Pages 11-?", uiControl(innerTab));

//	page11 = makePage11();
//	uiTabAppend(innerTab, "Page 11", uiControl(page11));

	page12 = makePage12();
	uiTabAppend(innerTab, "Page 12", uiControl(page12));

	page13 = makePage13();
	uiTabAppend(innerTab, "Page 13", uiControl(page13));

	if (startspaced)
		setSpaced(1);

	uiControlShow(uiControl(w));
	uiMain();
	printf("after uiMain()\n");
	uiUninit();
	printf("after uiUninit()\n");
	return 0;
}
