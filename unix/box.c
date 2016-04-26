// 7 april 2015
#include "uipriv_unix.h"

// TODO clean this up

struct uiBox {
	uiUnixControl c;
	GtkWidget *widget;
	GtkContainer *container;
	GtkBox *box;
	struct ptrArray *controls;			// TODO switch to GArray
	int vertical;
	int padded;
	GtkSizeGroup *stretchygroup;		// ensures all stretchy controls have the same size
};

uiUnixControlAllDefaultsExceptDestroy(uiBox)

static void uiBoxDestroy(uiControl *c)
{
	uiBox *b = uiBox(c);
	struct child *bc;

	while (b->controls->len != 0) {
		bc = ptrArrayIndex(b->controls, struct child *, 0);
		childDestroy(bc);
		ptrArrayDelete(b->controls, 0);
	}
	ptrArrayDestroy(b->controls);
	// kill the size group
	g_object_unref(b->stretchygroup);
	// and then ourselves
	g_object_unref(b->widget);
	uiFreeControl(uiControl(b));
}

#define isStretchy(bc) childFlag(bc)

void uiBoxAppend(uiBox *b, uiControl *c, int stretchy)
{
	struct child *bc;
	GtkWidget *widget;

	bc = newChild(c, uiControl(b), b->container);
	childSetFlag(bc, stretchy);
	widget = childWidget(bc);
	if (isStretchy(bc)) {
		if (b->vertical) {
			gtk_widget_set_vexpand(widget, TRUE);
			gtk_widget_set_valign(widget, GTK_ALIGN_FILL);
		} else {
			gtk_widget_set_hexpand(widget, TRUE);
			gtk_widget_set_halign(widget, GTK_ALIGN_FILL);
		}
		gtk_size_group_add_widget(b->stretchygroup, widget);
	} else
		if (b->vertical)
			gtk_widget_set_vexpand(widget, FALSE);
		else
			gtk_widget_set_hexpand(widget, FALSE);
	// TODO make the other dimension fill
	ptrArrayAppend(b->controls, bc);
}

void uiBoxDelete(uiBox *b, uintmax_t index)
{
	struct child *bc;

	bc = ptrArrayIndex(b->controls, struct child *, index);
	ptrArrayDelete(b->controls, index);
	if (isStretchy(bc))
		gtk_size_group_remove_widget(b->stretchygroup, childWidget(bc));
	childRemove(bc);
}

int uiBoxPadded(uiBox *b)
{
	return b->padded;
}

void uiBoxSetPadded(uiBox *b, int padded)
{
	b->padded = padded;
	if (b->padded)
		if (b->vertical)
			gtk_box_set_spacing(b->box, gtkYPadding);
		else
			gtk_box_set_spacing(b->box, gtkXPadding);
	else
		gtk_box_set_spacing(b->box, 0);
}

static uiBox *finishNewBox(GtkOrientation orientation)
{
	uiBox *b;

	uiUnixNewControl(uiBox, b);

	b->widget = gtk_box_new(orientation, 0);
	b->container = GTK_CONTAINER(b->widget);
	b->box = GTK_BOX(b->widget);

	b->vertical = orientation == GTK_ORIENTATION_VERTICAL;

	if (b->vertical)
		b->stretchygroup = gtk_size_group_new(GTK_SIZE_GROUP_VERTICAL);
	else
		b->stretchygroup = gtk_size_group_new(GTK_SIZE_GROUP_HORIZONTAL);

	b->controls = newPtrArray();

	return b;
}

uiBox *uiNewHorizontalBox(void)
{
	return finishNewBox(GTK_ORIENTATION_HORIZONTAL);
}

uiBox *uiNewVerticalBox(void)
{
	return finishNewBox(GTK_ORIENTATION_VERTICAL);
}
