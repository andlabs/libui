// 20 april 2015
#import "uipriv_darwin.h"

struct menuConfig {
	BOOL hasQuit;
	BOOL hasPreferences;
	BOOL hasAbout;
};

void appendMenuItem(NSMenu *menu, const uiMenuItem *item, struct menuConfig *menuConfig)
{
	NSMenuItem *mitem;

	if (item->Name == uiMenuItemQuit) {
		// TODO verify type
		menuConfig->hasQuit = YES;
		return;
	}
	if (item->Name == uiMenuItemPreferences) {
		// TODO verify type
		menuConfig->hasPreferences = YES;
		return;
	}
	if (item->Name == uiMenuItemAbout) {
		// TODO verify type
		menuConfig->hasAbout = YES;
		return;
	}
	if (item->Name == uiMenuItemSeparator) {
		// TODO verify type
		[menu addItem:[NSMenuItem separatorItem]];
		return;
	}
	switch (item->Type) {
	case uiMenuItemTypeCommand:
	case uiMenuItemTypeCheckbox:
		mitem = [[NSMenuItem alloc] initWithTitle:toNSString(item->Name) action:NULL keyEquivalent:@""];
		[menu addItem:mitem];
		return;
	}
}

NSMenuItem *makeMenu(const char *name, const uiMenuItem *items, struct menuConfig *menuConfig)
{
	NSMenuItem *menubarItem;
	NSMenu *menu;
	const uiMenuItem *i;

	menubarItem = [[NSMenuItem alloc] initWithTitle:toNSString(name) action:NULL keyEquivalent:@""];
	menu = [[NSMenu alloc] initWithTitle:toNSString(name)];
	for (i = items; i->Name != NULL; i++)
		appendMenuItem(menu, i, menuConfig);
	[menubarItem setSubmenu:menu];
	return menubarItem;
}

NSMenu *makeMenubar(void)
{
	NSMenu *menubar;
	struct menuConfig menuConfig;
	const uiMenu *m;

	menubar = [[NSMenu alloc] initWithTitle:@""];

	memset(&menuConfig, 0, sizeof (struct menuConfig));
	for (m = options.Menu; m->Name != NULL; m++)
		[menubar addItem:makeMenu(m->Name, m->Items, &menuConfig)];

	return menubar;
}
