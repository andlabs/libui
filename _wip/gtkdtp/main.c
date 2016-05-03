// 4 september 2015
#include "dtp.h"

// #qo pkg-config: gtk+-3.0

GtkWidget *win;
GtkWidget *dtp;
GdkDevice *keyboard;
GdkDevice *mouse;

// again, a lot of this is in the order that GtkComboBox does it
static void hidePopup(void)
{
	if (keyboard != NULL)
		gdk_device_ungrab(keyboard, GDK_CURRENT_TIME);
	gdk_device_ungrab(mouse, GDK_CURRENT_TIME);
	gtk_device_grab_remove(win, mouse);
	keyboard = NULL;
	mouse = NULL;

	gtk_widget_hide(win);
}

static gboolean stopPopup(GtkWidget *widget, GdkEvent *event, gpointer data)
{
	hidePopup();
	return TRUE;		// this is what GtkComboBox does
}

// this consolidates a good chunk of what GtkComboBox does
static gboolean startGrab(void)
{
	GdkDevice *dev;
	guint32 time;
	GdkWindow *window;

	dev = gtk_get_current_event_device();
	if (dev == NULL)
		return FALSE; // TODO

	time = gtk_get_current_event_time();
	keyboard = dev;
	mouse = gdk_device_get_associated_device(dev);
	if (gdk_device_get_source(dev) != GDK_SOURCE_KEYBOARD) {
		dev = mouse;
		mouse = keyboard;
		keyboard = dev;
	}

	window = gtk_widget_get_window(win);
	if (keyboard != NULL)
		if (gdk_device_grab(keyboard, window,
			GDK_OWNERSHIP_WINDOW, TRUE,
			GDK_KEY_PRESS_MASK | GDK_KEY_RELEASE_MASK,
			NULL, time) != GDK_GRAB_SUCCESS)
			return FALSE;
	if (mouse != NULL)
		if (gdk_device_grab(mouse, window,
			GDK_OWNERSHIP_WINDOW, TRUE,
			GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK | GDK_POINTER_MOTION_MASK,
			NULL, time) != GDK_GRAB_SUCCESS) {
			if (keyboard != NULL)
				gdk_device_ungrab(keyboard, time);
			return FALSE;
		}

	gtk_device_grab_add(win, mouse, TRUE);
	// TODO store keyboard and mouse
	g_signal_connect(win, "grab-broken-event", G_CALLBACK(stopPopup), NULL);
	g_signal_connect(win, "button-release-event", G_CALLBACK(stopPopup), NULL);
	return TRUE;
}

// based on gtk_combo_box_list_position() in the GTK+ source code
static void allocationToScreen(GtkWidget *w, GtkAllocation *a, gint *x, gint *y)
{
	GdkWindow *window;

	*x = 0;
	*y = 0;
	if (!gtk_widget_get_has_window(w)) {
		*x = a->x;
		*y = a->y;
	}
	window = gtk_widget_get_window(w);
	gdk_window_get_root_coords(window, *x, *y, x, y);
	if (gtk_widget_get_direction(w) == GTK_TEXT_DIR_RTL)
		*x += a->width;		// TODO subtract target width
	// TODO monitor detection
	*y += a->height;
}

static void showPopup(GtkWidget *button)
{
	GtkWidget *toplevel;
	GtkAllocation allocation;
	gint x, y;

	toplevel = gtk_widget_get_toplevel(button);
	if (GTK_IS_WINDOW(toplevel))
		gtk_window_group_add_window(gtk_window_get_group(GTK_WINDOW(toplevel)),
			GTK_WINDOW(win));

	gtk_widget_get_allocation(button, &allocation);
	allocationToScreen(button, &allocation, &x, &y);
	gtk_window_move(GTK_WINDOW(win), x, y);

	gtk_widget_show(win);

	if (!startGrab())
		hidePopup();
}

// TODO verify signature
static void toggled(GtkToggleButton *t, gpointer data)
{
	if (gtk_toggle_button_get_active(t))
		showPopup(GTK_WIDGET(t));
	else
		hidePopup();
}

int main(void)
{
	GtkWidget *mainwin;
	GtkWidget *box;
	GtkWidget *button;

	gtk_init(NULL, NULL);

	mainwin = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	g_signal_connect(mainwin, "destroy", G_CALLBACK(gtk_main_quit), NULL);
	gtk_container_set_border_width(GTK_CONTAINER(mainwin), 12);
	box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 6);
	gtk_container_add(GTK_CONTAINER(mainwin), box);
	button = gtk_toggle_button_new_with_label("Click");
	g_signal_connect(button, "toggled", G_CALLBACK(toggled), NULL);
	gtk_container_add(GTK_CONTAINER(box), button);
	gtk_container_add(GTK_CONTAINER(box), gtk_entry_new());

	win = gtk_window_new(GTK_WINDOW_POPUP);
	gtk_window_set_resizable(GTK_WINDOW(win), FALSE);
	gtk_window_set_attached_to(GTK_WINDOW(win), button);
	// TODO set_keep_above()?
	gtk_window_set_decorated(GTK_WINDOW(win), FALSE);
	gtk_window_set_deletable(GTK_WINDOW(win), FALSE);
	gtk_window_set_type_hint(GTK_WINDOW(win), GDK_WINDOW_TYPE_HINT_COMBO);
	gtk_window_set_skip_taskbar_hint(GTK_WINDOW(win), TRUE);
	gtk_window_set_skip_pager_hint(GTK_WINDOW(win), TRUE);
	// TODO accept_focus()?
	// TODO focus_on_map()?
	gtk_window_set_has_resize_grip(GTK_WINDOW(win), FALSE);
	gtk_container_set_border_width(GTK_CONTAINER(win), 12);

	dtp = GTK_WIDGET(g_object_new(dateTimePickerWidget_get_type(), NULL));
	gtk_container_add(GTK_CONTAINER(win), dtp);
	gtk_widget_show(dtp);

	gtk_widget_show_all(mainwin);
	gtk_main();
	return 0;
}
