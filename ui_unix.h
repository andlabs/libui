// 7 april 2015

/*
This file assumes that you have included <gtk/gtk.h> and "ui.h" beforehand. It provides API-specific functions for interfacing with foreign controls on Unix systems that use GTK+ to provide their UI (currently all except Mac OS X).
*/

#ifndef __UI_UI_UNIX_H__
#define __UI_UI_UNIX_H__

// uiUnixNewControl() creates a new uiControl with the given GTK+ control inside.
// The first three parameters are used for creating the control itself and are the same as passed to g_object_newv().
// The three scrolledWindow parameters allow placing scrollbars on the new control.
// The data parameter can be accessed with uiUnixControlData().
extern uiControl *uiUnixNewControl(GType type, guint nConstructParams, GParameter *constructParams, gboolean inScrolledWindow, gboolean needsViewport, gboolean scrolledWindowHasBorder, void *data);
extern void *uiUnixControlData(uiControl *c);

#endif
