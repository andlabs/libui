// 11 june 2015
#include "uipriv_unix.h"

struct group {
	uiGroup g;
	GtkWidget *widget;
	GtkContainer *container;
	GtkFrame *frame;

	// unfortunately, even though a GtkFrame is a GtkBin, calling gtk_container_set_border_width() on it /includes/ the GtkFrame's label; we don't want tht
	uiControl *bin;
	uiControl *child;

	void (*baseCommitDestroy)(uiControl *);
};

uiDefineControlType(uiGroup, uiTypeGroup, struct group)

static void groupCommitDestroy(uiControl *c)
{
	struct group *g = (struct group *) c;

	if (g->child != NULL) {
		binSetChild(g->bin, NULL);
		uiControlDestroy(g->child);
	}
	uiControlDestroy(g->bin);
	(*(g->baseCommitDestroy))(uiControl(g));
}

static uintptr_t groupHandle(uiControl *c)
{
	struct group *g = (struct group *) c;

	return (uintptr_t) (g->widget);
}

static void groupContainerUpdateState(uiControl *c)
{
	struct group *g = (struct group *) c;

	if (g->child != NULL)
		uiControlUpdateState(g->child);
}

static char *groupTitle(uiGroup *gg)
{
	struct group *g = (struct group *) gg;

	return uiUnixStrdupText(gtk_frame_get_label(g->frame));
}

static void groupSetTitle(uiGroup *gg, const char *text)
{
	struct group *g = (struct group *) gg;

	gtk_frame_set_label(g->frame, text);
	// changing the text might necessitate a change in the groupbox's size
	uiControlQueueResize(uiControl(g));
}

static void groupSetChild(uiGroup *gg, uiControl *child)
{
	struct group *g = (struct group *) gg;

	if (g->child != NULL)
		binSetChild(g->bin, NULL);
	g->child = child;
	if (g->child != NULL) {
		binSetChild(g->bin, g->child);
		uiControlQueueResize(g->child);
	}
}

static int groupMargined(uiGroup *gg)
{
	struct group *g = (struct group *) gg;

	return binMargined(g->bin);
}

// TODO this includes the label
static void groupSetMargined(uiGroup *gg, int margined)
{
	struct group *g = (struct group *) gg;

	binSetMargined(g->bin, margined);
	uiControlQueueResize(uiControl(g));
}

uiGroup *uiNewGroup(const char *text)
{
	struct group *g;
	gfloat yalign;
	GtkLabel *label;
	PangoAttribute *bold;
	PangoAttrList *boldlist;

	g = (struct group *) uiNewControl(uiTypeGroup());

	g->widget = gtk_frame_new(text);
	g->container = GTK_CONTAINER(g->widget);
	g->frame = GTK_FRAME(g->widget);
	uiUnixMakeSingleWidgetControl(uiControl(g), g->widget);

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

	g->bin = newBin();
	// can't use uiControlSetParent() because we didn't set the vtable yet
	gtk_container_add(g->container, GTK_WIDGET(uiControlHandle(g->bin)));
	// TODO this is a mess
	gtk_widget_show(GTK_WIDGET(uiControlHandle(g->bin)));

	uiControl(g)->Handle = groupHandle;
	g->baseCommitDestroy = uiControl(g)->CommitDestroy;
	uiControl(g)->CommitDestroy = groupCommitDestroy;
	uiControl(g)->ContainerUpdateState = groupContainerUpdateState;

	uiGroup(g)->Title = groupTitle;
	uiGroup(g)->SetTitle = groupSetTitle;
	uiGroup(g)->SetChild = groupSetChild;
	uiGroup(g)->Margined = groupMargined;
	uiGroup(g)->SetMargined = groupSetMargined;

	return uiGroup(g);
}
