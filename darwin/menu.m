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

	switch (item->Type) {
	case uiMenuItemTypeCommand:
	case uiMenuItemTypeCheckbox:
		mitem = [[NSMenuItem alloc] initWithTitle:toNSString(item->Name) action:NULL keyEquivalent:@""];
		[menu addItem:mitem];
		return;
	case uiMenuItemTypeQuit:
		// TODO verify name
		menuConfig->hasQuit = YES;
		return;
	case uiMenuItemTypePreferences:
		// TODO verify name
		menuConfig->hasPreferences = YES;
		return;
	case uiMenuItemTypeAbout:
		// TODO verify name
		menuConfig->hasAbout = YES;
		return;
	case uiMenuItemTypeSeparator:
		// TODO verify name
		[menu addItem:[NSMenuItem separatorItem]];
		return;
	}
	// TODO complain
}

NSMenuItem *makeMenu(const char *name, const uiMenuItem *items, struct menuConfig *menuConfig)
{
	NSMenuItem *menubarItem;
	NSMenu *menu;
	const uiMenuItem *i;

	menubarItem = [[NSMenuItem alloc] initWithTitle:toNSString(name) action:NULL keyEquivalent:@""];
	menu = [[NSMenu alloc] initWithTitle:toNSString(name)];
	for (i = items; i->Type != 0; i++)
		appendMenuItem(menu, i, menuConfig);
	[menubarItem setSubmenu:menu];
	return menubarItem;
}

NSMenu *makeMenubar(void)
{
	NSMenu *menubar;
	NSMenuItem *applicationMenuItem;
	NSMenu *applicationMenu;
	struct menuConfig menuConfig;
	const uiMenu *m;

	menubar = [[NSMenu alloc] initWithTitle:@""];

	// TODO give the application name?
	applicationMenuItem = [[NSMenuItem alloc] initWithTitle:@"" action:NULL keyEquivalent:@""];
	applicationMenu = [[NSMenu alloc] initWithTitle:@""];
	[applicationMenuItem setSubmenu:applicationMenu];
	[menubar addItem:applicationMenuItem];

	memset(&menuConfig, 0, sizeof (struct menuConfig));
	for (m = options.Menu; m->Name != NULL; m++)
		[menubar addItem:makeMenu(m->Name, m->Items, &menuConfig)];

	return menubar;
}
