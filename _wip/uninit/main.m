// 6 april 2015
#import "uipriv_darwin.h"
#import "attrstr.h"

static BOOL canQuit = NO;
static NSAutoreleasePool *globalPool;
static uiprivApplicationClass *app;
static uiprivAppDelegate *delegate;

static BOOL (^isRunning)(void);
static BOOL stepsIsRunning;

@implementation uiprivApplicationClass

- (void)sendEvent:(NSEvent *)e
{
	if (uiprivSendAreaEvents(e) != 0)
		return;
	[super sendEvent:e];
}

// NSColorPanel always sends changeColor: to the first responder regardless of whether there's a target set on it
// we can override it here (see colorbutton.m)
// thanks to mikeash in irc.freenode.net/#macdev for informing me this is how the first responder chain is initiated
// it turns out NSFontManager also sends changeFont: through this; let's inhibit that here too (see fontbutton.m)
- (BOOL)sendAction:(SEL)sel to:(id)to from:(id)from
{
	if (uiprivColorButtonInhibitSendAction(sel, from, to))
		return NO;
	if (uiprivFontButtonInhibitSendAction(sel, from, to))
		return NO;
	return [super sendAction:sel to:to from:from];
}

// likewise, NSFontManager also sends NSFontPanelValidation messages to the first responder, however it does NOT use sendAction:from:to:!
// instead, it uses this one (thanks swillits in irc.freenode.net/#macdev)
// we also need to override it (see fontbutton.m)
- (id)targetForAction:(SEL)sel to:(id)to from:(id)from
{
	id override;

	if (uiprivFontButtonOverrideTargetForAction(sel, from, to, &override))
		return override;
	return [super targetForAction:sel to:to from:from];
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

	if (!canQuit)
		uiprivImplBug("call to [NSApp terminate:] when not ready to terminate; definitely contact andlabs");

	[uiprivNSApp() stop:uiprivNSApp()];
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
	[uiprivNSApp() postEvent:e atStart:NO];		// let pending events take priority (this is what PostQuitMessage() on Windows does so we have to do it here too for parity; thanks to mikeash in irc.freenode.net/#macdev for confirming that this parameter should indeed be NO)

	// and in case uiMainSteps() was called
	stepsIsRunning = NO;
}

@end

@implementation uiprivAppDelegate

- (void)dealloc
{
	// Apple docs: "Don't Use Accessor Methods in Initializer Methods and dealloc"
	[_menuManager release];
	[super dealloc];
}

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

- (BOOL)applicationShouldTerminateAfterLastWindowClosed:(NSApplication *)app
{
	return NO;
}

@end

uiInitOptions uiprivOptions;

const char *uiInit(uiInitOptions *o)
{
	@autoreleasepool {
		uiprivOptions = *o;
		app = [[uiprivApplicationClass sharedApplication] retain];
		// don't check for a NO return; something (launch services?) causes running from application bundles to always return NO when asking to change activation policy, even if the change is to the same activation policy!
		// see https://github.com/andlabs/ui/issues/6
		[uiprivNSApp() setActivationPolicy:NSApplicationActivationPolicyRegular];
		delegate = [uiprivAppDelegate new];
		[uiprivNSApp() setDelegate:delegate];

		uiprivInitAlloc();
		uiprivLoadFutures();
		uiprivLoadUndocumented();

		// always do this so we always have an application menu
		uiprivAppDelegate().menuManager = [[uiprivMenuManager new] autorelease];
		[uiprivNSApp() setMainMenu:[uiprivAppDelegate().menuManager makeMenubar]];

		uiprivSetupFontPanel();

		uiprivInitUnderlineColors();
	}

	globalPool = [[NSAutoreleasePool alloc] init];

	return NULL;
}

void uiUninit(void)
{
	if (!globalPool)
		uiprivUserBug("You must call uiInit() first!");
	[globalPool release];

	@autoreleasepool {
		uiprivUninitUnderlineColors();
		[delegate release];
		[uiprivNSApp() setDelegate:nil];
		[app release];
		uiprivUninitAlloc();
	}
}

