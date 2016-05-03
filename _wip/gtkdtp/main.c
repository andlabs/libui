// 4 september 2015
#include "dtp.h"

// #qo pkg-config: gtk+-3.0

GtkWidget *win;
GtkWidget *dtp;

// based on gtk_combo_box_list_position() in the GTK+ source code
// TODO monitor detection
static void allocationToScreen(GtkWidget *w, GtkAllocation *a)
{
	gint x, y;
	GdkWindow *window;

	x = 0;
	y = 0;
	if (!gtk_widget_get_has_window(w)) {
		x = a->x;
		y = a->y;
	}
	window = gtk_widget_get_window(w);
	gdk_window_get_root_coords(window, x, y, &x, &y);
	if (gtk_widget_get_direction(w) == GTK_TEXT_DIR_RTL)
		x += a->width;
	a->x = x;
	a->y = y;
}

static void toggled(GtkToggleButton *t, gpointer data)
{
	GtkWidget *toplevel;
	GtkAllocation allocation;

	toplevel = gtk_widget_get_toplevel(GTK_WIDGET(t));
	if (GTK_IS_WINDOW(toplevel))
		gtk_window_group_add_window(gtk_window_get_group(GTK_WINDOW(toplevel)),
			GTK_WINDOW(win));

	gtk_widget_get_allocation(GTK_WIDGET(t), &allocation);
	allocationToScreen(GTK_WIDGET(t), &allocation);
	gtk_window_move(GTK_WINDOW(win), allocation.x, allocation.y + allocation.height);

	gtk_widget_show_all(win);
}

int main(void)
{
	GtkWidget *mainwin;
	GtkWidget *button;

	gtk_init(NULL, NULL);

	mainwin = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	g_signal_connect(mainwin, "destroy", G_CALLBACK(gtk_main_quit), NULL);
	gtk_container_set_border_width(GTK_CONTAINER(mainwin), 12);
	button = gtk_toggle_button_new_with_label("Click");
	g_signal_connect(button, "toggled", G_CALLBACK(toggled), NULL);
	gtk_container_add(GTK_CONTAINER(mainwin), button);

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

	gtk_widget_show_all(mainwin);
	gtk_main();
	return 0;
}
