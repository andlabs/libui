// 7 april 2015

/*
This file assumes that you have imported <Cocoa/Cocoa.h> and "ui.h" beforehand. It provides API-specific functions for interfacing with foreign controls on Mac OS X.
*/

#ifndef __UI_UI_DARWIN_H__
#define __UI_UI_DARWIN_H__

// uiDarwinNewControl() initializes the given uiControl with the given Cocoa control inside.
// The second parameter should come from [RealControlType class].
// The two scrollView parameters allow placing scrollbars on the new control.
// The two onDestroy parameters define a function and its parameter to call when the widget is destroyed.
// Your control must call uiDarwinControlFreeWhenAppropriate() on the returned uiControl in its -[viewDidMoveToSuperview] method.
// If it returns a value other than NO, then the uiControl has been freed and you should set references to it to NULL.
extern void uiDarwinNewControl(uiControl *c, Class class, BOOL inScrollView, BOOL scrollViewHasBorder);
extern BOOL uiDarwinControlFreeWhenAppropriate(uiControl *c, NSView *newSuperview, void (*onDestroy)(void *), void *onDestroyData);

// You can use this function from within your control implementations to return text strings that can be freed with uiTextFree().
extern char *uiDarwinNSStringToText(NSString *);

struct uiSizingSys {
	// this structure currently left blank
};

#endif
