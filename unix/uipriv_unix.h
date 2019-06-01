// 22 april 2015
#define G_LOG_DOMAIN "libui"
#define GLIB_VERSION_MIN_REQUIRED GLIB_VERSION_2_40
#define GLIB_VERSION_MAX_ALLOWED GLIB_VERSION_2_40
#define GDK_VERSION_MIN_REQUIRED GDK_VERSION_3_10
#define GDK_VERSION_MAX_ALLOWED GDK_VERSION_3_10
#include <gtk/gtk.h>
#include <dlfcn.h>		// see future.c
#include <langinfo.h>
#include <inttypes.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>
#define uiprivOSHeader "../ui_unix.h"
#include "../common/uipriv.h"
