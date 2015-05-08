// 24 april 2015
#include "uipriv_windows.h"

// TODO migrate to ptrArray

static struct menu **menus = NULL;
static uintmax_t len = 0;
static uintmax_t cap = 0;
static BOOL menusFinalized = FALSE;
static WORD curID = 100;			// start somewhere safe
static BOOL hasQuit = FALSE;
static BOOL hasPreferences = FALSE;
static BOOL hasAbout = FALSE;

struct menu {
	uiMenu m;
	WCHAR *name;
	struct menuItem **items;
	uintmax_t len;
	uintmax_t cap;
};

struct menuItem {
	uiMenuItem mi;
	WCHAR *name;
	int type;
	WORD id;
	void (*onClicked)(uiMenuItem *, uiWindow *, void *);
	void *onClickedData;
	BOOL disabled;				// template for new instances; kept in sync with everything else
	BOOL checked;
	HMENU *hmenus;
	uintmax_t len;
	uintmax_t cap;
};

enum {
	typeRegular,
	typeCheckbox,
	typeQuit,
	typePreferences,
	typeAbout,
	typeSeparator,
};

#define grow 32

static void sync(struct menuItem *item)
{
	uintmax_t i;
	MENUITEMINFOW mi;

	ZeroMemory(&mi, sizeof (MENUITEMINFOW));
	mi.cbSize = sizeof (MENUITEMINFOW);
	mi.fMask = MIIM_STATE;
	if (item->disabled)
		mi.fState |= MFS_DISABLED;
	if (item->checked)
		mi.fState |= MFS_CHECKED;

	for (i = 0; i < item->len; i++)
		if (SetMenuItemInfo(item->hmenus[i], item->id, FALSE, &mi) == 0)
			logLastError("error synchronizing menu items in windows/menu.c sync()");
}

static void defaultOnClicked(uiMenuItem *item, uiWindow *w, void *data)
{
	// do nothing
}

static void menuItemEnable(uiMenuItem *ii)
{
	struct menuItem *item = (struct menuItem *) ii;

	item->disabled = FALSE;
	sync(item);
}

static void menuItemDisable(uiMenuItem *ii)
{
	struct menuItem *item = (struct menuItem *) ii;

	item->disabled = TRUE;
	sync(item);
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

	return item->checked != FALSE;
}

static void menuItemSetChecked(uiMenuItem *ii, int checked)
{
	struct menuItem *item = (struct menuItem *) ii;

	// use explicit values
	item->checked = FALSE;
	if (checked)
		item->checked = TRUE;
	sync(item);
}

static uiMenuItem *newItem(struct menu *m, int type, const char *name)
{
	struct menuItem *item;

	if (menusFinalized)
		complain("attempt to create a new menu item after menus have been finalized");

	if (m->len >= m->cap) {
		m->cap += grow;
		m->items = (struct menuItem **) uiRealloc(m->items, m->cap * sizeof (struct menuItem *), "struct menuItem *[]");
	}

	item = uiNew(struct menuItem);
	m->items[m->len] = item;
	m->len++;

	item->type = type;
	switch (item->type) {
	case typeQuit:
		item->name = toUTF16("Quit");
		break;
	case typePreferences:
		item->name = toUTF16("Preferences...");
		break;
	case typeAbout:
		item->name = toUTF16("About");
		break;
	case typeSeparator:
		break;
	default:
		item->name = toUTF16(name);
		break;
	}

	if (item->type != typeSeparator) {
		item->id = curID;
		curID++;
	}

	item->onClicked = defaultOnClicked;

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
	if (len >= cap) {
		cap += grow;
		menus = (struct menu **) uiRealloc(menus, cap * sizeof (struct menu *), "struct menu *[]");
	}

	m = uiNew(struct menu);
	menus[len] = m;
	len++;

	m->name = toUTF16(name);

	uiMenu(m)->AppendItem = menuAppendItem;
	uiMenu(m)->AppendCheckItem = menuAppendCheckItem;
	uiMenu(m)->AppendQuitItem = menuAppendQuitItem;
	uiMenu(m)->AppendPreferencesItem = menuAppendPreferencesItem;
	uiMenu(m)->AppendAboutItem = menuAppendAboutItem;
	uiMenu(m)->AppendSeparator = menuAppendSeparator;

	return uiMenu(m);
}

