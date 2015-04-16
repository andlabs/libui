// 6 april 2015
#define GLIB_VERSION_MIN_REQUIRED GLIB_VERSION_2_32
#define GLIB_VERSION_MAX_ALLOWED GLIB_VERSION_2_32
#define GDK_VERSION_MIN_REQUIRED GDK_VERSION_3_4
#define GDK_VERSION_MAX_ALLOWED GDK_VERSION_3_4
#include <gtk/gtk.h>
#include "../uipriv.h"
#include "../ui_unix.h"

#define gtkXMargin 12
#define gtkYMargin 12

#define widget(c) uiControlHandle(uiControl(c))
#define WIDGET(c) GTK_WIDGET(widget(c))
