// 11 june 2015
#include "uipriv_unix.h"

// on GTK+ a uiRadioButtons is a GtkBox with each of the GtkRadioButtons as children

struct radiobuttons {
	uiRadioButtons r;
	GtkWidget *boxWidget;
	GtkContainer *boxContainer;
	GtkBox *box;
	GPtrArray *buttons;
};

uiDefineControlType(uiRadioButtons, uiTypeRadioButtons, struct radiobuttons)

// TODO destroy

// TODO note that the handle of a uiRadioButtons is undefined (or at least highly platform-dependent and unreliable)
static uintptr_t radiobuttonsHandle(uiControl *c)
{
	struct radiobuttons *r = (struct radiobuttons *) c;

	return (uintptr_t) (r->boxWidget);
}

static void radiobuttonsAppend(uiRadioButtons *rr, const char *text)
{
	struct radiobuttons *r = (struct radiobuttons *) rr;
	GtkWidget *rb;
	GtkRadioButton *previous;

	previous = NULL;
	if (r->buttons->len > 0)
		previous = GTK_RADIO_BUTTON(g_ptr_array_index(r->buttons, 0));
	rb = gtk_radio_button_new_with_label_from_widget(previous, text);
	gtk_container_add(r->boxContainer, rb);
	g_ptr_array_add(r->buttons, rb);
	gtk_widget_show(rb);
	uiControlQueueResize(uiControl(r));
}

uiRadioButtons *uiNewRadioButtons(void)
{
	struct radiobuttons *r;

	r = (struct radiobuttons *) uiNewControl(uiTypeRadioButtons());

	r->boxWidget = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
	r->boxContainer = GTK_CONTAINER(r->boxWidget);
	r->box = GTK_BOX(r->boxWidget);
	uiUnixMakeSingleWidgetControl(uiControl(r), r->boxWidget);

	r->buttons = g_ptr_array_new();

	uiControl(r)->Handle = radiobuttonsHandle;

	uiRadioButtons(r)->Append = radiobuttonsAppend;

	return uiRadioButtons(r);
}
