// 7 april 2015

/*
This file assumes that you have included <gtk/gtk.h> and "ui.h" beforehand. It provides API-specific functions for interfacing with foreign controls on Unix systems that use GTK+ to provide their UI (currently all except Mac OS X).
*/

#ifndef __UI_UI_UNIX_H__
#define __UI_UI_UNIX_H__

// TODO write this comment
_UI_EXTERN void uiUnixMakeSingleWidgetControl(uiControl *c, GtkWidget *widget);

// uiUnixStrdupText() takes the given string and produces a copy of it suitable for being freed by uiFreeText().
extern char *uiUnixStrdupText(const char *);

struct uiSizingSys {
	// this structure currently left blank
};

#endif
