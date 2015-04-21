// 20 april 2015
#include "uipriv_windows.h"

static void appendSeparator(HMENU menu)
{
	if (AppendMenuW(menu, MF_SEPARATOR, 0, NULL) == 0)
		logLastError("error appending separator in appendSeparator()");
}

static void appendTextItem(HMENU menu, const char *text, UINT_PTR *id)
{
	WCHAR *wtext;

	wtext = toUTF16(text);
	if (AppendMenuW(menu, MF_STRING, *id, wtext) == 0)
		logLastError("error appending menu item in appendTextItem()");
	uiFree(wtext);
	(*id)++;
}

static void appendMenuItem(HMENU menu, const uiMenuItem *item, UINT_PTR *id)
{
	switch (item->Type) {
	case uiMenuItemTypeCommand:
	case uiMenuItemTypeCheckbox:
		appendTextItem(menu, item->Name, id);
		return;
	// TODO see if there are stock items for these three
	case uiMenuItemTypeQuit:
		// TODO verify name
		appendSeparator(menu);
		appendTextItem(menu, "Quit", id);
		return;
	case uiMenuItemTypePreferences:
		// TODO verify name
		appendSeparator(menu);
		appendTextItem(menu, "Preferences", id);
		return;
	case uiMenuItemTypeAbout:
		// TODO verify name
		appendSeparator(menu);
		appendTextItem(menu, "About", id);
		return;
	case uiMenuItemTypeSeparator:
		// TODO verify name
		appendSeparator(menu);
		return;
	}
	// TODO complain
}


static HMENU makeMenu(uiMenuItem *items, UINT_PTR *id)
{
	HMENU menu;
	const uiMenuItem *i;

	menu = CreatePopupMenu();
	if (menu == NULL)
		logLastError("error creating menu in makeMenu()");
	for (i = items; i->Type != 0; i++)
		appendMenuItem(menu, i, id);
	return menu;
}

HMENU makeMenubar(void)
{
	HMENU menubar;
	const uiMenu *m;
	WCHAR *wname;
	HMENU menu;
	UINT_PTR id;

	if (options.Menu == NULL)
		complain("asked to give uiWindow a menubar but didn't specify a menu in uiInitOptions");

	menubar = CreateMenu();
	if (menubar == NULL)
		logLastError("error creating menubar in makeMenubar()");

	id = 100;		// start at a safe number
	for (m = options.Menu; m->Name != NULL; m++) {
		wname = toUTF16(m->Name);
		menu = makeMenu(m->Items, &id);
		if (AppendMenuW(menubar, MF_POPUP | MF_STRING, (UINT_PTR) menu, wname) == 0)
			logLastError("error appending menu to menubar in makeMenubar()");
		uiFree(wname);
	}

	return menubar;
}

// this is slow, but it will do for now
// TODO investigate faster options

static const uiMenuItem *lookupID(const uiMenuItem *items, UINT_PTR *cur, UINT_PTR id)
{
	const uiMenuItem *i;

	for (i = items; i->Type != 0; i++) {
		if (i->Type == uiMenuItemTypeSeparator)
			continue;
		if (*cur == id)
			return i;
		(*cur)++;
	}
	return NULL;
}

const uiMenuItem *menuIDToItem(UINT_PTR id)
{
	UINT_PTR cur;
	const uiMenu *m;
	const uiMenuItem *item;

	cur = 100;
	for (m = options.Menu; m->Name != NULL; m++) {
		item = lookupID(m->Items, &cur, id);
		if (item != NULL)
			return item;
	}
	// TODO complain
	return NULL;
}
