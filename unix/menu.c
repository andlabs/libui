// 20 april 2015
#include "uipriv_unix.h"

static GtkWidget *makeMenuItem(const char *name, uiMenuItem *items)
{
	return gtk_menu_item_new_with_label(name);
}

GtkWidget *makeMenubar(void)
{
	GtkWidget *menubar;
	const uiMenu *m;

	if (options.Menu == NULL)
		complain("asked to give uiWindow a menubar but didn't specify a menu in uiInitOptions");

	menubar = gtk_menu_bar_new();

	for (m = options.Menu; m->Name != NULL; m++)
		gtk_menu_shell_append(GTK_MENU_SHELL(menubar), makeMenuItem(m->Name, m->Items));

	gtk_widget_set_hexpand(menubar, TRUE);
	gtk_widget_set_halign(menubar, GTK_ALIGN_FILL);
	gtk_widget_show_all(menubar);
	return menubar;
}
