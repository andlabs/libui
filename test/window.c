// 24 may 2020
#include "test.h"

Test(CannotMakeWindowAChild)
{
	uiWindow *a, *b;
	void *ctx;

	a = uiNewWindow();
	b = uiNewWindow();

	ctx = beginCheckProgrammerError("cannot set a uiWindow as the child of another uiControl");
	uiControlSetParent(a, b);
	// TODO this should not be necessary
	uiControlSetParent(a, NULL);
	endCheckProgrammerError(ctx);

	uiControlFree(uiControl(b));
	uiControlFree(uiControl(a));
}

Test(InitialWindowTitleIsEmptyString)
{
	uiWindow *w;
	const char *title;

	w = uiNewWindow();
	title = uiWindowTitle(w);
	// TODO have a utf8cmp()
	if (*title != 0)
		// TODO have a diffUTF8
		TestErrorf("brand new uiWindow has wrong title:" diff("%s"),
			title, "(empty string)");
	uiControlFree(uiControl(w));
}

// TODO check that SetTitle works, and also that it sanitizes
// TODO for all the above, check that the underlying title was also set appropriately
