#include "../../ui.h"
#include <string.h>
#include <stdlib.h>

int onClosing(uiWindow *w, void *data)
{
	uiQuit();
	return 1;
}

int main(void)
{
	uiInitOptions o;
	uiWindow *w;
	uiScrollView *s;
	uiBox *box;

	memset(&o, 0, sizeof (uiInitOptions));
	if (uiInit(&o) != NULL)
		abort();

	w = uiNewWindow("Scrolling", 320, 240, 0);
	s = uiNewScrollView();
	box = uiNewVerticalBox();
	uiScrollViewSetChild(s, uiControl(box));
	uiScrollViewSetMargined(s, 1);
	uiBoxSetPadded(box, 1);

	for (int i = 0; i < 32; i++) {
		uiButton *btn = uiNewButton("Button");
		uiBoxAppend(box, uiControl(btn), 0);
	}

	uiWindowOnClosing(w, onClosing, NULL);
	uiWindowSetChild(w, uiControl(s));
	uiControlShow(uiControl(w));
	uiMain();
	return 0;
}
