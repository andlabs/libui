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

#define gtkXMargin 12
#define gtkYMargin 12
#define gtkXPadding 12
#define gtkYPadding 6

// menu.c
extern GtkWidget *makeMenubar(uiWindow *);
extern void freeMenubar(GtkWidget *);
extern void uninitMenus(void);

// alloc.c
extern void initAlloc(void);
extern void uninitAlloc(void);

// util.c
extern void setMargined(GtkContainer *, int);

// child.c
extern struct child *newChild(uiControl *child, uiControl *parent, GtkContainer *parentContainer);
extern struct child *newChildWithBox(uiControl *child, uiControl *parent, GtkContainer *parentContainer, int margined);
extern void childRemove(struct child *c);
extern void childDestroy(struct child *c);
extern GtkWidget *childWidget(struct child *c);
extern int childFlag(struct child *c);
extern void childSetFlag(struct child *c, int flag);
extern GtkWidget *childBox(struct child *c);
extern void childSetMargined(struct child *c, int margined);

// draw.c
extern uiDrawContext *newContext(cairo_t *cr, GtkStyleContext *style);
extern void freeContext(uiDrawContext *);

// image.c
/*TODO remove this*/typedef struct uiImage uiImage;
extern cairo_surface_t *imageAppropriateSurface(uiImage *i, GtkWidget *w);

// cellrendererbutton.c
extern GtkCellRenderer *newCellRendererButton(void);

// future.c
extern void loadFutures(void);
extern PangoAttribute *FUTURE_pango_attr_font_features_new(const gchar *features);
extern PangoAttribute *FUTURE_pango_attr_foreground_alpha_new(guint16 alpha);
extern gboolean FUTURE_gtk_widget_path_iter_set_object_name(GtkWidgetPath *path, gint pos, const char *name);

// drawtext.c
extern void fontdescFromPangoFontDescription(PangoFontDescription *pdesc, uiDrawFontDescriptor *uidesc);

// attrstr.c
extern PangoAttrList *attrstrToPangoAttrList(uiDrawTextLayoutParams *p, GPtrArray **backgroundClosures);
extern void invokeBackgroundClosure(GClosure *closure, uiDrawContext *c, uiDrawTextLayout *layout, double x, double y);

// drawtext.c
// TODO get rid of these (for attrstr.c)
#define cairoToPango(cairo) (pango_units_from_double(cairo))
extern const PangoStyle pangoItalics[];
extern const PangoStretch pangoStretches[];

// opentype.c
extern gchar *otfToPangoCSSString(const uiOpenTypeFeatures *otf);
