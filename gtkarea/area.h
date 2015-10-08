// 4 september 2015
#define GLIB_VERSION_MIN_REQUIRED GLIB_VERSION_2_32
#define GLIB_VERSION_MAX_ALLOWED GLIB_VERSION_2_32
#define GDK_VERSION_MIN_REQUIRED GDK_VERSION_3_4
#define GDK_VERSION_MAX_ALLOWED GDK_VERSION_3_4
#include <gtk/gtk.h>
#include <stdint.h>
#include <stddef.h>


extern GType areaWidget_get_type(void);

#include "ui.h"
#include "uipriv.h"

extern GtkWidget *newArea(uiAreaHandler *ah);
extern void areaUpdateScroll(GtkWidget *area);

