// 6 april 2015
#import "uipriv_darwin.h"

static BOOL canQuit = NO;

@implementation applicationClass

- (void)sendEvent:(NSEvent *)e
{
	if (sendAreaEvents(e) != 0)
		return;
	[super sendEvent:e];
}

// hey look! we're overriding terminate:!
// we're going to make sure we can go back to main() whether Cocoa likes it or not!
// and just how are we going to do that, hm?
// (note: this is called after applicationShouldTerminate:)
- (void)terminate:(id)sender
{
	// yes that's right folks: DO ABSOLUTELY NOTHING.
	// the magic is [NSApp run] will just... stop.

	// well let's not do nothing; let's actually quit our graceful way
	NSEvent *e;

	// for debugging
	NSLog(@"in terminate:");

	if (!canQuit)
		complain("call to [NSApp terminate:] when not ready to terminate");

	[realNSApp() stop:realNSApp()];
	// stop: won't register until another event has passed; let's synthesize one
	e = [NSEvent otherEventWithType:NSApplicationDefined
		location:NSZeroPoint
		modifierFlags:0
		timestamp:[[NSProcessInfo processInfo] systemUptime]
		windowNumber:0
		context:[NSGraphicsContext currentContext]
		subtype:0
		data1:0
		data2:0];
	[realNSApp() postEvent:e atStart:NO];		// let pending events take priority (this is what PostQuitMessage() on Windows does so we have to do it here too for parity; thanks to mikeash in irc.freenode.net/#macdev for confirming that this parameter should indeed be NO)
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
	if (shouldQuit()) {
		canQuit = YES;
		// this will call terminate:, which is the same as uiQuit()
		return NSTerminateNow;
	}
	return NSTerminateCancel;
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
	[realNSApp() setActivationPolicy:NSApplicationActivationPolicyRegular];
	[realNSApp() setDelegate:[appDelegate new]];

	initAlloc();

	// always do this so we always have an application menu
	appDelegate().menuManager = [menuManager new];
	[realNSApp() setMainMenu:[appDelegate().menuManager makeMenubar]];

	return NULL;
}

void uiUninit(void)
{
	uninitMenus();
	[realNSApp() setDelegate:nil];
	[appDelegate() release];
	[realNSApp() release];
	uninitTypes();
	uninitAlloc();
}

void uiFreeInitError(const char *err)
{
}

void uiMain(void)
{
	[realNSApp() run];
}

// TODO make this delayed
void uiQuit(void)
{
	canQuit = YES;
	[realNSApp() terminate:realNSApp()];
}

// thanks to mikeash in irc.freenode.net/#macdev for suggesting the use of Grand Central Dispatch for this
// TODO will dispatch_get_main_queue() break after _CFRunLoopSetCurrent()?
void uiQueueMain(void (*f)(void *data), void *data)
{
	// dispatch_get_main_queue() is a serial queue so it will not execute multiple uiQueueMain() functions concurrently
	// the signature of f matches dispatch_function_t
	dispatch_async_f(dispatch_get_main_queue(), data, f);
}
