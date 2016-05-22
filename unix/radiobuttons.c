// 11 june 2015
#include "uipriv_unix.h"

// on GTK+ a uiRadioButtons is a GtkBox with each of the GtkRadioButtons as children

struct uiRadioButtons {
	uiUnixControl c;
	GtkWidget *widget;
	GtkContainer *container;
	GtkBox *box;
	GPtrArray *buttons;
};

uiUnixControlAllDefaultsExceptDestroy(uiRadioButtons)

static void uiRadioButtonsDestroy(uiControl *c)
{
	uiRadioButtons *r = uiRadioButtons(c);
	GtkWidget *b;

	while (r->buttons->len != 0) {
		b = GTK_WIDGET(g_ptr_array_remove_index(r->buttons, 0));
		gtk_widget_destroy(b);
	}
	g_ptr_array_free(r->buttons, TRUE);
	// and free ourselves
	g_object_unref(r->widget);
	uiFreeControl(uiControl(r));
}

void uiRadioButtonsAppend(uiRadioButtons *r, const char *text)
{
	GtkWidget *rb;
	GtkRadioButton *previous;

	previous = NULL;
	if (r->buttons->len > 0)
		previous = GTK_RADIO_BUTTON(g_ptr_array_index(r->buttons, 0));
	rb = gtk_radio_button_new_with_label_from_widget(previous, text);
	gtk_container_add(r->container, rb);
	g_ptr_array_add(r->buttons, rb);
	gtk_widget_show(rb);
}

uiRadioButtons *uiNewRadioButtons(void)
{
	uiRadioButtons *r;

	uiUnixNewControl(uiRadioButtons, r);

	r->widget = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
	r->container = GTK_CONTAINER(r->widget);
	r->box = GTK_BOX(r->widget);

	r->buttons = g_ptr_array_new();

	return r;
}
