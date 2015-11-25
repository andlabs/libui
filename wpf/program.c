// 24 november 2015
#include <stdio.h>
#include "wpf.h"

static void onClosing(wpfWindow *w, void *data)
{
	printf("exiting\n");
	wpfQuit();
}

int main(void)
{
	wpfWindow *w;

	wpfInit();
	w = wpfNewWindow("Hi", 320, 240);
	wpfWindowOnClosing(w, onClosing, NULL);

	wpfRun();
	printf("out\n");
	return 0;
}
