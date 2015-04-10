// 7 april 2015

/*
This file assumes that you have imported <Cocoa/Cocoa.h> and "ui.h" beforehand. It provides API-specific functions for interfacing with foreign controls on Mac OS X.
*/

#ifndef __UI_UI_DARWIN_H__
#define __UI_UI_DARWIN_H__

// uiDarwinNewControl() creates a new uiControl with the given Cocoa control inside.
// The first parameter should come from [RealControlType class].
// The two scrollView parameters allow placing scrollbars on the new control.
// Your control must call uiDarwinControlFreeWhenAppropriate() on the returned uiControl in its -[viewDidMoveToSuperview] method.
// If it returns a value other than NO, then the uiControl has been freed and you should set references to it to NULL.
extern uiControl *uiDarwinNewControl(Class class, BOOL inScrollView, BOOL scrollViewHasBorder);
extern BOOL uiDarwinControlFreeWhenAppropriate(uiControl *c, NSView *newSuperview);

// You can use this function from within your control implementations to return text strings that can be freed with uiTextFree().
extern char *uiDarwinNSStringToText(NSString *);

struct uiSizingSys {
	// this structure currently left blank
};

#endif
