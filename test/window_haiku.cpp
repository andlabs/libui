// 24 may 2020
#include "test_haiku.hpp"

Test(WindowHasHandleFromStart)
{
	uiWindow *a;

	a = uiNewWindow();
	if (uiHaikuControlHandle(uiControl(a)) == NULL)
		TestErrorf("uiUnixControlHandle(brand new uiWindow) is NULL; should not be");
	uiControlFree(uiControl(a));
}

Test(InitialWindowTitleIsEmptyString_OSLevel)
{
	uiWindow *w;
	BWindow *bw;
	const char *title;

	w = uiNewWindow();
	bw = (BWindow *) uiHaikuControlHandle(uiControl(w));
	title = bw->Title();
	if (!utf8equal(title, testUTF8Empty))
		utf8diffError("brand new uiWindow has wrong title", title, testUTF8Empty);
	uiControlFree(uiControl(w));
}

static void testSetWindowTitleImplFull(const char *file, long line, const char *title, const char *want)
{
	uiWindow *w;
	BWindow *bw;
	const char *got;

	w = uiNewWindow();
	uiWindowSetTitle(w, title);
	bw = (BWindow *) uiHaikuControlHandle(uiControl(w));
	got = bw->Title();
	if (!utf8equal(got, want))
		utf8diffErrorFull(file, line, "gtk_window_get_title() reported wrong title after uiWindowSetTitle()", got, want);
	uiControlFree(uiControl(w));
}

#define testSetWindowTitleImpl(title, want) testSetWindowTitleImplFull(__FILE__, __LINE__, title, want)

Test(SetWindowTitle_OSLevel_Empty)
{
	testSetWindowTitleImpl(testUTF8Empty, testUTF8Empty);
}

Test(SetWindowTitle_OSLevel_ASCIIOnly)
{
	testSetWindowTitleImpl(testUTF8ASCIIOnly, testUTF8ASCIIOnly);
}

Test(SetWindowTitle_OSLevel_WithTwoByte)
{
	testSetWindowTitleImpl(testUTF8WithTwoByte, testUTF8WithTwoByte);
}

Test(SetWindowTitle_OSLevel_WithThreeByte)
{
	testSetWindowTitleImpl(testUTF8WithThreeByte, testUTF8WithThreeByte);
}

Test(SetWindowTitle_OSLevel_WithFourByte)
{
	testSetWindowTitleImpl(testUTF8WithFourByte, testUTF8WithFourByte);
}

Test(SetWindowTitle_OSLevel_Combined)
{
	testSetWindowTitleImpl(testUTF8Combined, testUTF8Combined);
}

Test(SetWindowTitle_OSLevel_Invalid)
{
	testSetWindowTitleImpl(testUTF8InvalidInput, testUTF8InvalidOutput);
}
