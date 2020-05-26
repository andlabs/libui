// 7 april 2015

/*
This file assumes that you have imported <Cocoa/Cocoa.h> and "ui.h" beforehand. It provides API-specific functions for interfacing with foreign controls on Mac OS X.
*/

#ifndef uiprivIncludeGuard_ui_darwin_h
#define uiprivIncludeGuard_ui_darwin_h

#ifdef __cplusplus
extern "C" {
#endif

struct uiControlOSVtable {
	size_t Size;
	id (*Handle)(uiControl *c, void *implData);
	// TODO provide events for handle creation and destruction
};

uiprivExtern id uiDarwinControlHandle(uiControl *c);

#ifdef __cplusplus
}
#endif

#endif
