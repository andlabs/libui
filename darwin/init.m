// 6 april 2015
#import "uipriv_darwin.h"

@interface uiApplication : NSApplication
@end

@implementation uiApplication

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

	// TODO figure out what to do here
}

@end

@implementation appDelegate

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

// we are not in control of the actual lifetimes and refcounts of NSViews (see http://stackoverflow.com/a/29523141/3408572)
// when we're done with a view, it'll be added as a subview of this one, and this one will be released on application shutdown
// we need this separate view because it's possible for controls to have no parent but still be alive
NSView *destroyedControlsView;

uiInitOptions options;

const char *uiInit(uiInitOptions *o)
{
	options = *o;
	[uiApplication sharedApplication];
	// don't check for a NO return; something (launch services?) causes running from application bundles to always return NO when asking to change activation policy, even if the change is to the same activation policy!
	// see https://github.com/andlabs/ui/issues/6
	[NSApp setActivationPolicy:NSApplicationActivationPolicyRegular];
	[NSApp setDelegate:[appDelegate new]];

	// always do this so we always have an application menu
	appDelegate().menuManager = [menuManager new];
	[NSApp setMainMenu:[appDelegate().menuManager makeMenubar]];

	// we can use a stock NSView for this
	destroyedControlsView = [[NSView alloc] initWithFrame:NSZeroRect];

	return NULL;
}

void uiFreeInitError(const char *err)
{
}
