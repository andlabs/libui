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

// Cocoa constructs the default application menu by hand for each program; that's what MainMenu.[nx]ib does
static void buildApplicationMenu(NSMenu *menubar)
{
	NSString *appName;
	NSMenuItem *appMenuItem;
	NSMenu *appMenu;
	NSMenuItem *item;
	NSString *title;

	appName = [[NSProcessInfo processInfo] processName];
	appMenuItem = [[NSMenuItem alloc] initWithTitle:appName action:NULL keyEquivalent:@""];
	appMenu = [[NSMenu alloc] initWithTitle:appName];
	[appMenuItem setSubmenu:appMenu];
	[menubar addItem:appMenuItem];

	// first is About
	title = [@"About " stringByAppendingString:appName];
	item = [[NSMenuItem alloc] initWithTitle:title action:NULL keyEquivalent:@""];
	[item setEnabled:NO];
	[appMenu addItem:item];
	aboutItem = item;

	[appMenu addItem:[NSMenuItem separatorItem]];

	// next is Preferences
	item = [[NSMenuItem alloc] initWithTitle:@"Preferences…" action:NULL keyEquivalent:@","];
	[item setEnabled:NO];
	[appMenu addItem:item];
	preferencesItem = item;

	[appMenu addItem:[NSMenuItem separatorItem]];

	// next is Services
	item = [[NSMenuItem alloc] initWithTitle:@"Services" action:NULL keyEquivalent:@""];
	// TODO build this part

	[appMenu addItem:[NSMenuItem separatorItem]];

	// next are the three hiding options
	title = [@"Hide " stringByAppendingString:appName];
	item = [[NSMenuItem alloc] initWithTitle:title action:@selector(hide:) keyEquivalent:@"h"];
	// TODO set target for all three of these? the .xib file says they go to -1 ("First Responder", which sounds wrong...)
	[appMenu addItem:item];
	item = [[NSMenuItem alloc] initWithTitle:@"Hide Others" action:@selector(hideOtherApplications:) keyEquivalent:@"h"];
	[item setKeyEquivalentModifierMask:(NSAlternateKeyMask | NSCommandKeyMask)];
	[appMenu addItem:item];
	item = [[NSMenuItem alloc] initWithTitle:@"Show All" action:@selector(unhideAllApplications:) keyEquivalent:@""];
	[appMenu addItem:item];

	[appMenu addItem:[NSMenuItem separatorItem]];

	// and finally Quit
	// DON'T use @selector(terminate:) as the action; we handle termination ourselves (TODO figure out how)
	title = [@"Quit " stringByAppendingString:appName];
	item = [[NSMenuItem alloc] initWithTitle:title action:NULL keyEquivalent:@"q"];
	[appMenu addItem:item];
	quitItem = item;
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
