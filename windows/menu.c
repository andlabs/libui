// 20 april 2015
#include "uipriv_windows.h"

static void appendSeparator(HMENU menu)
{
	if (AppendMenuW(menu, MF_SEPARATOR, 0, NULL) == 0)
		logLastError("error appending separator in appendSeparator()");
}

static void appendTextItem(HMENU menu, const char *text)
{
	WCHAR *wtext;

	wtext = toUTF16(text);
	if (AppendMenuW(menu, MF_STRING, 0, wtext) == 0)
		logLastError("error appending menu item in appendTextItem()");
	uiFree(wtext);
}

static void appendMenuItem(HMENU menu, const uiMenuItem *item)
{
	switch (item->Type) {
	case uiMenuItemTypeCommand:
	case uiMenuItemTypeCheckbox:
		appendTextItem(menu, item->Name);
		return;
	// TODO see if there are stock items for these three
	case uiMenuItemTypeQuit:
		// TODO verify name
		appendSeparator(menu);
		appendTextItem(menu, "Quit");
		return;
	case uiMenuItemTypePreferences:
		// TODO verify name
		appendSeparator(menu);
		appendTextItem(menu, "Preferences");
		return;
	case uiMenuItemTypeAbout:
		// TODO verify name
		appendSeparator(menu);
		appendTextItem(menu, "About");
		return;
	case uiMenuItemTypeSeparator:
		// TODO verify name
		appendSeparator(menu);
		return;
	}
	// TODO complain
}


static HMENU makeMenu(uiMenuItem *items)
{
	HMENU menu;
	const uiMenuItem *i;

	menu = CreatePopupMenu();
	if (menu == NULL)
		logLastError("error creating menu in makeMenu()");
	for (i = items; i->Type != 0; i++)
		appendMenuItem(menu, i);
	return menu;
}

HMENU makeMenubar(void)
{
	HMENU menubar;
	const uiMenu *m;
	WCHAR *wname;
	HMENU menu;

	if (options.Menu == NULL)
		complain("asked to give uiWindow a menubar but didn't specify a menu in uiInitOptions");

	menubar = CreateMenu();
	if (menubar == NULL)
		logLastError("error creating menubar in makeMenubar()");

	for (m = options.Menu; m->Name != NULL; m++) {
		wname = toUTF16(m->Name);
		menu = makeMenu(m->Items);
		if (AppendMenuW(menubar, MF_POPUP | MF_STRING, (UINT_PTR) menu, wname) == 0)
			logLastError("error appending menu to menubar in makeMenubar()");
		uiFree(wname);
	}

	return menubar;
}
