// 28 april 2015
#import "uipriv_darwin.h"

@implementation menuManager

// TODO allocation logger

- (id)init
{
	self = [super init];
	if (self)
		self->items = [NSMutableDictionary new];
	return self;
}

- (void)dealloc
{
	[self->items release];
	[super dealloc];
}

- (IBAction)onClicked:(id)sender
{
	// TODO
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

uiMenu *uiNewMenu(const char *name)
{
	// TODO
	return NULL;
}
