// 25 november 2015
#include <stdio.h>
#include "../ui.h"
int onClosing(uiWindow *w, void *data)
{
	printf("in closing\n");
	uiQuit();
	return 1;
}
int main(void)
{
	uiInitOptions o;
	uiWindow *w;
	uiButton *btn;
	if (uiInit(&o) != NULL) return 1;
	w = uiNewWindow("Hello from C",
		320, 240, 0);
	uiWindowOnClosing(w, onClosing, NULL);
	btn = uiNewButton("Hello from C");
	uiWindowSetChild(w, uiControl(btn));
	uiWindowSetMargined(w, 1);
	uiControlShow(uiControl(w));
	uiMain();
	printf("after main\n");
	uiUninit();
	printf("out\n");
}
