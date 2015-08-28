// 11 june 2015
#include "uipriv_unix.h"

struct uiGroup {
	uiUnixControl c;
	GtkWidget *widget;
	GtkContainer *container;
	GtkFrame *frame;

	// unfortunately, even though a GtkFrame is a GtkBin, calling gtk_container_set_border_width() on it /includes/ the GtkFrame's label; we don't want tht
	GtkWidget *box;
	uiControl *child;

	int margined;
};

static void onDestroy(uiGroup *);

uiUnixDefineControlWithOnDestroy(
	uiGroup,								// type name
	uiGroupType,							// type function
	onDestroy(this);						// on destroy
)

static void onDestroy(uiGroup *g)
{
	if (g->child != NULL) {
		uiControlSetParent(g->child, NULL);
		uiControlDestroy(g->child);
	}
	gtk_widget_destroy(g->box);
}

static void groupContainerUpdateState(uiControl *c)
{
	uiGroup *g = uiGroup(c);

	if (g->child != NULL)
		controlUpdateState(g->child);
}

char *uiGroupTitle(uiGroup *g)
{
	return uiUnixStrdupText(gtk_frame_get_label(g->frame));
}

void uiGroupSetTitle(uiGroup *g, const char *text)
{
	gtk_frame_set_label(g->frame, text);
	// changing the text might necessitate a change in the groupbox's size
	uiControlQueueResize(uiControl(g));
}

void uiGroupSetChild(uiGroup *g, uiControl *child)
{
	if (g->child != NULL) {
		gtk_container_remove(GTK_CONTAINER(g->box),
			GTK_WIDGET(uiControlHandle(g->child)));
		uiControlSetParent(g->child, NULL);
	}
	g->child = child;
	if (g->child != NULL) {
		uiControlSetParent(g->child, uiControl(g));
		gtk_container_add(GTK_CONTAINER(g->box),
			GTK_WIDGET(uiControlHandle(g->child)));
		uiControlQueueResize(g->child);
	}
}

int uiGroupMargined(uiGroup *g)
{
	return g->margined;
}

void uiGroupSetMargined(uiGroup *g, int margined)
{
	g->margined = margined;
	setMargined(GTK_CONTAINER(g->box), g->margined);
	uiControlQueueResize(uiControl(g));
}

uiGroup *uiNewGroup(const char *text)
{
	uiGroup *g;
	gfloat yalign;
	GtkLabel *label;
	PangoAttribute *bold;
	PangoAttrList *boldlist;

	g = (uiGroup *) uiNewControl(uiGroupType());

	g->widget = gtk_frame_new(text);
	g->container = GTK_CONTAINER(g->widget);
	g->frame = GTK_FRAME(g->widget);

	// with GTK+, groupboxes by default have frames and slightly x-offset regular text
	// they should have no frame and fully left-justified, bold text
	// preserve default y-alignment
	gtk_frame_get_label_align(g->frame, NULL, &yalign);
	gtk_frame_set_label_align(g->frame, 0, yalign);
	gtk_frame_set_shadow_type(g->frame, GTK_SHADOW_NONE);
	label = GTK_LABEL(gtk_frame_get_label_widget(g->frame));
	// this is the boldness level used by GtkPrintUnixDialog
	// (it technically uses "bold" but see pango's pango-enum-types.c for the name conversion; GType is weird)
	bold = pango_attr_weight_new(PANGO_WEIGHT_BOLD);
	boldlist = pango_attr_list_new();
	pango_attr_list_insert(boldlist, bold);
	gtk_label_set_attributes(label, boldlist);
	pango_attr_list_unref(boldlist);		// thanks baedert in irc.gimp.net/#gtk+

	g->box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
	gtk_container_add(g->container, g->box);
	gtk_widget_show(g->box);

	uiUnixFinishNewControl(g, uiGroup);
	uiControl(g)->ContainerUpdateState = groupContainerUpdateState;

	return g;
}
