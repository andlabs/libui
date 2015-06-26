// 23 april 2015
#include "uipriv_unix.h"

static GArray *menus = NULL;
static gboolean menusFinalized = FALSE;
static gboolean hasQuit = FALSE;
static gboolean hasPreferences = FALSE;
static gboolean hasAbout = FALSE;

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
	GType gtype;					// template for new instances; kept in sync with everything else
	gboolean disabled;
	gboolean checked;
	GHashTable *windows;			// map[GtkMenuItem]*menuItemWindow
};

struct menuItemWindow {
	uiWindow *w;
	gulong signal;
};

enum {
	typeRegular,
	typeCheckbox,
	typeQuit,
	typePreferences,
	typeAbout,
	typeSeparator,
};

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
	gpointer ww;
	struct menuItemWindow *w;

	item->checked = checked;
	g_hash_table_iter_init(&iter, item->windows);
	while (g_hash_table_iter_next(&iter, &widget, &ww)) {
		w = (struct menuItemWindow *) ww;
		singleSetChecked(GTK_CHECK_MENU_ITEM(widget), item->checked, w->signal);
	}
}

static void onClicked(GtkMenuItem *menuitem, gpointer data)
{
	struct menuItem *item = (struct menuItem *) data;
	struct menuItemWindow *w;

	// we need to manually update the checked states of all menu items if one changes
	// notice that this is getting the checked state of the menu item that this signal is sent from
	if (item->type == typeCheckbox)
		setChecked(item, gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(menuitem)));

	w = (struct menuItemWindow *) g_hash_table_lookup(item->windows, menuitem);
	(*(item->onClicked))(uiMenuItem(item), w->w, item->onClickedData);
}

static void defaultOnClicked(uiMenuItem *item, uiWindow *w, void *data)
{
	// do nothing
}

static void onQuitClicked(uiMenuItem *item, uiWindow *w, void *data)
{
	if (shouldQuit())
		uiQuit();
}

static void menuItemEnableDisable(struct menuItem *item, gboolean enabled)
{
	GHashTableIter iter;
	gpointer widget;

	item->disabled = !enabled;
	g_hash_table_iter_init(&iter, item->windows);
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

	if (item->type == typeQuit)
		complain("attempt to call uiMenuItemOnClicked() on a Quit item; use uiOnShouldQuit() instead");
	item->onClicked = f;
	item->onClickedData = data;
}

static int menuItemChecked(uiMenuItem *ii)
{
	struct menuItem *item = (struct menuItem *) ii;

	return item->checked != FALSE;
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
	uiTyped(item)->Type = uiTypeMenuItem();

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
	if (item->type == typeQuit)
		item->onClicked = onQuitClicked;

	switch (item->type) {
	case typeCheckbox:
		item->gtype = GTK_TYPE_CHECK_MENU_ITEM;
		break;
	case typeSeparator:
		item->gtype = GTK_TYPE_SEPARATOR_MENU_ITEM;
		break;
	default:
		item->gtype = GTK_TYPE_MENU_ITEM;
		break;
	}

	item->windows = g_hash_table_new(g_direct_hash, g_direct_equal);

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
	if (hasQuit)
		complain("attempt to add multiple Quit menu items");
	hasQuit = TRUE;
	newItem((struct menu *) mm, typeSeparator, NULL);
	return newItem((struct menu *) mm, typeQuit, NULL);
}

uiMenuItem *menuAppendPreferencesItem(uiMenu *mm)
{
	if (hasPreferences)
		complain("attempt to add multiple Preferences menu items");
	hasPreferences = TRUE;
	newItem((struct menu *) mm, typeSeparator, NULL);
	return newItem((struct menu *) mm, typePreferences, NULL);
}

uiMenuItem *menuAppendAboutItem(uiMenu *mm)
{
	if (hasAbout)
		complain("attempt to add multiple About menu items");
	hasAbout = TRUE;
	newItem((struct menu *) mm, typeSeparator, NULL);
	return newItem((struct menu *) mm, typeAbout, NULL);
}

void menuAppendSeparator(uiMenu *mm)
{
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
	uiTyped(m)->Type = uiTypeMenu();

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
	struct menuItemWindow *ww;

	menuitem = g_object_new(item->gtype, NULL);
	if (item->name != NULL)
		gtk_menu_item_set_label(GTK_MENU_ITEM(menuitem), item->name);
	if (item->type != typeSeparator) {
		signal = g_signal_connect(menuitem, "activate", G_CALLBACK(onClicked), item);
		gtk_widget_set_sensitive(menuitem, !item->disabled);
		if (item->type == typeCheckbox)
			singleSetChecked(GTK_CHECK_MENU_ITEM(menuitem), item->checked, signal);
	}
	gtk_menu_shell_append(submenu, menuitem);
	ww = uiNew(struct menuItemWindow);
	ww->w = w;
	ww->signal = signal;
	g_hash_table_insert(item->windows, menuitem, ww);
}

GtkWidget *makeMenubar(uiWindow *w)
{
	GtkWidget *menubar;
	guint i, j;
	struct menu *m;
	GtkWidget *menuitem;
	GtkWidget *submenu;

	menusFinalized = TRUE;

	menubar = gtk_menu_bar_new();

	if (menus != NULL)
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

struct freeMenuItemData {
	GArray *items;
	guint i;
};

static void freeMenuItem(GtkWidget *widget, gpointer data)
{
	struct freeMenuItemData *fmi = (struct freeMenuItemData *) data;
	struct menuItem *item;
	struct menuItemWindow *w;

	item = g_array_index(fmi->items, struct menuItem *, fmi->i);
	w = (struct menuItemWindow *) g_hash_table_lookup(item->windows, widget);
	if (g_hash_table_remove(item->windows, widget) == FALSE)
		complain("GtkMenuItem %p not in menu item's item/window map", widget);
	uiFree(w);
	fmi->i++;
}

static void freeMenu(GtkWidget *widget, gpointer data)
{
	guint *i = (guint *) data;
	struct menu *m;
	GtkMenuItem *item;
	GtkWidget *submenu;
	struct freeMenuItemData fmi;

	m = g_array_index(menus, struct menu *, *i);
	item = GTK_MENU_ITEM(widget);
	submenu = gtk_menu_item_get_submenu(item);
	fmi.items = m->items;
	fmi.i = 0;
	gtk_container_foreach(GTK_CONTAINER(submenu), freeMenuItem, &fmi);
	(*i)++;
}

void freeMenubar(GtkWidget *mb)
{
	guint i;

	i = 0;
	gtk_container_foreach(GTK_CONTAINER(mb), freeMenu, &i);
	// no need to worry about destroying any widgets; destruction of the window they're in will do it for us
}

void uninitMenus(void)
{
	struct menu *m;
	struct menuItem *item;
	guint i, j;

	if (menus == NULL)
		return;
	for (i = 0; i < menus->len; i++) {
		m = g_array_index(menus, struct menu *, i);
		g_free(m->name);
		for (j = 0; j < m->items->len; j++) {
			item = g_array_index(m->items, struct menuItem *, j);
			if (g_hash_table_size(item->windows) != 0)
				complain("menu item %p (%s) still has uiWindows attached; did you forget to destroy some windows?", item, item->name);
			g_free(item->name);
			g_hash_table_destroy(item->windows);
			uiFree(item);
		}
		g_array_free(m->items, TRUE);
		uiFree(m);
	}
	g_array_free(menus, TRUE);
}
