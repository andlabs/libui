// 6 april 2015
#import "uipriv_darwin.h"

@interface applicationClass : NSApplication
@end

@implementation applicationClass

// hey look! we're overriding terminate:!
// we're going to make sure we can go back to main() whether Cocoa likes it or not!
// and just how are we going to do that, hm?
// (note: this is called after applicationShouldTerminate:)
- (void)terminate:(id)sender
{
	// yes that's right folks: DO ABSOLUTELY NOTHING.
	// the magic is [NSApp run] will just... stop.

	// for debugging
	NSLog(@"in terminate:");
}

@end

@implementation appDelegate

- (void)dealloc
{
	[self.menuManager release];
	[super dealloc];
}

- (NSApplicationTerminateReply)applicationShouldTerminate:(NSApplication *)app
{
	// for debugging
	NSLog(@"in applicationShouldTerminate:");
	return NSTerminateNow;
}

- (BOOL)applicationShouldTerminateAfterLastWindowClosed:(NSApplication *)app
{
	return NO;
}

@end

uiInitOptions options;

const char *uiInit(uiInitOptions *o)
{
	options = *o;
	[applicationClass sharedApplication];
	// don't check for a NO return; something (launch services?) causes running from application bundles to always return NO when asking to change activation policy, even if the change is to the same activation policy!
	// see https://github.com/andlabs/ui/issues/6
	[NSApp setActivationPolicy:NSApplicationActivationPolicyRegular];
	[NSApp setDelegate:[appDelegate new]];

	initAlloc();

	// always do this so we always have an application menu
	appDelegate().menuManager = [menuManager new];
	[NSApp setMainMenu:[appDelegate().menuManager makeMenubar]];

	return NULL;
}

void uiUninit(void)
{
	uninitMenus();
	// TODO free application delegate
	// TODO free NSApplication resources (main menu, etc.)
	uninitAlloc();
}

void uiFreeInitError(const char *err)
{
}
