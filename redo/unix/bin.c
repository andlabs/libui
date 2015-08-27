// 30 june 2015
#include "uipriv_unix.h"

// This is a uiControl wrapper a la GtkBin.
// It serves the function of tabPage on Windows: it allows uiWindow and uiTab to give their children a real uiControl as a parent while not screwing with the internal GtkWidget structure of those uiControls.
// It also provides margins.

struct bin {
	uiControl c;
	GtkWidget *widget;
	GtkContainer *container;
	GtkBox *box;		// GtkBin is abstract and none of the implementations seem adequate (GtkFrame is the closest but eh)
	uiControl *child;
	int margined;
};

uiDefineControlType(bin, binType, struct bin)

static uintptr_t binHandle(uiControl *c)
{
	struct bin *b = (struct bin *) c;

	return (uintptr_t) (b->widget);
}

uiControl *newBin(void)
{
	struct bin *b;

	b = (struct bin *) uiNewControl(binType());

	b->widget = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
	b->container = GTK_CONTAINER(b->widget);
	b->box = GTK_BOX(b->widget);
	uiUnixMakeSingleWidgetControl(uiControl(b), b->widget);

	// a uiBox is theoretically used in a context where this shouldn't be necessary but because of uiWindow we'll do it anyway
	gtk_widget_set_hexpand(b->widget, TRUE);
	gtk_widget_set_halign(b->widget, GTK_ALIGN_FILL);
	gtk_widget_set_vexpand(b->widget, TRUE);
	gtk_widget_set_valign(b->widget, GTK_ALIGN_FILL);

	uiControl(b)->Handle = binHandle;

	return uiControl(b);
}

void binSetChild(uiControl *c, uiControl *child)
{
	struct bin *b = (struct bin *) c;
	GtkWidget *childWidget;

	if (b->child != NULL)
		uiControlSetParent(b->child, NULL);
	b->child = child;
	if (b->child != NULL) {
		uiControlSetParent(b->child, uiControl(b));
		childWidget = GTK_WIDGET(uiControlHandle(b->child));
		gtk_widget_set_hexpand(childWidget, TRUE);
		gtk_widget_set_halign(childWidget, GTK_ALIGN_FILL);
		gtk_widget_set_vexpand(childWidget, TRUE);
		gtk_widget_set_valign(childWidget, GTK_ALIGN_FILL);
	}
}

int binMargined(uiControl *c)
{
	struct bin *b = (struct bin *) c;

	return b->margined;
}

void binSetMargined(uiControl *c, int margined)
{
	struct bin *b = (struct bin *) c;

	b->margined = margined;
	if (b->margined)
		gtk_container_set_border_width(b->container, gtkXMargin);
	else
		gtk_container_set_border_width(b->container, 0);
}
