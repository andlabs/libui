// 12 january 2020

/*
This file assumes that you have included TODO_insert_appropriate_Haiku_headers and "ui.h" beforehand. It provides API-specific functions for interfacing with foreign controls in Haiku.
*/

#ifndef uiprivIncludeGuard_ui_haiku_h
#define uiprivIncludeGuard_ui_haiku_h

#ifdef __cplusplus
extern "C" {
#endif

struct uiControlOSVtable {
	size_t Size;
	void *(*Handle)(uiControl *c, void *implData);
};

// TODO return BHandler instead? and NSResponder on Darwin? this depends on how the implementation of this pans out over time
uiprivExtern void *uiHaikuControlHandle(uiControl *c);

#ifdef __cplusplus
}
#endif

#endif
