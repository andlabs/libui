// 7 april 2015

/*
This file assumes that you have imported <Cocoa/Cocoa.h> and "ui.h" beforehand. It provides API-specific functions for interfacing with foreign controls on Mac OS X.
*/

#ifndef __UI_UI_DARWIN_H__
#define __UI_UI_DARWIN_H__

// uiDarwinNewControl() creates a new uiControl with the given Cocoa control inside.
// The first parameter should come from [RealControlType class].
// The two scrollView parameters allow placing scrollbars on the new control.
// The data parameter can be accessed with uiDarwinControlData().
extern uiControl *uiDarwinNewControl(Class class, gboolean inScrollView, gboolean scrollViewHasBorder, void *data);
extern void *uiDarwinControlData(uiControl *c);

#endif
