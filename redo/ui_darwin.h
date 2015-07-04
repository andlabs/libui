// 7 april 2015

/*
This file assumes that you have imported <Cocoa/Cocoa.h> and "ui.h" beforehand. It provides API-specific functions for interfacing with foreign controls on Mac OS X.
*/

#ifndef __UI_UI_DARWIN_H__
#define __UI_UI_DARWIN_H__

// TODO document
_UI_EXTERN void uiDarwinMakeSingleViewControl(uiControl *, NSView *);

// You can use this function from within your control implementations to return text strings that can be freed with uiFreeText().
_UI_EXTERN char *uiDarwinNSStringToText(NSString *);

struct uiSizingSys {
	// this structure currently left blank
};

#endif
