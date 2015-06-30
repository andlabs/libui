// 7 april 2015
#include "uipriv_unix.h"

// TODO
// - the move the label box on GTK+ is not sized halfway, or the label below it is not; one of the two

struct box {
	uiBox b;
	GtkWidget *widget;
	GtkBox *box;
	void (*baseCommitDestroy)(uiControl *);
	// TODO switch to GArray
	struct ptrArray *controls;
	int vertical;
	int padded;
};

struct boxControl {
	uiControl *c;
	int stretchy;
};

uiDefineControlType(uiBox, uiTypeBox, struct box)

static void boxCommitDestroy(uiControl *c)
{
	struct box *b = (struct box *) c;
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
	// NOW we can chain up to base
	(*(b->baseCommitDestroy))(uiControl(b));
}

static uintptr_t boxHandle(uiControl *c)
{
	struct box *b = (struct box *) c;

	return (uintptr_t) (b->widget);
}

static void boxContainerUpdateState(uiControl *c)
{
	struct box *b = (struct box *) c;
	struct boxControl *bc;
	uintmax_t i;

	for (i = 0; i < b->controls->len; i++) {
		bc = ptrArrayIndex(b->controls, struct boxControl *, i);
		uiControlUpdateState(bc->c);
	}
}

static void boxAppend(uiBox *ss, uiControl *c, int stretchy)
{
	struct box *b = (struct box *) ss;
	struct boxControl *bc;
	uintmax_t i;
	GtkWidget *widget;
	gboolean hexpand, vexpand;

	bc = uiNew(struct boxControl);
	bc->c = c;
	bc->stretchy = stretchy;
	widget = GTK_WIDGET(uiControlHandle(bc->c));
	hexpand = FALSE;
	vexpand = FALSE;
	if (bc->stretchy)
		if (b->vertical)
			vexpand = TRUE;
		else
			hexpand = TRUE;
	gtk_widget_set_hexpand(widget, hexpand);
	gtk_widget_set_halign(widget, GTK_ALIGN_FILL);
	gtk_widget_set_vexpand(widget, vexpand);
	gtk_widget_set_valign(widget, GTK_ALIGN_FILL);
	uiControlSetParent(bc->c, uiControl(b));
	ptrArrayAppend(b->controls, bc);
	uiControlQueueResize(uiControl(b));
	// TODO when adding boxInsertAt(), we use gtk_box_reorder_child()
}

static void boxDelete(uiBox *ss, uintmax_t index)
{
	struct box *b = (struct box *) ss;
	struct boxControl *bc;
	uiControl *removed;

	bc = ptrArrayIndex(b->controls, struct boxControl *, index);
	removed = bc->c;
	ptrArrayDelete(b->controls, index);
	uiControlSetParent(removed, NULL);
	uiFree(bc);
	uiControlQueueResize(uiControl(b));
}

static int boxPadded(uiBox *ss)
{
	struct box *b = (struct box *) ss;

	return b->padded;
}

static void boxSetPadded(uiBox *ss, int padded)
{
	struct box *b = (struct box *) ss;

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

uiBox *finishNewBox(GtkOrientation orientation)
{
	struct box *b;

	b = (struct box *) uiNewControl(uiTypeBox());

	b->widget = gtk_box_new(orientation, 0);
	b->box = GTK_BOX(b->widget);
	uiUnixMakeSingleWidgetControl(uiControl(b), b->widget);

	b->controls = newPtrArray();

	b->vertical = orientation == GTK_ORIENTATION_VERTICAL;

	uiControl(b)->Handle = boxHandle;
	b->baseCommitDestroy = uiControl(b)->CommitDestroy;
	uiControl(b)->CommitDestroy = boxCommitDestroy;
	uiControl(b)->ContainerUpdateState = boxContainerUpdateState;

	uiBox(b)->Append = boxAppend;
	uiBox(b)->Delete = boxDelete;
	uiBox(b)->Padded = boxPadded;
	uiBox(b)->SetPadded = boxSetPadded;

	return uiBox(b);
}

uiBox *uiNewHorizontalBox(void)
{
	return finishNewBox(GTK_ORIENTATION_HORIZONTAL);
}

uiBox *uiNewVerticalBox(void)
{
	return finishNewBox(GTK_ORIENTATION_VERTICAL);
}
