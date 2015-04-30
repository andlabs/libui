// 28 april 2015
#import "uipriv_darwin.h"

// general TODO: release Objective-C objects in dealloc since we can't use ARC

struct menu {
	uiMenu m;
	NSMenu *menu;
	NSMenuItem *item;
};

struct menuItem {
	uiMenuItem mi;
	NSMenuItem *item;
	int type;
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
	if (self)
		self->items = [NSMapTable mapTableWithKeyOptions:NSPointerFunctionsOpaqueMemory
			valueOptions:NSPointerFunctionsOpaqueMemory];
	return self;
}

- (void)dealloc
{
	[self->items release];
	[super dealloc];
}

// TODO make sure this is the right signature
- (IBAction)onClicked:(id)sender
{
	struct menuItem *item;
	NSValue *v;

	v = (NSValue *) [self->items objectForKey:sender];
	item = (struct menuItem *) [v pointerValue];
	if (item->type == typeCheckbox)
		uiMenuItemSetChecked(uiMenuItem(item), !uiMenuItemChecked(uiMenuItem(item)));
	// TODO get key window
	(*(item->onClicked))(uiMenuItem(item), NULL, item->onClickedData);
}

- (void)register:(NSMenuItem *)item to:(struct menuItem *)smi
{
	NSValue *v;

	v = [NSValue valueWithPointer:smi];
	[self->items setObject:v forKey:item];
}

// Cocoa constructs the default application menu by hand for each program; that's what MainMenu.[nx]ib does
// TODO investigate setAppleMenu:
- (void)buildApplicationMenu:(NSMenu *)menubar
{
	NSString *appName;
	NSMenuItem *appMenuItem;
	NSMenu *appMenu;
	NSMenuItem *item;
	NSString *title;

	appName = [[NSProcessInfo processInfo] processName];
	appMenuItem = [[NSMenuItem alloc] initWithTitle:appName action:NULL keyEquivalent:@""];
	appMenu = [[NSMenu alloc] initWithTitle:appName];
	// TODO see if this is safe for the items we don't care about
	[appMenu setAutoenablesItems:NO];
	[appMenuItem setSubmenu:appMenu];
	[menubar addItem:appMenuItem];

	// first is About
	title = [@"About " stringByAppendingString:appName];
	item = [[NSMenuItem alloc] initWithTitle:title action:@selector(onClicked:) keyEquivalent:@""];
	[item setTarget:self];
	[item setEnabled:NO];
	[appMenu addItem:item];
	self.aboutItem = item;

	[appMenu addItem:[NSMenuItem separatorItem]];

	// next is Preferences
	item = [[NSMenuItem alloc] initWithTitle:@"Preferences…" action:@selector(onClicked:) keyEquivalent:@","];
	[item setTarget:self];
	[item setEnabled:NO];
	[appMenu addItem:item];
	self.preferencesItem = item;

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
	item = [[NSMenuItem alloc] initWithTitle:title action:@selector(onClicked:) keyEquivalent:@"q"];
	[item setTarget:self];
//TODO	[item setEnabled:NO];
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

static void menuItemEnable(uiMenuItem *ii)
{
	struct menuItem *item = (struct menuItem *) ii;

	[item->item setEnabled:YES];
}

static void menuItemDisable(uiMenuItem *ii)
{
	struct menuItem *item = (struct menuItem *) ii;

	[item->item setEnabled:NO];
}

static void menuItemOnClicked(uiMenuItem *ii, void (*f)(uiMenuItem *, uiWindow *, void *), void *data)
{
	struct menuItem *item = (struct menuItem *) ii;

	item->onClicked = f;
	item->onClickedData = data;
}

static int menuItemChecked(uiMenuItem *ii)
{
	struct menuItem *item = (struct menuItem *) ii;

	return [item->item state] != NSOffState;
}

static void menuItemSetChecked(uiMenuItem *ii, int checked)
{
	struct menuItem *item = (struct menuItem *) ii;
	NSInteger state;

	state = NSOffState;
	if ([item->item state] == NSOffState)
		state = NSOnState;
	[item->item setState:state];
}

static uiMenuItem *newItem(struct menu *m, int type, const char *name)
{
	struct menuItem *item;

	item = uiNew(struct menuItem);

	item->type = type;
	switch (item->type) {
	case typeQuit:
		item->item = appDelegate().menuManager.quitItem;
		[item->item setEnabled:YES];
		break;
	case typePreferences:
		item->item = appDelegate().menuManager.preferencesItem;
		[item->item setEnabled:YES];
		break;
	case typeAbout:
		item->item = appDelegate().menuManager.aboutItem;
		[item->item setEnabled:YES];
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

	uiMenuItem(item)->Enable = menuItemEnable;
	uiMenuItem(item)->Disable = menuItemDisable;
	uiMenuItem(item)->OnClicked = menuItemOnClicked;
	uiMenuItem(item)->Checked = menuItemChecked;
	uiMenuItem(item)->SetChecked = menuItemSetChecked;

	return uiMenuItem(item);
}

uiMenuItem *menuAppendItem(uiMenu *mm, const char *name)
{
	return newItem((struct menu *) mm, typeRegular, name);
}

uiMenuItem *menuAppendCheckItem(uiMenu *mm, const char *name)
{
	return newItem((struct menu *) mm, typeCheckbox, name);
}

uiMenuItem *menuAppendQuitItem(uiMenu *mm)
{
	// TODO check multiple quit items
	// TODO conditionally add separator
	newItem((struct menu *) mm, typeSeparator, NULL);
	return newItem((struct menu *) mm, typeQuit, NULL);
}

uiMenuItem *menuAppendPreferencesItem(uiMenu *mm)
{
	// TODO check multiple preferences items
	// TODO conditionally add separator
	newItem((struct menu *) mm, typeSeparator, NULL);
	return newItem((struct menu *) mm, typePreferences, NULL);
}

uiMenuItem *menuAppendAboutItem(uiMenu *mm)
{
	// TODO check multiple about items
	// TODO conditionally add separator
	newItem((struct menu *) mm, typeSeparator, NULL);
	return newItem((struct menu *) mm, typeAbout, NULL);
}

void menuAppendSeparator(uiMenu *mm)
{
	// TODO check multiple about items
	newItem((struct menu *) mm, typeSeparator, NULL);
}

uiMenu *uiNewMenu(const char *name)
{
	struct menu *m;

	m = uiNew(struct menu);

	m->menu = [[NSMenu alloc] initWithTitle:toNSString(name)];
	[m->menu setAutoenablesItems:NO];

	m->item = [[NSMenuItem alloc] initWithTitle:toNSString(name) action:NULL keyEquivalent:@""];
	[m->item setSubmenu:m->menu];

	[[NSApp mainMenu] addItem:m->item];

	uiMenu(m)->AppendItem = menuAppendItem;
	uiMenu(m)->AppendCheckItem = menuAppendCheckItem;
	uiMenu(m)->AppendQuitItem = menuAppendQuitItem;
	uiMenu(m)->AppendPreferencesItem = menuAppendPreferencesItem;
	uiMenu(m)->AppendAboutItem = menuAppendAboutItem;
	uiMenu(m)->AppendSeparator = menuAppendSeparator;

	return uiMenu(m);
}
