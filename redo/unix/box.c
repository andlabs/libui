// 7 april 2015
#include "out/ui.h"
#include "uipriv.h"

struct box {
	uiBox b;
	GtkWidget *widget;
	GtkContainer *container;
	GtkBox *box;
	void (*baseCommitDestroy)(uiControl *);
	struct ptrArray *controls;		// TODO switch to GArray
	int vertical;
	int padded;
	GtkSizeGroup *stretchygroup;		// ensures all stretchy controls have the same size
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
	// kill the size group
	g_object_unref(b->stretchygroup);
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

	bc = uiNew(struct boxControl);
	bc->c = c;
	bc->stretchy = stretchy;
	uiControlSetParent(bc->c, uiControl(b));
	if (bc->stretchy)
		gtk_size_group_add_widget(b->stretchygroup, GTK_WIDGET(uiControlHandle(bc->c)));
	ptrArrayAppend(b->controls, bc);
	uiControlQueueResize(uiControl(b));
}

static void boxDelete(uiBox *ss, uintmax_t index)
{
	struct box *b = (struct box *) ss;
	struct boxControl *bc;

	bc = ptrArrayIndex(b->controls, struct boxControl *, index);
	ptrArrayDelete(b->controls, index);
	if (bc->stretchy)
		gtk_size_group_remove_widget(b->stretchygroup, GTK_WIDGET(uiControlHandle(bc->c)));
	uiControlSetParent(bc->c, NULL);
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

static uiBox *finishNewBox(GtkOrientationType orientation)
{
	struct box *b;

	b = (struct box *) uiNewControl(uiTypeBox());

	b->widget = gtk_box_new(orientation, 0);
	b->container = GTK_CONTAINER(b->widget);
	b->box = GTK_BOX(b->widget);

	b->vertical = orientation == GTK_ORIENTATION_VERTICAL;

	if (b->vertical)
		b->stretchygroup = gtk_size_group_new(GTK_SIZE_GROUP_VERTICAL);
	else
		b->stretchygroup = gtk_size_group_new(GTK_SIZE_GROUP_HORIZONTAL);

	b->controls = newPtrArray();

	uiControl(b)->Handle = boxHandle;
	uiControl(b)->PreferredSize = boxPreferredSize;
	b->baseResize = uiControl(b)->Resize;
	uiControl(b)->Resize = boxResize;
	uiControl(b)->HasTabStops = boxHasTabStops;
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
