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

const char **uiprivSysInitErrors(void)
{
	return initErrors;
}

static pthread_t mainThread;
static BOOL initialized = NO;		// TODO deduplicate this from common/init.c

bool uiprivSysInit(void *options, uiInitError *err)
{
	uiprivApp = [uiprivApplication sharedApplication];
	if (![NSApp isKindOfClass:[uiprivApplication class]])
		return uiprivInitReturnError(err, errNSAppAlreadyInitialized);

	// don't check for a NO return; something (launch services?) causes running from application bundles to always return NO when asking to change activation policy, even if the change is to the same activation policy!
	// see https://github.com/andlabs/ui/issues/6
	[uiprivApp setActivationPolicy:NSApplicationActivationPolicyRegular];

	uiprivAppDelegate = [uiprivApplicationDelegate new];
	[uiprivApp setDelegate:uiprivAppDelegate];

	mainThread = pthread_self();
	initialized = YES;
	return true;
}

void uiMain(void)
{
	if (!uiprivCheckInitializedAndThread())
		return;
	[uiprivApp run];
}

void uiQuit(void)
{
	if (!uiprivCheckInitializedAndThread())
		return;
	@autoreleasepool {
		NSEvent *e;

		[uiprivApp stop:uiprivApp];
		// stop: won't register until another event has passed; let's synthesize one
		// TODO instead of using NSApplicationDefined, create a private event type for libui internal use only
		e = [NSEvent otherEventWithType:NSApplicationDefined
			location:NSZeroPoint
			modifierFlags:0
			timestamp:[[NSProcessInfo processInfo] systemUptime]
			windowNumber:0
			context:[NSGraphicsContext currentContext]
			subtype:0
			data1:0
			data2:0];
		[uiprivApp postEvent:e atStart:NO];		// let pending events take priority (this is what PostQuitMessage() on Windows does so we have to do it here too for parity; thanks to mikeash in irc.freenode.net/#macdev for confirming that this parameter should indeed be NO)
	}
}

// thanks to mikeash in irc.freenode.net/#macdev for suggesting the use of Grand Central Dispatch for this
void uiQueueMain(void (*f)(void *data), void *data)
{
	if (!initialized) {
		uiprivProgrammerError(uiprivProgrammerErrorNotInitialized, uiprivFunc);
		return;
	}
	// dispatch_get_main_queue() is a serial queue so it will not execute multiple uiQueueMain() functions concurrently
	// the signature of f matches dispatch_function_t
	dispatch_async_f(dispatch_get_main_queue(), data, f);
}

bool uiprivSysCheckThread(void)
{
	return pthread_equal(pthread_self(), mainThread);
}

// Debugger() was deprecated in macOS 10.8 (as part of the larger CarbonCore deprecation), but they did not provide a replacement.
// Though some people say inline asm, I'd rather make this work automatically everywhere.
// Others say raise(SIGTRAP) and others still say __builtin_trap(), but I can't confirm these do what I want (some sources, including documentation, claim they either cause a core dump or send a SIGILL instead).
// I've also heard of some new clang intrinsics, __builtin_debugtrap(), but this is totally undocumented and the original patch for this suggested it would be identical to __builtin_trap(), so...
// Also I cannot figure out how to manually raise EXC_BREAKPOINT.
// So that leaves us with one option: just use Debugger(), turning off the deprecation warnings.
// Also, while we could turn off the deprecation warning temporarily in gcc >= 4.6 and any clang, I'm not sure what minimum version of gcc we need, and I'm not sure if "any clang" is even accurate.
// So instead of juggling version macros, just turn off deprecation warnings at the bottom of this file.
// References:
// - https://stackoverflow.com/questions/37299/xcode-equivalent-of-asm-int-3-debugbreak-halt
// - https://stackoverflow.com/questions/2622017/suppressing-deprecated-warnings-in-xcode
// - https://stackoverflow.com/questions/28166565/detect-gcc-as-opposed-to-msvc-clang-with-macro
// - https://stackoverflow.com/questions/16555585/why-pragma-gcc-diagnostic-push-pop-warning-in-gcc-c
// - possibly others, all on stackoverflow.com (and maybe once on Apple's own forums?); I forget now
static void debugBreak(void);

void uiprivReportError(const char *prefix, const char *msg, const char *suffix, bool internal)
{
	NSExceptionName exceptionName;

	NSLog(@"*** %s: %s. %s", prefix, msg, suffix);
	exceptionName = NSInternalInconsistencyException;
	if (!internal)
		// TODO either find an appropriate exception for each possible message or use a custom exception name
		exceptionName = NSInvalidArgumentException;
	[NSException raise:exceptionName
		format:@"%s: %s", prefix, msg];
	debugBreak();
	abort();		// we shouldn't reach here
}

#ifdef __clang__
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
#else
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#endif
static void debugBreak(void)
{
	Debugger();
}

// DO NOT ADD NEW CODE HERE. IT WILL NOT BE SUBJECT TO DEPRECATION WARNINGS.
// I am making an exception here with Debugger(); see the large comment above.
