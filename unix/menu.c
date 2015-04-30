// 23 april 2015
#include "uipriv_unix.h"

// TODO window destruction
// TODO get rid of the base item and store the GType, the disabled flag, and the checked flag like we do on Windows?

static GArray *menus = NULL;
static gboolean menusFinalized = FALSE;

struct menu {
	uiMenu m;
	char *name;
	GArray *items;					// []*struct menuItem
};

struct menuItem {
	uiMenuItem mi;
	char *name;
	int type;
	void (*onClicked)(uiMenuItem *, uiWindow *, void *);
	void *onClickedData;
	GtkWidget *baseItem;			// template for new instances; kept in sync with everything else
	GHashTable *uiWindows;			// map[GtkMenuItem]uiWindow
	// TODO this assumes that a gulong can fit in a gpointer
	GHashTable *signals;			// map[GtkMenuItem]gulong
};

enum {
	typeRegular,
	typeCheckbox,
	typeQuit,
	typePreferences,
	typeAbout,
	typeSeparator,
};

#define NEWHASH() g_hash_table_new(g_direct_hash, g_direct_equal)

// we do NOT want programmatic updates to raise an ::activated signal
static void singleSetChecked(GtkCheckMenuItem *menuitem, gboolean checked, gulong signal)
{
	g_signal_handler_block(menuitem, signal);
	gtk_check_menu_item_set_active(menuitem, checked);
	g_signal_handler_unblock(menuitem, signal);
}

static void setChecked(struct menuItem *item, gboolean checked)
{
	GHashTableIter iter;
	gpointer widget;

	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(item->baseItem), checked);
	g_hash_table_iter_init(&iter, item->uiWindows);
	while (g_hash_table_iter_next(&iter, &widget, NULL))
		singleSetChecked(GTK_CHECK_MENU_ITEM(widget), checked, (gulong) g_hash_table_lookup(item->signals, widget));
}

static void onClicked(GtkMenuItem *menuitem, gpointer data)
{
	struct menuItem *item = (struct menuItem *) data;
	uiWindow *w;

	// we need to manually update the checked states of all menu items if one changes
	// notice that this is getting the checked state of the menu item that this signal is sent from
	if (item->type == typeCheckbox)
		setChecked(item, gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(menuitem)));

	w = uiWindow(g_hash_table_lookup(item->uiWindows, menuitem));
	(*(item->onClicked))(uiMenuItem(item), w, item->onClickedData);
}

static void defaultOnClicked(uiMenuItem *item, uiWindow *w, void *data)
{
	// do nothing
}

static void menuItemEnableDisable(struct menuItem *item, gboolean enabled)
{
	GHashTableIter iter;
	gpointer widget;

	gtk_widget_set_sensitive(item->baseItem, enabled);
	g_hash_table_iter_init(&iter, item->uiWindows);
	while (g_hash_table_iter_next(&iter, &widget, NULL))
		gtk_widget_set_sensitive(GTK_WIDGET(widget), enabled);
}

static void menuItemEnable(uiMenuItem *ii)
{
	struct menuItem *item = (struct menuItem *) ii;

	menuItemEnableDisable(item, TRUE);
}

static void menuItemDisable(uiMenuItem *ii)
{
	struct menuItem *item = (struct menuItem *) ii;

	menuItemEnableDisable(item, FALSE);
}

static void menuItemOnClicked(uiMenuItem *ii, void (*f)(uiMenuItem *, uiWindow *, void *), void *data)
{
	struct menuItem *item = (struct menuItem *) ii;

	item->onClicked = f;
	item->onClickedData = data;
}

static int menuItemChecked(uiMenuItem *ii)
{
	struct menuItem *item = (struct menuItem *) ii;

	return gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(item->baseItem)) != FALSE;
}

static void menuItemSetChecked(uiMenuItem *ii, int checked)
{
	struct menuItem *item = (struct menuItem *) ii;
	gboolean c;

	// use explicit values
	c = FALSE;
	if (checked)
		c = TRUE;
	setChecked(item, c);
}

static uiMenuItem *newItem(struct menu *m, int type, const char *name)
{
	struct menuItem *item;

	if (menusFinalized)
		complain("attempt to create a new menu item after menus have been finalized");

	item = uiNew(struct menuItem);
	g_array_append_val(m->items, item);

	item->type = type;
	switch (item->type) {
	case typeQuit:
		item->name = g_strdup("Quit");
		break;
	case typePreferences:
		item->name = g_strdup("Preferences...");
		break;
	case typeAbout:
		item->name = g_strdup("About");
		break;
	case typeSeparator:
		break;
	default:
		item->name = g_strdup(name);
		break;
	}

	item->onClicked = defaultOnClicked;

	switch (item->type) {
	case typeCheckbox:
		item->baseItem = gtk_check_menu_item_new_with_label(item->name);
		break;
	case typeSeparator:
		item->baseItem = gtk_separator_menu_item_new();
		break;
	default:
		item->baseItem = gtk_menu_item_new_with_label(item->name);
		break;
	}

	item->uiWindows = NEWHASH();
	item->signals = NEWHASH();

	uiMenuItem(item)->Enable = menuItemEnable;
	uiMenuItem(item)->Disable = menuItemDisable;
	uiMenuItem(item)->OnClicked = menuItemOnClicked;
	uiMenuItem(item)->Checked = menuItemChecked;
	uiMenuItem(item)->SetChecked = menuItemSetChecked;

	return uiMenuItem(item);
}