static void appendMenuItem(HMENU menu, struct menuItem *item)
{
	UINT uFlags;

	uFlags = MF_SEPARATOR;
	if (item->type != typeSeparator) {
		uFlags = MF_STRING;
		if (item->disabled)
			uFlags |= MF_DISABLED | MF_GRAYED;
		if (item->checked)
			uFlags |= MF_CHECKED;
	}
	if (AppendMenuW(menu, uFlags, item->id, item->name) == 0)
		logLastError("error appending menu item in appendMenuItem()");

	if (item->len >= item->cap) {
		item->cap += grow;
		item->hmenus = (HMENU *) uiRealloc(item->hmenus, item->cap * sizeof (HMENU), "HMENU[]");
	}
	item->hmenus[item->len] = menu;
	item->len++;
}

static HMENU makeMenu(struct menu *m)
{
	HMENU menu;
	uintmax_t i;

	menu = CreatePopupMenu();
	if (menu == NULL)
		logLastError("error creating menu in makeMenu()");
	for (i = 0; i < m->len; i++)
		appendMenuItem(menu, m->items[i]);
	return menu;
}

HMENU makeMenubar(void)
{
	HMENU menubar;
	HMENU menu;
	uintmax_t i;

	menusFinalized = TRUE;

	menubar = CreateMenu();
	if (menubar == NULL)
		logLastError("error creating menubar in makeMenubar()");

	for (i = 0; i < len; i++) {
		menu = makeMenu(menus[i]);
		if (AppendMenuW(menubar, MF_POPUP | MF_STRING, (UINT_PTR) menu, menus[i]->name) == 0)
			logLastError("error appending menu to menubar in makeMenubar()");
	}

	return menubar;
}

void runMenuEvent(WORD id, uiWindow *w)
{
	struct menu *m;
	struct menuItem *item;
	uintmax_t i, j;
	uiMenuItem *umi;

	// this isn't optimal, but it works, and it should be just fine for most cases
	for (i = 0; i < len; i++) {
		m = menus[i];
		for (j = 0; j < m->len; j++) {
			item = m->items[j];
			if (item->id == id)
				goto found;
		}
	}
	// no match
	complain("unknown menu ID %hu in runMenuEvent()", id);

found:
	umi = uiMenuItem(item);

	// first toggle checkboxes, if any
	if (item->type == typeCheckbox)
		uiMenuItemSetChecked(umi, !uiMenuItemChecked(umi));

	// then run the event
	(*(item->onClicked))(umi, w, item->onClickedData);
}

static void freeMenu(struct menu *m, HMENU submenu)
{
	uintmax_t i;
	struct menuItem *item;
	uintmax_t j;

	for (i = 0; i < m->len; i++) {
		item = m->items[i];
		for (j = 0; j < item->len; j++)
			if (item->hmenus[j] == submenu)
				break;
		if (j >= item->len)
			complain("submenu handle %p not found in freeMenu()", submenu);
		for (; j < item->len - 1; j++)
			item->hmenus[j] = item->hmenus[j + 1];
		item->hmenus[j] = NULL;
		item->len--;
	}
}

void freeMenubar(HMENU menubar)
{
	uintmax_t i;
	MENUITEMINFOW mi;

	for (i = 0; i < len; i++) {
		ZeroMemory(&mi, sizeof (MENUITEMINFOW));
		mi.cbSize = sizeof (MENUITEMINFOW);
		mi.fMask = MIIM_SUBMENU;
		if (GetMenuItemInfoW(menubar, i, TRUE, &mi) == 0)
			logLastError("error getting menu to delete item references from in freeMenubar()");
		freeMenu(menus[i], mi.hSubMenu);
	}
	// no need to worry about destroying any menus; destruction of the window they're in will do it for us
}

void uninitMenus(void)
{
	struct menu *m;
	struct menuItem *item;
	uintmax_t i, j;

	for (i = 0; i < len; i++) {
		m = menus[i];
		uiFree(m->name);
		for (j = 0; j < m->len; j++) {
			item = m->items[j];
			if (item->len != 0)
				complain("menu item %p (%ws) still has uiWindows attached; did you forget to free some?", item, item->name);
			if (item->name != NULL)
				uiFree(item->name);
			if (item->hmenus != NULL)
				uiFree(item->hmenus);
			uiFree(item);
		}
		if (m->items != NULL)
			uiFree(m->items);
		uiFree(m);
	}
	if (menus != NULL)
		uiFree(menus);
}
