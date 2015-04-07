// 7 april 2015

/*
This file assumes that you have included <gtk/gtk.h> and "ui.h" beforehand. It provides API-specific functions for interfacing with foreign controls on Unix systems that use GTK+ to provide their UI (currently all except Mac OS X).
*/

#ifndef __UI_UI_UNIX_H__
#define __UI_UI_UNIX_H__

// uiUnixNewControl() creates a new uiControl with the given GTK+ control inside.
// The first parameter is the type of the control, as passed to the first argument of g_object_new().
// The three scrolledWindow parameters allow placing scrollbars on the new control.
// The data parameter can be accessed with uiUnixControlData().
// The firstProperty parameter and beyond allow passing construct properties to the new control, as with g_object_new(); end this list with NULL.
extern uiControl *uiUnixNewControl(GType type, gboolean inScrolledWindow, gboolean needsViewport, gboolean scrolledWindowHasBorder, void *data, const char *firstProperty, ...);
extern void *uiUnixControlData(uiControl *c);

#endif
