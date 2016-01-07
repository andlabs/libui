// 22 december 2015
#include "test.h"

static void onListFonts(uiButton *b, void *data)
{
	uiDrawFontFamilies *ff;
	char *this;
	uintmax_t i, n;

	uiMultilineEntrySetText(uiMultilineEntry(data), "");
	ff = uiDrawListFontFamilies();
	n = uiDrawFontFamiliesNumFamilies(ff);
	for (i = 0; i < n; i++) {
		this = uiDrawFontFamiliesFamily(ff, i);
		uiMultilineEntryAppend(uiMultilineEntry(data), this);
		uiMultilineEntryAppend(uiMultilineEntry(data), "\n");
		uiFreeText(this);
	}
	uiDrawFreeFontFamilies(ff);
}

uiBox *makePage8(void)
{
	uiBox *page8;
	uiGroup *group;
	uiBox *vbox;
	uiMultilineEntry *me;
	uiButton *button;

	page8 = newHorizontalBox();

	group = newGroup("Font Families");
	uiBoxAppend(page8, uiControl(group), 1);

	vbox = newVerticalBox();
	uiGroupSetChild(group, uiControl(vbox));

	me = uiNewMultilineEntry();
	uiBoxAppend(vbox, uiControl(me), 1);

	button = uiNewButton("List Font Families");
	uiButtonOnClicked(button, onListFonts, me);
	uiBoxAppend(vbox, uiControl(button), 0);

	return page8;
}
