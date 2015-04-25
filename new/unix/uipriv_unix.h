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

// menu.c
extern GtkWidget *makeMenubar(uiWindow *);

// oscontainer.c
#define uipOSContainerType (uipOSContainer_get_type())
#define uipOSContainer(obj) (G_TYPE_CHECK_INSTANCE_CAST((obj), uipOSContainerType, uipOSContainer))
#define uipIsOSContainer(obj) (G_TYPE_CHECK_INSTANCE_TYPE((obj), uipOSContainerType))
#define uipOSContainerClass(class) (G_TYPE_CHECK_CLASS_CAST((class), uipOSContainerType, uipOSContainerClass))
#define uipIsOSContainerClass(class) (G_TYPE_CHECK_CLASS_TYPE((class), uipOSContainer))
#define uipGetParentClass(obj) (G_TYPE_INSTANCE_GET_CLASS((obj), uipOSContainerType, uipOSContainerClass))
typedef struct uipOSContainer uipOSContainer;
typedef struct uipOSContainerClass uipOSContainerClass;
struct uipOSContainer {
	GtkContainer parent_instance;
	// TODO make these private
	uiControl *mainControl;
	GPtrArray *children;		// for forall()
	intmax_t marginLeft;
	intmax_t marginTop;
	intmax_t marginRight;
	intmax_t marginBottom;
	gboolean canDestroy;
};
struct uipOSContainerClass {
	GtkContainerClass parent_class;
};
extern GType uipOSContainer_get_type(void);
extern GtkWidget *newOSContainer(void);
extern void osContainerDestroy(uipOSContainer *);
extern void osContainerSetMainControl(uipOSContainer *, uiControl *);
extern void osContainerSetMargins(uipOSContainer *, intmax_t, intmax_t, intmax_t, intmax_t);
