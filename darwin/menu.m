// 20 april 2015
#import "uipriv_darwin.h"

struct menuConfig {
	BOOL hasQuit;
	BOOL hasPreferences;
	BOOL hasAbout;
};

static NSMenuItem *quitItem;
static NSMenuItem *preferencesItem;
static NSMenuItem *aboutItem;

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
	struct menuConfig menuConfig;
	const uiMenu *m;

	menubar = [[NSMenu alloc] initWithTitle:@""];

	// always build the application menu
	buildApplicationMenu(menubar);

	memset(&menuConfig, 0, sizeof (struct menuConfig));
	if (options.Menu != NULL)
		for (m = options.Menu; m->Name != NULL; m++)
			[menubar addItem:makeMenu(m->Name, m->Items, &menuConfig)];

	if (menuConfig.hasQuit)
		[quitItem setEnabled:YES];
	if (menuConfig.hasPreferences)
		[preferencesItem setEnabled:YES];
	if (menuConfig.hasAbout)
		[aboutItem setEnabled:YES];

	return menubar;
}
