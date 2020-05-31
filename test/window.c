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
	if (!utf8equal(title, testUTF8Empty))
		utf8diffError("brand new uiWindow has wrong title", title, testUTF8Empty);
	uiControlFree(uiControl(w));
}

static void testSetWindowTitleImplFull(const char *file, long line, const char *title, const char *want)
{
	uiWindow *w;
	const char *got;

	w = uiNewWindow();
	uiWindowSetTitle(w, title);
	got = uiWindowTitle(w);
	if (!utf8equal(got, want))
		utf8diffErrorFull(file, line, "uiWindowTitle() reported wrong title after uiWindowSetTitle()", got, want);
	uiControlFree(uiControl(w));
}

#define testSetWindowTitleImpl(title, want) testSetWindowTitleImplFull(__FILE__, __LINE__, title, want)

Test(SetWindowTitle_Empty)
{
	testSetWindowTitleImpl(testUTF8Empty, testUTF8Empty);
}

Test(SetWindowTitle_ASCIIOnly)
{
	testSetWindowTitleImpl(testUTF8ASCIIOnly, testUTF8ASCIIOnly);
}

Test(SetWindowTitle_WithTwoByte)
{
	testSetWindowTitleImpl(testUTF8WithTwoByte, testUTF8WithTwoByte);
}

Test(SetWindowTitle_WithThreeByte)
{
	testSetWindowTitleImpl(testUTF8WithThreeByte, testUTF8WithThreeByte);
}

Test(SetWindowTitle_WithFourByte)
{
	testSetWindowTitleImpl(testUTF8WithFourByte, testUTF8WithFourByte);
}

Test(SetWindowTitle_Combined)
{
	testSetWindowTitleImpl(testUTF8Combined, testUTF8Combined);
}

Test(SetWindowTitle_Invalid)
{
	testSetWindowTitleImpl(testUTF8InvalidInput, testUTF8InvalidOutput);
}

Test(WindowTitleWIthNULLWindowIsProgrammerError)
{
	void *ctx;

	ctx = beginCheckProgrammerError("uiWindowTitle(): invalid null pointer for uiWindow");
	uiWindowTitle(NULL);
	endCheckProgrammerError(ctx);
}

Test(SetWindowTitleWIthNULLWindowIsProgrammerError)
{
	void *ctx;

	ctx = beginCheckProgrammerError("uiWindowSetTitle(): invalid null pointer for uiWindow");
	uiWindowSetTitle(NULL, NULL);
	endCheckProgrammerError(ctx);
}

Test(SetWindowTitleWIthNULLTitleIsProgrammerError)
{
	uiWindow *w;
	void *ctx;

	w = uiNewWindow();
	ctx = beginCheckProgrammerError("uiWindowSetTitle(): invalid null pointer for title");
	uiWindowSetTitle(w, NULL);
	endCheckProgrammerError(ctx);
	uiControlFree(uiControl(w));
}
