// 22 may 2015
#include "test.h"

static void openFile(uiButton *b, void *data)
{
	char *fn;

	fn = uiOpenFile();
	if (fn == NULL)
		uiLabelSetText(uiLabel(data), "(cancelled)");
	else {
		uiLabelSetText(uiLabel(data), fn);
		uiFreeText(fn);
	}
}

uiBox *makePage5(void)
{
	uiBox *page5;
	uiBox *hbox;
	uiButton *button;
	uiLabel *label;

	page5 = newVerticalBox();

	// TODO label should not be stretchy - figure out how to autosize it on text change
#define D(n, f) \
	hbox = newHorizontalBox(); \
	button = uiNewButton(n); \
	label = uiNewLabel(""); \
	uiButtonOnClicked(button, f, label); \
	uiBoxAppend(hbox, uiControl(button), 0); \
	uiBoxAppend(hbox, uiControl(label), 1); \
	uiBoxAppend(page5, uiControl(hbox), 0);

	D("Open File", openFile);

	return page5;
}
