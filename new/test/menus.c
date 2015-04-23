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
uiMenu *helpMenu;
uiMenuItem *helpItem;
uiMenuItem *aboutItem;

void initMenus(void)
{
	fileMenu = uiNewMenu("File");
	newItem = uiMenuAppendItem(fileMenu, "New");
	openItem = uiMenuAppendItem(fileMenu, "Open");
	quitItem = uiMenuAppendQuitItem(fileMenu);

	editMenu = uiNewMenu("Edit");
	undoItem = uiMenuAppendItem(editMenu, "Undo");
	uiMenuAppendSeparator(editMenu);
	checkItem = uiMenuAppendCheckItem(editMenu, "Check Me\tTest");
	accelItem = uiMenuAppendItem(editMenu, "A&ccele&&rator T_es__t");
	prefsItem = uiMenuAppendPreferencesItem(editMenu);

	helpMenu = uiNewMenu("Help");
	helpItem = uiMenuAppendItem(helpMenu, "Help");
	aboutItem = uiMenuAppendAboutItem(helpMenu);
}
