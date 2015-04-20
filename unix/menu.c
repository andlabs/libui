// 20 april 2015
#include "uipriv_unix.h"

static void appendSeparator(GtkMenuShell *menu)
{
	gtk_menu_shell_append(menu, gtk_separator_menu_item_new());
}

static void appendMenuItem(GtkMenuShell *menu, const uiMenuItem *item)
{
	GtkWidget *iw;

	// TODO see if there are stock items for these three
	if (item->Name == uiMenuItemQuit) {
		// TODO verify type
		appendSeparator(menu);
		iw = gtk_menu_item_new_with_label("Quit");
		gtk_menu_shell_append(menu, iw);
		return;
	}
	if (item->Name == uiMenuItemPreferences) {
		// TODO verify type
		appendSeparator(menu);
		iw = gtk_menu_item_new_with_label("Preferences");
		gtk_menu_shell_append(menu, iw);
		return;
	}
	if (item->Name == uiMenuItemAbout) {
		// TODO verify type
		appendSeparator(menu);
		iw = gtk_menu_item_new_with_label("About");
		gtk_menu_shell_append(menu, iw);
		return;
	}
	if (item->Name == uiMenuItemSeparator) {
		// TODO verify type
		appendSeparator(menu);
		return;
	}
	switch (item->Type) {
	case uiMenuItemTypeCommand:
		iw = gtk_menu_item_new_with_label(item->Name);
		gtk_menu_shell_append(menu, iw);
		return;
	case uiMenuItemTypeCheckbox:
		iw = gtk_check_menu_item_new_with_label(item->Name);
		gtk_menu_shell_append(menu, iw);
		return;
	}
	// TODO complain
}

static GtkWidget *makeMenu(const char *name, uiMenuItem *items)
{
	GtkWidget *menu;
	GtkWidget *submenu;
	const uiMenuItem *i;

	menu = gtk_menu_item_new_with_label(name);
	submenu = gtk_menu_new();
	for (i = items; i->Name != NULL; i++)
		appendMenuItem(GTK_MENU_SHELL(submenu), i);
	gtk_menu_item_set_submenu(GTK_MENU_ITEM(menu), submenu);
	return menu;
}

GtkWidget *makeMenubar(void)
{
	GtkWidget *menubar;
	const uiMenu *m;

	if (options.Menu == NULL)
		complain("asked to give uiWindow a menubar but didn't specify a menu in uiInitOptions");

	menubar = gtk_menu_bar_new();

	for (m = options.Menu; m->Name != NULL; m++)
		gtk_menu_shell_append(GTK_MENU_SHELL(menubar), makeMenu(m->Name, m->Items));

	gtk_widget_set_hexpand(menubar, TRUE);
	gtk_widget_set_halign(menubar, GTK_ALIGN_FILL);
	gtk_widget_show_all(menubar);
	return menubar;
}
