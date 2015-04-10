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
}

@end

// TODO applicationShouldTerminateAfterLastWindowClosed

uiInitOptions options;

uiInitError *uiInit(uiInitOptions *o)
{
	options = *o;
	[uiApplication sharedApplication];
	// don't check for a NO return; something (launch services?) causes running from application bundles to always return NO when asking to change activation policy, even if the change is to the same activation policy!
	// see https://github.com/andlabs/ui/issues/6
	[NSApp setActivationPolicy:NSApplicationActivationPolicyRegular];
	return NULL;
}

const char *uiInitErrorMessage(uiInitError *err)
{
	return "";
}

void uiInitErrorFree(uiInitError *err)
{
}
