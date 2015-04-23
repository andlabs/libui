// 22 april 2015
#define GLIB_VERSION_MIN_REQUIRED GLIB_VERSION_2_32
#define GLIB_VERSION_MAX_ALLOWED GLIB_VERSION_2_32
#define GDK_VERSION_MIN_REQUIRED GDK_VERSION_3_4
#define GDK_VERSION_MAX_ALLOWED GDK_VERSION_3_4
#include <gtk/gtk.h>
#include "../ui.h"
#include "../ui_unix.h"
#include "../uipriv.h"

#define gtkXMargin 12
#define gtkYMargin 12

// text.c
extern char *strdupText(const char *);
