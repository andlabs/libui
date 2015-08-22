// 28 april 2015
#import "uipriv_darwin.h"

static NSMutableArray *menus = nil;
static BOOL menusFinalized = NO;

struct uiMenu {
	uiTyped t;
	NSMenu *menu;
	NSMenuItem *item;
	NSMutableArray *items;
};

struct uiMenuItem {
	uiTyped t;
	NSMenuItem *item;
	int type;
	BOOL disabled;
	void (*onClicked)(uiMenuItem *, uiWindow *, void *);
	void *onClickedData;
};

enum {
	typeRegular,
	typeCheckbox,
	typeQuit,
	typePreferences,
	typeAbout,
	typeSeparator,
};

@implementation menuManager

- (id)init
{
	self = [super init];
	if (self) {
		self->items = newMap();
		self->hasQuit = NO;
		self->hasPreferences = NO;
		self->hasAbout = NO;
	}
	return self;
}

- (void)dealloc
{
	[self->items release];
	[super dealloc];
}

- (IBAction)onClicked:(id)sender
{
	uiMenuItem *item;

	item = (uiMenuItem *) mapGet(self->items, sender);
	if (item->type == typeCheckbox)
		uiMenuItemSetChecked(uiMenuItem(item), !uiMenuItemChecked(uiMenuItem(item)));
	// use the key window as the source of the menu event; it's the active window
	(*(item->onClicked))(uiMenuItem(item), windowFromNSWindow([realNSApp() keyWindow]), item->onClickedData);
}

- (IBAction)onQuitClicked:(id)sender
{
	if (shouldQuit())
		uiQuit();
}

- (void)register:(NSMenuItem *)item to:(uiMenuItem *)smi
{
	NSValue *v;

	switch (smi->type) {
	case typeQuit:
		if (self->hasQuit)
			complain("attempt to add multiple Quit menu items");
		self->hasQuit = YES;
		break;
	case typePreferences:
		if (self->hasPreferences)
			complain("attempt to add multiple Preferences menu items");
		self->hasPreferences = YES;
		break;
	case typeAbout:
		if (self->hasAbout)
			complain("attempt to add multiple About menu items");
		self->hasAbout = YES;
		break;
	}
	mapSet(self->items, item, smi);
}

// on OS X there are two ways to handle menu items being enabled or disabled: automatically and manually
// unfortunately, the application menu requires automatic menu handling for the Hide, Hide Others, and Show All items to work correctly
// therefore, we have to handle enabling of the other options ourselves
- (BOOL)validateMenuItem:(NSMenuItem *)item
{
	uiMenuItem *smi;

	// disable the special items if they aren't present
	if (item == self.quitItem && !self->hasQuit)
		return NO;
	if (item == self.preferencesItem && !self->hasPreferences)
		return NO;
	if (item == self.aboutItem && !self->hasAbout)
		return NO;
	// then poll the item's enabled/disabled state
	smi = (uiMenuItem *) mapGet(self->items, item);
	return !smi->disabled;
}

// Cocoa constructs the default application menu by hand for each program; that's what MainMenu.[nx]ib does
- (void)buildApplicationMenu:(NSMenu *)menubar
{
	NSString *appName;
	NSMenuItem *appMenuItem;
	NSMenu *appMenu;
	NSMenuItem *item;
	NSString *title;
	NSMenu *servicesMenu;

	appName = [[NSProcessInfo processInfo] processName];
	appMenuItem = [[NSMenuItem alloc] initWithTitle:appName action:NULL keyEquivalent:@""];
	appMenu = [[NSMenu alloc] initWithTitle:appName];
	[appMenuItem setSubmenu:appMenu];
	[menubar addItem:appMenuItem];

	// first is About
	title = [@"About " stringByAppendingString:appName];
	item = [[NSMenuItem alloc] initWithTitle:title action:@selector(onClicked:) keyEquivalent:@""];
	[item setTarget:self];
	[appMenu addItem:item];
	self.aboutItem = item;

	[appMenu addItem:[NSMenuItem separatorItem]];

	// next is Preferences
	item = [[NSMenuItem alloc] initWithTitle:@"Preferences…" action:@selector(onClicked:) keyEquivalent:@","];
	[item setTarget:self];
	[appMenu addItem:item];
	self.preferencesItem = item;

	[appMenu addItem:[NSMenuItem separatorItem]];

	// next is Services
	item = [[NSMenuItem alloc] initWithTitle:@"Services" action:NULL keyEquivalent:@""];
	servicesMenu = [[NSMenu alloc] initWithTitle:@"Services"];
	[item setSubmenu:servicesMenu];
	[realNSApp() setServicesMenu:servicesMenu];
	[appMenu addItem:item];

	[appMenu addItem:[NSMenuItem separatorItem]];

	// next are the three hiding options
	title = [@"Hide " stringByAppendingString:appName];
	item = [[NSMenuItem alloc] initWithTitle:title action:@selector(hide:) keyEquivalent:@"h"];
	// the .xib file says they go to -1 ("First Responder", which sounds wrong...)
	// to do that, we simply leave the target as nil
	[appMenu addItem:item];
	item = [[NSMenuItem alloc] initWithTitle:@"Hide Others" action:@selector(hideOtherApplications:) keyEquivalent:@"h"];
	[item setKeyEquivalentModifierMask:(NSAlternateKeyMask | NSCommandKeyMask)];
	[appMenu addItem:item];
	item = [[NSMenuItem alloc] initWithTitle:@"Show All" action:@selector(unhideAllApplications:) keyEquivalent:@""];
	[appMenu addItem:item];

	[appMenu addItem:[NSMenuItem separatorItem]];

	// and finally Quit
	// DON'T use @selector(terminate:) as the action; we handle termination ourselves
	title = [@"Quit " stringByAppendingString:appName];
	item = [[NSMenuItem alloc] initWithTitle:title action:@selector(onQuitClicked:) keyEquivalent:@"q"];
	[item setTarget:self];
	[appMenu addItem:item];
	self.quitItem = item;
}

