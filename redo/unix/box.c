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

struct boxControl {
	uiControl *c;
	int stretchy;
};

static void onDestroy(uiBox *b);

uiUnixDefineControlWithOnDestroy(
	uiBox,								// type name
	uiBoxType,							// type function
	onDestroy(this);						// on destroy
)

static void onDestroy(uiBox *b)
{
	struct boxControl *bc;

	// don't chain up to base here; we need to destroy children ourselves first
	while (b->controls->len != 0) {
		bc = ptrArrayIndex(b->controls, struct boxControl *, 0);
		uiControlSetParent(bc->c, NULL);
		uiControlDestroy(bc->c);
		ptrArrayDelete(b->controls, 0);
		uiFree(bc);
	}
	ptrArrayDestroy(b->controls);
	// kill the size group
	g_object_unref(b->stretchygroup);
}

static void boxContainerUpdateState(uiControl *c)
{
	uiBox *b = uiBox(c);
	struct boxControl *bc;
	uintmax_t i;

	for (i = 0; i < b->controls->len; i++) {
		bc = ptrArrayIndex(b->controls, struct boxControl *, i);
		uiControlUpdateState(bc->c);
	}
}

void uiBoxAppend(uiBox *b, uiControl *c, int stretchy)
{
	struct boxControl *bc;
	GtkWidget *widget;

	bc = uiNew(struct boxControl);
	bc->c = c;
	bc->stretchy = stretchy;
	uiControlSetParent(bc->c, uiControl(b));
	widget = GTK_WIDGET(uiControlHandle(bc->c));
	if (bc->stretchy) {
		if (b->vertical) {
			gtk_widget_set_vexpand(widget, TRUE);
			gtk_widget_set_valign(widget, GTK_ALIGN_FILL);
		} else {
			gtk_widget_set_hexpand(widget, TRUE);
			gtk_widget_set_halign(widget, GTK_ALIGN_FILL);
		}
		gtk_size_group_add_widget(b->stretchygroup, widget);
	} else		// TODO undo this all in delete
		if (b->vertical)
			gtk_widget_set_vexpand(widget, FALSE);
		else
			gtk_widget_set_hexpand(widget, FALSE);
	// TODO make the other dimension fill
	ptrArrayAppend(b->controls, bc);
	uiControlQueueResize(uiControl(b));
}

void uiBoxDelete(uiBox *b, uintmax_t index)
{
	struct boxControl *bc;

	bc = ptrArrayIndex(b->controls, struct boxControl *, index);
	ptrArrayDelete(b->controls, index);
	if (bc->stretchy)
		gtk_size_group_remove_widget(b->stretchygroup, GTK_WIDGET(uiControlHandle(bc->c)));
	uiControlSetParent(bc->c, NULL);
	uiFree(bc);
	uiControlQueueResize(uiControl(b));
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
	uiControlQueueResize(uiControl(b));
}

static uiBox *finishNewBox(GtkOrientation orientation)
{
	uiBox *b;

	b = (uiBox *) uiNewControl(uiTypeBox());

	b->widget = gtk_box_new(orientation, 0);
	b->container = GTK_CONTAINER(b->widget);
	b->box = GTK_BOX(b->widget);

	b->vertical = orientation == GTK_ORIENTATION_VERTICAL;

	if (b->vertical)
		b->stretchygroup = gtk_size_group_new(GTK_SIZE_GROUP_VERTICAL);
	else
		b->stretchygroup = gtk_size_group_new(GTK_SIZE_GROUP_HORIZONTAL);

	b->controls = newPtrArray();

	uiUnixMakeSingleWidgetControl(uiControl(b), b->widget);

	uiUnixFinishNewControl(b, uiBox);
	uiControl(b)->ContainerUpdateState = boxContainerUpdateState;

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
