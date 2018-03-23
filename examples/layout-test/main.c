#include <stdio.h>
#include <string.h>
#include "../../ui.h"

int main(void)
{
	uiInitOptions options;
	const char *err;

	memset(&options, 0, sizeof (uiInitOptions));
	err = uiInit(&options);
	if (err != NULL) {
		fprintf(stderr, "error initializing libui: %s", err);
		uiFreeInitError(err);
		return 1;
	}

	uiWindow *mainwin = uiNewWindow("Layout test", 640, 480, 1);

    uiFixed *fixed = uiNewFixed();

    uiWindowSetChild(mainwin, uiControl(fixed));

    uiLabel *label = uiNewLabel("Hello");

    uiFixedAppend(fixed, uiControl(label), 150, 200);

    uiButton *button = uiNewButton("Button");

    uiFixedAppend(fixed, uiControl(button), 50, 50);

	uiControlShow(uiControl(mainwin));


    int width, height;

    uiNaturalSize(uiControl(label), &width, &height);
    printf("Width: %d, Height: %d\n", width, height);

        uiNaturalSize(uiControl(button), &width, &height);
    printf("Width: %d, Height: %d\n", width, height);

	uiMain();
	return 0;
}