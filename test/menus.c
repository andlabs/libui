// 23 april 2015
#include "test.h"

uiMenu *fileMenu;
uiMenuItem *newItem;
uiMenuItem *openItem;
uiMenuItem *quitItem;
uiMenu *editMenu;
uiMenuItem *undoItem;
uiMenuItem *checkItem;
uiMenuItem *accelItem;
uiMenuItem *prefsItem;
uiMenu *testMenu;
uiMenuItem *enabledItem;
uiMenuItem *enableThisItem;
uiMenuItem *forceCheckedItem;
uiMenuItem *forceUncheckedItem;
uiMenu *helpMenu;
uiMenuItem *helpItem;
uiMenuItem *aboutItem;

static void enableItemTest(uiMenuItem *item, uiWindow *w, void *data)
{
	if (uiMenuItemChecked(enabledItem))
		uiMenuItemEnable(enableThisItem);
	else
		uiMenuItemDisable(enableThisItem);
}

static void forceOn(uiMenuItem *item, uiWindow *w, void *data)
{
	uiMenuItemSetChecked(enabledItem, 1);
}

static void forceOff(uiMenuItem *item, uiWindow *w, void *data)
{
	uiMenuItemSetChecked(enabledItem, 0);
}

void initMenus(void)
{
	fileMenu = uiNewMenu("File");
	newItem = uiMenuAppendItem(fileMenu, "New");
	openItem = uiMenuAppendItem(fileMenu, "Open");
	quitItem = uiMenuAppendQuitItem(fileMenu);

	editMenu = uiNewMenu("Edit");
	undoItem = uiMenuAppendItem(editMenu, "Undo");
	uiMenuItemDisable(undoItem);
	uiMenuAppendSeparator(editMenu);
	checkItem = uiMenuAppendCheckItem(editMenu, "Check Me\tTest");
	accelItem = uiMenuAppendItem(editMenu, "A&ccele&&rator T_es__t");
	prefsItem = uiMenuAppendPreferencesItem(editMenu);

	testMenu = uiNewMenu("Test");
	enabledItem = uiMenuAppendCheckItem(testMenu, "Enable Below Item");
	uiMenuItemSetChecked(enabledItem, 1);
	uiMenuItemOnClicked(enabledItem, enableItemTest, NULL);
	enableThisItem = uiMenuAppendItem(testMenu, "This Will Be Enabled");
	forceCheckedItem = uiMenuAppendItem(testMenu, "Force Above Checked");
	uiMenuItemOnClicked(forceCheckedItem, forceOn, NULL);
	forceUncheckedItem = uiMenuAppendItem(testMenu, "Force Above Unchecked");
	uiMenuItemOnClicked(forceUncheckedItem, forceOff, NULL);

	helpMenu = uiNewMenu("Help");
	helpItem = uiMenuAppendItem(helpMenu, "Help");
	aboutItem = uiMenuAppendAboutItem(helpMenu);
}
