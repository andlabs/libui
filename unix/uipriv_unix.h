// 22 april 2015
#define GLIB_VERSION_MIN_REQUIRED GLIB_VERSION_2_40
#define GLIB_VERSION_MAX_ALLOWED GLIB_VERSION_2_40
#define GDK_VERSION_MIN_REQUIRED GDK_VERSION_3_10
#define GDK_VERSION_MAX_ALLOWED GDK_VERSION_3_10
#include <gtk/gtk.h>
#include <math.h>
#include <dlfcn.h>		// see future.c
#include <langinfo.h>
#include <string.h>
#include <stdlib.h>
#include <inttypes.h>
#include "../ui.h"
#include "../ui_unix.h"
#include "../common/uipriv.h"
