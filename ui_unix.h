// 7 april 2015

/*
This file assumes that you have included <gtk/gtk.h> and "ui.h" beforehand. It provides API-specific functions for interfacing with foreign controls on Unix systems that use GTK+ to provide their UI (currently all except Mac OS X).
*/

#ifndef __UI_UI_UNIX_H__
#define __UI_UI_UNIX_H__

// uiUnixMakeControl() creates a new uiControl with the given GTK+ control inside, storing it in the uiControl at c.
// The second parameter is the type of the control, as passed to the first argument of g_object_new().
// The two scrolledWindow parameters allow placing scrollbars on the new control.
// The destroy parameters are for a function that should be called when destroying the widget.
// The firstProperty parameter and beyond allow passing construct properties to the new control, as with g_object_new(); end this list with NULL.
_UI_EXTERN void uiUnixMakeControl(uiControl *c, GType type, gboolean inScrolledWindow, gboolean scrolledWindowHasBorder, void (*destroy)(void *), void *onDestroyData, const char *firstProperty, ...);

// uiUnixStrdupText() takes the given string and produces a copy of it suitable for being freed by uiFreeText().
extern char *uiUnixStrdupText(const char *);

struct uiSizingSys {
	// this structure currently left blank
};

#endif
