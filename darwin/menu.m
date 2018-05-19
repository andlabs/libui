// 28 april 2015
#import "uipriv_darwin.h"

static NSMutableArray *menus = nil;
static BOOL menusFinalized = NO;

struct uiMenu {
	NSMenu *menu;
	NSMenuItem *item;
	NSMutableArray *items;
};

struct uiMenuItem {
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

static void mapItemReleaser(void *key, void *value)
{
	uiMenuItem *item;
 
	item = (uiMenuItem *) value;
	[item->item release];
}

@implementation uiprivMenuManager

- (id)init
{
	self = [super init];
	if (self) {
		self->items = uiprivNewMap();
		self->hasQuit = NO;
		self->hasPreferences = NO;
		self->hasAbout = NO;
	}
	return self;
}

- (void)dealloc
{
	uiprivMapWalk(self->items, mapItemReleaser);
	uiprivMapReset(self->items);
	uiprivMapDestroy(self->items);
	uiprivUninitMenus();
	[super dealloc];
}

- (IBAction)onClicked:(id)sender
{
	uiMenuItem *item;

	item = (uiMenuItem *) uiprivMapGet(self->items, sender);
	if (item->type == typeCheckbox)
		uiMenuItemSetChecked(item, !uiMenuItemChecked(item));
	// use the key window as the source of the menu event; it's the active window
	(*(item->onClicked))(item, uiprivWindowFromNSWindow([uiprivNSApp() keyWindow]), item->onClickedData);
}

- (IBAction)onQuitClicked:(id)sender
{
	if (uiprivShouldQuit())
		uiQuit();
}

- (void)register:(NSMenuItem *)item to:(uiMenuItem *)smi
{
	switch (smi->type) {
	case typeQuit:
		if (self->hasQuit)
			uiprivUserBug("You can't have multiple Quit menu items in one program.");
		self->hasQuit = YES;
		break;
	case typePreferences:
		if (self->hasPreferences)
			uiprivUserBug("You can't have multiple Preferences menu items in one program.");
		self->hasPreferences = YES;
		break;
	case typeAbout:
		if (self->hasAbout)
			uiprivUserBug("You can't have multiple About menu items in one program.");
		self->hasAbout = YES;
		break;
	}
	uiprivMapSet(self->items, item, smi);
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
	smi = (uiMenuItem *) uiprivMapGet(self->items, item);
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

	// note: no need to call setAppleMenu: on this anymore; see https://developer.apple.com/library/mac/releasenotes/AppKit/RN-AppKitOlderNotes/#X10_6Notes
	appName = [[NSProcessInfo processInfo] processName];
	appMenuItem = [[[NSMenuItem alloc] initWithTitle:appName action:NULL keyEquivalent:@""] autorelease];
	appMenu = [[[NSMenu alloc] initWithTitle:appName] autorelease];
	[appMenuItem setSubmenu:appMenu];
	[menubar addItem:appMenuItem];

	// first is About
	title = [@"About " stringByAppendingString:appName];
	item = [[[NSMenuItem alloc] initWithTitle:title action:@selector(onClicked:) keyEquivalent:@""] autorelease];
	[item setTarget:self];
	[appMenu addItem:item];
	self.aboutItem = item;

	[appMenu addItem:[NSMenuItem separatorItem]];

	// next is Preferences
	item = [[[NSMenuItem alloc] initWithTitle:@"Preferences…" action:@selector(onClicked:) keyEquivalent:@","] autorelease];
	[item setTarget:self];
	[appMenu addItem:item];
	self.preferencesItem = item;

	[appMenu addItem:[NSMenuItem separatorItem]];

	// next is Services
	item = [[[NSMenuItem alloc] initWithTitle:@"Services" action:NULL keyEquivalent:@""] autorelease];
	servicesMenu = [[[NSMenu alloc] initWithTitle:@"Services"] autorelease];
	[item setSubmenu:servicesMenu];
	[uiprivNSApp() setServicesMenu:servicesMenu];
	[appMenu addItem:item];

	[appMenu addItem:[NSMenuItem separatorItem]];

	// next are the three hiding options
	title = [@"Hide " stringByAppendingString:appName];
	item = [[[NSMenuItem alloc] initWithTitle:title action:@selector(hide:) keyEquivalent:@"h"] autorelease];
	// the .xib file says they go to -1 ("First Responder", which sounds wrong...)
	// to do that, we simply leave the target as nil
	[appMenu addItem:item];
	item = [[[NSMenuItem alloc] initWithTitle:@"Hide Others" action:@selector(hideOtherApplications:) keyEquivalent:@"h"] autorelease];
	[item setKeyEquivalentModifierMask:(NSAlternateKeyMask | NSCommandKeyMask)];
	[appMenu addItem:item];
	item = [[[NSMenuItem alloc] initWithTitle:@"Show All" action:@selector(unhideAllApplications:) keyEquivalent:@""] autorelease];
	[appMenu addItem:item];

	[appMenu addItem:[NSMenuItem separatorItem]];

	// and finally Quit
	// DON'T use @selector(terminate:) as the action; we handle termination ourselves
	title = [@"Quit " stringByAppendingString:appName];
	item = [[[NSMenuItem alloc] initWithTitle:title action:@selector(onQuitClicked:) keyEquivalent:@"q"] autorelease];
	[item setTarget:self];
	[appMenu addItem:item];
	self.quitItem = item;
}

- (NSMenu *)makeMenubar
{
	NSMenu *menubar;

	menubar = [[[NSMenu alloc] initWithTitle:@""] autorelease];
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
		uiprivUserBug("You can't call uiMenuItemOnClicked() on a Quit item; use uiOnShouldQuit() instead.");
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
	@autoreleasepool {

	uiMenuItem *item;

	if (menusFinalized)
		uiprivUserBug("You can't create a new menu item after menus have been finalized.");

	item = uiprivNew(uiMenuItem);

	item->type = type;
	switch (item->type) {
	case typeQuit:
		item->item = [uiprivAppDelegate().menuManager.quitItem retain];
		break;
	case typePreferences:
		item->item = [uiprivAppDelegate().menuManager.preferencesItem retain];
		break;
	case typeAbout:
		item->item = [uiprivAppDelegate().menuManager.aboutItem retain];
		break;
	case typeSeparator:
		item->item = [[NSMenuItem separatorItem] retain];
		[m->menu addItem:item->item];
		break;
	default:
		item->item = [[NSMenuItem alloc] initWithTitle:uiprivToNSString(name) action:@selector(onClicked:) keyEquivalent:@""];
		[item->item setTarget:uiprivAppDelegate().menuManager];
		[m->menu addItem:item->item];
		break;
	}

	[uiprivAppDelegate().menuManager register:item->item to:item];
	item->onClicked = defaultOnClicked;

	[m->items addObject:[NSValue valueWithPointer:item]];

	return item;

	} // @autoreleasepool
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
	@autoreleasepool {

	uiMenu *m;

	if (menusFinalized)
		uiprivUserBug("You can't create a new menu after menus have been finalized.");
	if (menus == nil)
		menus = [NSMutableArray new];

	m = uiprivNew(uiMenu);

	m->menu = [[NSMenu alloc] initWithTitle:uiprivToNSString(name)];
	// use automatic menu item enabling for all menus for consistency's sake

	m->item = [[NSMenuItem alloc] initWithTitle:uiprivToNSString(name) action:NULL keyEquivalent:@""];
	[m->item setSubmenu:m->menu];

	m->items = [NSMutableArray new];

	[[uiprivNSApp() mainMenu] addItem:m->item];

	[menus addObject:[NSValue valueWithPointer:m]];

	return m;

	} // @autoreleasepool
}

void uiprivFinalizeMenus(void)
{
	menusFinalized = YES;
}

void uiprivUninitMenus(void)
{
	if (menus == NULL)
		return;
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
			uiprivFree(mi);
		}];
		[m->items release];
		uiprivFree(m);
	}];
	[menus release];
}
