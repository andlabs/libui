// 24 april 2015
#include "uipriv_windows.h"

// TODO window destruction

static struct menu *menus = NULL;
static uintmax_t len = 0;
static uintmax_t cap = 0;
static BOOL menusFinalized = FALSE;

struct menu {
	uiMenu m;
	WCHAR *name;
	struct menuItem *items;
	uintmax_t len;
	uintmax_t cap;
	WORD curID;
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
	HWND *hwnds;
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

	// TODO do we need to get the current state and amend it?
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
		m->items = (struct menuItem *) uiRealloc(m->items, m->cap * sizeof (struct menuItem), "struct menuItem[]");
	}
	item = &(m->items[m->len]);
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
		item->id = m->curID;
		m->curID++;
	}
	// TODO this shouldn't be necessary, but uiRealloc() doesn't yet zero out new bytes
	else
		item->id = 0;

	item->onClicked = defaultOnClicked;

	// TODO this shouldn't be necessary, but uiRealloc() doesn't yet zero out new bytes
	item->disabled = FALSE;
	item->checked = FALSE;
	item->hwnds = NULL;
	item->hmenus = NULL;
	item->len = 0;
	item->cap = 0;

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
	if (len >= cap) {
		cap += grow;
		menus = (struct menu *) uiRealloc(menus, cap * sizeof (struct menu), "struct menu[]");
	}
	m = &menus[len];
	len++;

	m->name = toUTF16(name);

	// TODO this shouldn't be necessary, but uiRealloc() doesn't yet zero out new bytes
	m->items = NULL;
	m->len = 0;
	m->cap = 0;

	// start somewhere safe
	m->curID = 100;

	uiMenu(m)->AppendItem = menuAppendItem;
	uiMenu(m)->AppendCheckItem = menuAppendCheckItem;
	uiMenu(m)->AppendQuitItem = menuAppendQuitItem;
	uiMenu(m)->AppendPreferencesItem = menuAppendPreferencesItem;
	uiMenu(m)->AppendAboutItem = menuAppendAboutItem;
	uiMenu(m)->AppendSeparator = menuAppendSeparator;

	return uiMenu(m);
}
