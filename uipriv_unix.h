// 6 april 2015
#define GLIB_VERSION_MIN_REQUIRED GLIB_VERSION_2_32
#define GLIB_VERSION_MAX_ALLOWED GLIB_VERSION_2_32
#define GDK_VERSION_MIN_REQUIRED GDK_VERSION_3_4
#define GDK_VERSION_MAX_ALLOWED GDK_VERSION_3_4
#include <gtk/gtk.h>
#include "uipriv.h"
#include "ui_unix.h"

// container_unix.c
#define uiContainerType (uiContainer_get_type())
#define uiContainer(obj) (G_TYPE_CHECK_INSTANCE_CAST((obj), uiContainerType, uiContainer))
#define uiIsContainer(obj) (G_TYPE_CHECK_INSTANCE_TYPE((obj), uiContainerType))
#define uiContainerClass(class) (G_TYPE_CHECK_CLASS_CAST((class), uiContainerType, uiContainerClass))
#define uiIsContainerClass(class) (G_TYPE_CHECK_CLASS_TYPE((class), uiContainer))
#define uiGetContainerClass(obj) (G_TYPE_INSTANCE_GET_CLASS((obj), uiContainerType, uiContainerClass))
typedef struct uiContainer uiContainer;
typedef struct uiContainerClass uiContainerClass;
struct uiContainer {
	GtkContainer parent_instance;
	// this is what triggers the resizing of all the children
	uiControl *child;
	// these are the actual children widgets of the container as far as GTK+ is concerned
	GPtrArray *children;		// for forall()
};
struct uiContainerClass {
	GtkContainerClass parent_class;
};
extern GType uiContainer_get_type(void);
extern GtkWidget *newContainer(void);
