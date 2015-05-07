// 7 april 2015

/*
This file assumes that you have imported <Cocoa/Cocoa.h> and "ui.h" beforehand. It provides API-specific functions for interfacing with foreign controls on Mac OS X.
*/

#ifndef __UI_UI_DARWIN_H__
#define __UI_UI_DARWIN_H__

// uiDarwinMakeControl() initializes the given uiControl with the given Cocoa control inside.
// The second parameter should come from [RealControlType class].
// The two scrollView parameters allow placing scrollbars on the new control.
// The two onDestroy parameters define a function and its parameter to call when the widget is destroyed.
extern void uiDarwinMakeControl(uiControl *c, Class class, BOOL inScrollView, BOOL scrollViewHasBorder, void (*onDestroy)(void *), void *onDestroyData);

// You can use this function from within your control implementations to return text strings that can be freed with uiFreeText().
extern char *uiDarwinNSStringToText(NSString *);

struct uiSizingSys {
	// this structure currently left blank
};

struct uiControlSysFuncParams {
	int Func;
};

enum {
	// These should enable and disable the uiControl while preserving the user enable/disable setting.
	// These are needed because enabling and disabling of views on OS X is available on a view-by-view basis, and is not transitive to subviews by default as a result.
	uiDarwinSysFuncContainerEnable,
	uiDarwinSysFuncContainerDisable,
};

#endif
