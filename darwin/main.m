// 20 april 2019
#import "uipriv_darwin.h"

@interface uiprivApplication : NSApplication
@end

@implementation uiprivApplication
@end

@interface uiprivApplicationDelegate : NSObject<NSApplicationDelegate>
@end

static uiprivApplication *uiprivApp;
static uiprivApplicationDelegate *uiprivAppDelegate;

@implementation uiprivApplicationDelegate

#if 0
- (NSApplicationTerminateReply)applicationShouldTerminate:(NSApplication *)app
{
	// for debugging
	NSLog(@"in applicationShouldTerminate:");
	if (uiprivShouldQuit()) {
		canQuit = YES;
		// this will call terminate:, which is the same as uiQuit()
		return NSTerminateNow;
	}
	return NSTerminateCancel;
}
#endif

- (BOOL)applicationShouldTerminateAfterLastWindowClosed:(NSApplication *)app
{
	return NO;
}

@end

#define errNSAppAlreadyInitialized "NSApp is not of type uiprivApplication; was likely already initialized beforehand"

static const char *initErrors[] = {
	errNSAppAlreadyInitialized,
	NULL,
};

int uiInit(void *options, uiInitError *err)
{
	if (!uiprivInitCheckParams(options, err, initErrors))
		return 0;

	uiprivApp = [uiprivApplication sharedApplication];
	if (![NSApp isKindOfClass:[uiprivApplication class]])
		return uiprivInitReturnError(err, errNSAppAlreadyInitialized);

	// don't check for a NO return; something (launch services?) causes running from application bundles to always return NO when asking to change activation policy, even if the change is to the same activation policy!
	// see https://github.com/andlabs/ui/issues/6
	[uiprivApp setActivationPolicy:NSApplicationActivationPolicyRegular];

	uiprivAppDelegate = [uiprivApplicationDelegate new];
	[uiprivApp setDelegate:uiprivAppDelegate];

	uiprivMarkInitialized();
	return 1;
}

void uiMain(void)
{
	[uiprivApp run];
}

void uiQuit(void)
{
	[uiprivApp stop:uiprivApp];
}

// thanks to mikeash in irc.freenode.net/#macdev for suggesting the use of Grand Central Dispatch for this
void uiQueueMain(void (*f)(void *data), void *data)
{
	// dispatch_get_main_queue() is a serial queue so it will not execute multiple uiQueueMain() functions concurrently
	// the signature of f matches dispatch_function_t
	dispatch_async_f(dispatch_get_main_queue(), data, f);
}
