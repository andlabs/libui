// 7 april 2015

/*
This file assumes that you have included <gtk/gtk.h> and "ui.h" beforehand. It provides API-specific functions for interfacing with foreign controls on Unix systems that use GTK+ to provide their UI (currently all except Mac OS X and Haiku).
*/

#ifndef uiprivIncludeGuard_ui_unix_h
#define uiprivIncludeGuard_ui_unix_h

#ifdef __cplusplus
extern "C" {
#endif

struct uiControlOSVtable {
	size_t Size;
	GtkWidget *(*Handle)(uiControl *c, void *implData);
};

uiprivExtern GtkWidget *uiUnixControlHandle(uiControl *c);

#ifdef __cplusplus
}
#endif

#endif