- (NSMenu *)makeMenubar
{
	NSMenu *menubar;

	menubar = [[NSMenu alloc] initWithTitle:@""];
	[self buildApplicationMenu:menubar];
	return menubar;
}

@end

static void defaultOnClicked(uiMenuItem *item, uiWindow *w, void *data)
{
	// do nothing
}

void uiMenuItemEnable(uiMenuItem *item)
{
	item->disabled = NO;
	// we don't need to explicitly update the menus here; they'll be updated the next time they're opened (thanks mikeash in irc.freenode.net/#macdev)
}

void uiMenuItemDisable(uiMenuItem *item)
{
	item->disabled = YES;
}

void uiMenuItemOnClicked(uiMenuItem *item, void (*f)(uiMenuItem *, uiWindow *, void *), void *data)
{
	if (item->type == typeQuit)
		complain("attempt to call uiMenuItemOnClicked() on a Quit item; use uiOnShouldQuit() instead");
	item->onClicked = f;
	item->onClickedData = data;
}

int uiMenuItemChecked(uiMenuItem *item)
{
	return [item->item state] != NSOffState;
}

void uiMenuItemSetChecked(uiMenuItem *item, int checked)
{
	NSInteger state;

	state = NSOffState;
	if ([item->item state] == NSOffState)
		state = NSOnState;
	[item->item setState:state];
}

static uiMenuItem *newItem(uiMenu *m, int type, const char *name)
{
	uiMenuItem *item;

	if (menusFinalized)
		complain("attempt to create a new menu item after menus have been finalized");

	item = uiNew(uiMenuItem);
	uiTyped(item)->Type = uiMenuItemType();

	item->type = type;
	switch (item->type) {
	case typeQuit:
		item->item = appDelegate().menuManager.quitItem;
		break;
	case typePreferences:
		item->item = appDelegate().menuManager.preferencesItem;
		break;
	case typeAbout:
		item->item = appDelegate().menuManager.aboutItem;
		break;
	case typeSeparator:
		item->item = [NSMenuItem separatorItem];
		[m->menu addItem:item->item];
		break;
	default:
		item->item = [[NSMenuItem alloc] initWithTitle:toNSString(name) action:@selector(onClicked:) keyEquivalent:@""];
		[item->item setTarget:appDelegate().menuManager];
		[m->menu addItem:item->item];
		break;
	}

	[appDelegate().menuManager register:item->item to:item];
	item->onClicked = defaultOnClicked;

	[m->items addObject:[NSValue valueWithPointer:item]];

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
	// duplicate check is in the register:to: selector
	return newItem(m, typeQuit, NULL);
}

uiMenuItem *uiMenuAppendPreferencesItem(uiMenu *m)
{
	// duplicate check is in the register:to: selector
	return newItem(m, typePreferences, NULL);
}

uiMenuItem *uiMenuAppendAboutItem(uiMenu *m)
{
	// duplicate check is in the register:to: selector
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
	if (menus == nil)
		menus = [NSMutableArray new];

	m = uiNew(uiMenu);
	uiTyped(m)->Type = uiMenuType();

	m->menu = [[NSMenu alloc] initWithTitle:toNSString(name)];
	// use automatic menu item enabling for all menus for consistency's sake

	m->item = [[NSMenuItem alloc] initWithTitle:toNSString(name) action:NULL keyEquivalent:@""];
	[m->item setSubmenu:m->menu];

	m->items = [NSMutableArray new];

	[[realNSApp() mainMenu] addItem:m->item];

	[menus addObject:[NSValue valueWithPointer:m]];

	return m;
}

void finalizeMenus(void)
{
	menusFinalized = YES;
}

void uninitMenus(void)
{
	if (menus == NULL)
		return;
	// don't worry about the actual NSMenus and NSMenuItems; they'll be freed when we clean up the NSApplication
	[menus enumerateObjectsUsingBlock:^(id obj, NSUInteger index, BOOL *stop) {
		NSValue *v;
		uiMenu *m;

		v = (NSValue *) obj;
		m = (uiMenu *) [v pointerValue];
		[m->items enumerateObjectsUsingBlock:^(id obj, NSUInteger index, BOOL *stop) {
			NSValue *v;
			uiMenuItem *mi;

			v = (NSValue *) obj;
			mi = (uiMenuItem *) [v pointerValue];
			uiFree(mi);
		}];
		[m->items release];
		uiFree(m);
	}];
	[menus release];
}