uiMenuItem *menuAppendItem(uiMenu *mm, const char *name)
{
	return newItem((struct menu *) mm, typeRegular, name);
}

uiMenuItem *menuAppendCheckItem(uiMenu *mm, const char *name)
{
	return newItem((struct menu *) mm, typeCheckbox, name);
}

uiMenuItem *menuAppendQuitItem(uiMenu *mm)
{
	// TODO check multiple quit items
	// TODO conditionally add separator
	newItem((struct menu *) mm, typeSeparator, NULL);
	return newItem((struct menu *) mm, typeQuit, NULL);
}

uiMenuItem *menuAppendPreferencesItem(uiMenu *mm)
{
	// TODO check multiple preferences items
	// TODO conditionally add separator
	newItem((struct menu *) mm, typeSeparator, NULL);
	return newItem((struct menu *) mm, typePreferences, NULL);
}

uiMenuItem *menuAppendAboutItem(uiMenu *mm)
{
	// TODO check multiple about items
	// TODO conditionally add separator
	newItem((struct menu *) mm, typeSeparator, NULL);
	return newItem((struct menu *) mm, typeAbout, NULL);
}

void menuAppendSeparator(uiMenu *mm)
{
	// TODO check multiple about items
	newItem((struct menu *) mm, typeSeparator, NULL);
}

uiMenu *uiNewMenu(const char *name)
{
	struct menu *m;

	if (menusFinalized)
		complain("attempt to create a new menu after menus have been finalized");
	if (menus == NULL)
		menus = g_array_new(FALSE, TRUE, sizeof (struct menu *));

	m = uiNew(struct menu);
	g_array_append_val(menus, m);

	m->name = g_strdup(name);
	m->items = g_array_new(FALSE, TRUE, sizeof (struct menuItem *));

	uiMenu(m)->AppendItem = menuAppendItem;
	uiMenu(m)->AppendCheckItem = menuAppendCheckItem;
	uiMenu(m)->AppendQuitItem = menuAppendQuitItem;
	uiMenu(m)->AppendPreferencesItem = menuAppendPreferencesItem;
	uiMenu(m)->AppendAboutItem = menuAppendAboutItem;
	uiMenu(m)->AppendSeparator = menuAppendSeparator;

	return uiMenu(m);
}

static void appendMenuItem(GtkMenuShell *submenu, struct menuItem *item, uiWindow *w)
{
	GtkWidget *menuitem;
	gulong signal;

	menuitem = g_object_new(G_OBJECT_TYPE(item->baseItem), NULL);
	if (item->name != NULL)
		gtk_menu_item_set_label(GTK_MENU_ITEM(menuitem), item->name);
	if (item->type != typeSeparator) {
		signal = g_signal_connect(menuitem, "activate", G_CALLBACK(onClicked), item);
		gtk_widget_set_sensitive(menuitem, gtk_widget_get_sensitive(item->baseItem));
		if (item->type == typeCheckbox)
			singleSetChecked(GTK_CHECK_MENU_ITEM(menuitem), gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(item->baseItem)), signal);
	}
	gtk_menu_shell_append(submenu, menuitem);
	g_hash_table_insert(item->uiWindows, menuitem, w);
	g_hash_table_insert(item->signals, menuitem, (gpointer) signal);
}

// TODO should this return a zero-height widget (or NULL) if there are no menus defined?
GtkWidget *makeMenubar(uiWindow *w)
{
	GtkWidget *menubar;
	guint i, j;
	struct menu *m;
	GtkWidget *menuitem;
	GtkWidget *submenu;

	menusFinalized = TRUE;

	menubar = gtk_menu_bar_new();

	for (i = 0; i < menus->len; i++) {
		m = g_array_index(menus, struct menu *, i);
		menuitem = gtk_menu_item_new_with_label(m->name);
		submenu = gtk_menu_new();
		gtk_menu_item_set_submenu(GTK_MENU_ITEM(menuitem), submenu);
		for (j = 0; j < m->items->len; j++)
			appendMenuItem(GTK_MENU_SHELL(submenu), g_array_index(m->items, struct menuItem *, j), w);
		gtk_menu_shell_append(GTK_MENU_SHELL(menubar), menuitem);
	}

	gtk_widget_set_hexpand(menubar, TRUE);
	gtk_widget_set_halign(menubar, GTK_ALIGN_FILL);
	return menubar;
}

void freeMenubar(GtkWidget *mb)
{
	// TODO
}
