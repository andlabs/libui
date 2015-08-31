// 24 april 2015
#include "uipriv_windows.h"

// TODO migrate to ptrArray

static uiMenu **menus = NULL;
static uintmax_t len = 0;
static uintmax_t cap = 0;
static BOOL menusFinalized = FALSE;
static WORD curID = 100;			// start somewhere safe
static BOOL hasQuit = FALSE;
static BOOL hasPreferences = FALSE;
static BOOL hasAbout = FALSE;

struct uiMenu {
	uiTyped t;
	WCHAR *name;
	uiMenuItem **items;
	uintmax_t len;
	uintmax_t cap;
};

struct uiMenuItem {
	uiTyped t;
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

static void sync(uiMenuItem *item)
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

static void onQuitClicked(uiMenuItem *item, uiWindow *w, void *data)
{
	if (shouldQuit())
		uiQuit();
}

void uiMenuItemEnable(uiMenuItem *i)
{
	i->disabled = FALSE;
	sync(i);
}

void uiMenuItemDisable(uiMenuItem *i)
{
	i->disabled = TRUE;
	sync(i);
}

void uiMenuItemOnClicked(uiMenuItem *i, void (*f)(uiMenuItem *, uiWindow *, void *), void *data)
{
	if (i->type == typeQuit)
		complain("attempt to call uiMenuItemOnClicked() on a Quit item; use uiOnShouldQuit() instead");
	i->onClicked = f;
	i->onClickedData = data;
}

int uiMenuItemChecked(uiMenuItem *i)
{
	return i->checked != FALSE;
}

void uiMenuItemSetChecked(uiMenuItem *i, int checked)
{
	// use explicit values
	i->checked = FALSE;
	if (checked)
		i->checked = TRUE;
	sync(i);
}

static uiMenuItem *newItem(uiMenu *m, int type, const char *name)
{
	uiMenuItem *item;

	if (menusFinalized)
		complain("attempt to create a new menu item after menus have been finalized");

	if (m->len >= m->cap) {
		m->cap += grow;
		m->items = (uiMenuItem **) uiRealloc(m->items, m->cap * sizeof (uiMenuItem *), "uiMenuitem *[]");
	}

	item = uiNew(uiMenuItem);
	uiTyped(item)->Type = uiMenuItemType();

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

	// TODO copy this from the unix one
	item->onClicked = defaultOnClicked;
	if (item->type == typeQuit)
		item->onClicked = onQuitClicked;

	return item;
}

uiMenuItem *uiMenuAppendItem(uiMenu *m, const char *name)
{
	return newItem(m, typeRegular, name);
}

uiMenuItem *uiMenuAppendCheckItem(uiMenu *m, const char *name)
{
	return newItem(m, typeCheckbox, name);
}

uiMenuItem *uiMenuAppendQuitItem(uiMenu *m)
{
	if (hasQuit)
		complain("attempt to add multiple Quit menu items");
	hasQuit = TRUE;
	newItem(m, typeSeparator, NULL);
	return newItem(m, typeQuit, NULL);
}

uiMenuItem *uiMenuAppendPreferencesItem(uiMenu *m)
{
	if (hasPreferences)
		complain("attempt to add multiple Preferences menu items");
	hasPreferences = TRUE;
	newItem(m, typeSeparator, NULL);
	return newItem(m, typePreferences, NULL);
}

uiMenuItem *uiMenuAppendAboutItem(uiMenu *m)
{
	if (hasAbout)
		complain("attempt to add multiple About menu items");
	hasAbout = TRUE;
	newItem(m, typeSeparator, NULL);
	return newItem(m, typeAbout, NULL);
}

void uiMenuAppendSeparator(uiMenu *m)
{
	newItem(m, typeSeparator, NULL);
}

uiMenu *uiNewMenu(const char *name)
{
	uiMenu *m;

	if (menusFinalized)
		complain("attempt to create a new menu after menus have been finalized");
	if (len >= cap) {
		cap += grow;
		menus = (uiMenu **) uiRealloc(menus, cap * sizeof (uiMenu *), "uiMenu *[]");
	}

	m = uiNew(uiMenu);
	uiTyped(m)->Type = uiMenuType();

	menus[len] = m;
	len++;

	m->name = toUTF16(name);

	return m;
}

static void appendMenuItem(HMENU menu, uiMenuItem *item)
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

static HMENU makeMenu(uiMenu *m)
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
	uiMenu *m;
	uiMenuItem *item;
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
	// first toggle checkboxes, if any
	if (item->type == typeCheckbox)
		uiMenuItemSetChecked(item, !uiMenuItemChecked(item));

	// then run the event
	(*(item->onClicked))(item, w, item->onClickedData);
}

static void freeMenu(uiMenu *m, HMENU submenu)
{
	uintmax_t i;
	uiMenuItem *item;
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
	uiMenu *m;
	uiMenuItem *item;
	uintmax_t i, j;

	for (i = 0; i < len; i++) {
		m = menus[i];
		uiFree(m->name);
		for (j = 0; j < m->len; j++) {
			item = m->items[j];
			if (item->len != 0)
				complain("menu item %p (%ws) still has uiWindows attached; did you forget to destroy some windows?", item, item->name);
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
