// 24 may 2020
#include "test_windows.h"

Test(WindowHasHandleFromStart)
{
	uiWindow *a;

	a = uiNewWindow();
	if (uiWindowsControlHandle(uiControl(a)) == NULL)
		TestErrorf("uiWindowsControlHandle(brand new uiWindow) is NULL; should not be");
	uiControlFree(uiControl(a));
}

Test(InitialWindowTitleIsEmptyString_OSLevel)
{
	uiWindow *w;
	HWND hwnd;
	uint16_t *title;

	w = uiNewWindow();
	hwnd = uiWindowsControlHandle(uiControl(w));
	title = testGetWindowText(hwnd);
	if (!utf16equal(title, testUTF16Empty))
		utf16diffError("brand new uiWindow has wrong title", title, testUTF16Empty);
	free(title);
	uiControlFree(uiControl(w));
}

static void testSetWindowTitleImplFull(const char *file, long line, const char *title, const uint16_t *want)
{
	uiWindow *w;
	HWND hwnd;
	uint16_t *got;

	w = uiNewWindow();
	uiWindowSetTitle(w, title);
	hwnd = uiWindowsControlHandle(uiControl(w));
	got = testGetWindowText(hwnd);
	if (!utf16equal(got, want))
		utf16diffErrorFull(file, line, "GetWindowTextW() reported wrong title after uiWindowSetTitle()", got, want);
	free(got);
	uiControlFree(uiControl(w));
}

#define testSetWindowTitleImpl(title, want) testSetWindowTitleImplFull(__FILE__, __LINE__, title, want)

Test(SetWindowTitle_OSLevel_Empty)
{
	testSetWindowTitleImpl(testUTF8Empty, testUTF16Empty);
}

Test(SetWindowTitle_OSLevel_ASCIIOnly)
{
	testSetWindowTitleImpl(testUTF8ASCIIOnly, testUTF16ASCIIOnly);
}

Test(SetWindowTitle_OSLevel_WithTwoByte)
{
	testSetWindowTitleImpl(testUTF8WithTwoByte, testUTF16WithTwoByte);
}

Test(SetWindowTitle_OSLevel_WithThreeByte)
{
	testSetWindowTitleImpl(testUTF8WithThreeByte, testUTF16WithThreeByte);
}

Test(SetWindowTitle_OSLevel_WithFourByte)
{
	testSetWindowTitleImpl(testUTF8WithFourByte, testUTF16WithFourByte);
}

Test(SetWindowTitle_OSLevel_Combined)
{
	testSetWindowTitleImpl(testUTF8Combined, testUTF16Combined);
}

Test(SetWindowTitle_OSLevel_Invalid)
{
	testSetWindowTitleImpl(testUTF8InvalidInput, testUTF16InvalidOutput);
}

Test(WindowsCannotSetWindowControlPos)
{
	uiWindow *w;
	RECT r;
	void *ctx;

	w = uiNewWindow();

	ctx = beginCheckProgrammerError("cannot set a uiWindow as the child of another uiControl");
	r.left = 0;
	r.top = 0;
	r.right = 640;
	r.bottom = 480;
	uiWindowsControlSetControlPos(uiControl(w), &r);
	endCheckProgrammerError(ctx);

	uiControlFree(uiControl(w));
}
