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

#define uiprivGTKXMargin 12
#define uiprivGTKYMargin 12
#define uiprivGTKXPadding 12
#define uiprivGTKYPadding 6

// menu.c
extern GtkWidget *uiprivMakeMenubar(uiWindow *);
extern void uiprivFreeMenubar(GtkWidget *);
extern void uiprivUninitMenus(void);

// alloc.c
extern void uiprivInitAlloc(void);
extern void uiprivUninitAlloc(void);

// util.c
extern void uiprivSetMargined(GtkContainer *, int);

// child.c
typedef struct uiprivChild uiprivChild;
extern uiprivChild *uiprivNewChild(uiControl *child, uiControl *parent, GtkContainer *parentContainer);
extern uiprivChild *uiprivNewChildWithBox(uiControl *child, uiControl *parent, GtkContainer *parentContainer, int margined);
extern void uiprivChildRemove(uiprivChild *c);
extern void uiprivChildDestroy(uiprivChild *c);
extern GtkWidget *uiprivChildWidget(uiprivChild *c);
extern int uiprivChildFlag(uiprivChild *c);
extern void uiprivChildSetFlag(uiprivChild *c, int flag);
extern GtkWidget *uiprivChildBox(uiprivChild *c);
extern void uiprivChildSetMargined(uiprivChild *c, int margined);

// draw.c
extern uiDrawContext *uiprivNewContext(cairo_t *cr, GtkStyleContext *style);
extern void uiprivFreeContext(uiDrawContext *);

// image.c
extern cairo_surface_t *uiprivImageAppropriateSurface(uiImage *i, GtkWidget *w);

// cellrendererbutton.c
extern GtkCellRenderer *uiprivNewCellRendererButton(void);

// future.c
extern void uiprivLoadFutures(void);
extern PangoAttribute *uiprivFUTURE_pango_attr_font_features_new(const gchar *features);
extern PangoAttribute *uiprivFUTURE_pango_attr_foreground_alpha_new(guint16 alpha);
extern PangoAttribute *uiprivFUTURE_pango_attr_background_alpha_new(guint16 alpha);
extern gboolean uiprivFUTURE_gtk_widget_path_iter_set_object_name(GtkWidgetPath *path, gint pos, const char *name);