void uiFreeInitError(const char *err)
{
}

void uiMain(void)
{
	isRunning = ^{
		return [uiprivNSApp() isRunning];
	};
	[uiprivNSApp() run];
}

void uiMainSteps(void)
{
	// SDL does this and it seems to be necessary for the menubar to work (see #182)
	[uiprivNSApp() finishLaunching];
	isRunning = ^{
		return stepsIsRunning;
	};
	stepsIsRunning = YES;
}

int uiMainStep(int wait)
{
	uiprivNextEventArgs nea;

	nea.mask = NSAnyEventMask;

	// ProPuke did this in his original PR requesting this
	// I'm not sure if this will work, but I assume it will...
	nea.duration = [NSDate distantPast];
	if (wait)		// but this is normal so it will work
		nea.duration = [NSDate distantFuture];

	nea.mode = NSDefaultRunLoopMode;
	nea.dequeue = YES;

	return uiprivMainStep(&nea, ^(NSEvent *e) {
		return NO;
	});
}

// see also:
// - http://www.cocoawithlove.com/2009/01/demystifying-nsapplication-by.html
// - https://github.com/gnustep/gui/blob/master/Source/NSApplication.m
int uiprivMainStep(uiprivNextEventArgs *nea, BOOL (^interceptEvent)(NSEvent *e))
{
	NSDate *expire;
	NSEvent *e;
	NSEventType type;

	@autoreleasepool {
		if (!isRunning())
			return 0;

		e = [uiprivNSApp() nextEventMatchingMask:nea->mask
			untilDate:nea->duration
			inMode:nea->mode
			dequeue:nea->dequeue];
		if (e == nil)
			return 1;

		type = [e type];
		if (!interceptEvent(e))
			[uiprivNSApp() sendEvent:e];
		[uiprivNSApp() updateWindows];

		// GNUstep does this
		// it also updates the Services menu but there doesn't seem to be a public API for that so
		if (type != NSPeriodic && type != NSMouseMoved)
			[[uiprivNSApp() mainMenu] update];

		return 1;
	}
}

void uiQuit(void)
{
	canQuit = YES;
	[uiprivNSApp() terminate:uiprivNSApp()];
}

// thanks to mikeash in irc.freenode.net/#macdev for suggesting the use of Grand Central Dispatch for this
// LONGTERM will dispatch_get_main_queue() break after _CFRunLoopSetCurrent()?
void uiQueueMain(void (*f)(void *data), void *data)
{
	// dispatch_get_main_queue() is a serial queue so it will not execute multiple uiQueueMain() functions concurrently
	// the signature of f matches dispatch_function_t
	dispatch_async_f(dispatch_get_main_queue(), data, f);
}

@interface uiprivTimerDelegate : NSObject {
        int (*f)(void *data);
        void *data;
}
- (id)initWithCallback:(int (*)(void *))callback data:(void *)callbackData;
- (void)doTimer:(NSTimer *)timer;
@end

@implementation uiprivTimerDelegate

- (id)initWithCallback:(int (*)(void *))callback data:(void *)callbackData
{
        self = [super init];
        if (self) {
                self->f = callback;
                self->data = callbackData;
        }
        return self;
}

- (void)doTimer:(NSTimer *)timer
{
        if (!(*(self->f))(self->data))
                [timer invalidate];
}

@end

void uiTimer(int milliseconds, int (*f)(void *data), void *data)
{
        uiprivTimerDelegate *delegate;

        delegate = [[uiprivTimerDelegate alloc] initWithCallback:f data:data];
        [NSTimer scheduledTimerWithTimeInterval:(milliseconds / 1000.0)
                target:delegate
                selector:@selector(doTimer:)
                userInfo:nil
                repeats:YES];
        [delegate release];
}

// TODO figure out the best way to clean the above up in uiUninit(), if it's even necessary
// TODO that means figure out if timers can still fire without the main loop
