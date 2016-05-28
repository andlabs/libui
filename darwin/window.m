// 15 august 2015
#import "uipriv_darwin.h"

struct uiWindow {
	uiDarwinControl c;
	NSWindow *window;
	uiControl *child;
	int margined;
	int (*onClosing)(uiWindow *, void *);
	void *onClosingData;
	struct singleChildConstraints constraints;
};

@interface windowDelegateClass : NSObject<NSWindowDelegate> {
	struct mapTable *windows;
}
- (BOOL)windowShouldClose:(id)sender;
- (void)registerWindow:(uiWindow *)w;
- (void)unregisterWindow:(uiWindow *)w;
- (uiWindow *)lookupWindow:(NSWindow *)w;
@end

@implementation windowDelegateClass

- (id)init
{
	self = [super init];
	if (self)
		self->windows = newMap();
	return self;
}

- (void)dealloc
{
	mapDestroy(self->windows);
	[super dealloc];
}

- (BOOL)windowShouldClose:(id)sender
{
	uiWindow *w;

	w = [self lookupWindow:((NSWindow *) sender)];
	// w should not be NULL; we are only the delegate of registered windows
	if ((*(w->onClosing))(w, w->onClosingData))
		uiControlDestroy(uiControl(w));
	return NO;
}

- (void)registerWindow:(uiWindow *)w
{
	mapSet(self->windows, w->window, w);
	[w->window setDelegate:self];
}

- (void)unregisterWindow:(uiWindow *)w
{
	[w->window setDelegate:nil];
	mapDelete(self->windows, w->window);
}

- (uiWindow *)lookupWindow:(NSWindow *)w
{
	uiWindow *v;

	v = uiWindow(mapGet(self->windows, w));
	// this CAN (and IS ALLOWED TO) return NULL, just in case we're called with some OS X-provided window as the key window
	return v;
}

@end

static windowDelegateClass *windowDelegate = nil;

static void removeConstraints(uiWindow *w)
{
	NSView *cv;

	cv = [w->window contentView];
	singleChildConstraintsRemove(&(w->constraints), cv);
}

static void uiWindowDestroy(uiControl *c)
{
	uiWindow *w = uiWindow(c);

	// hide the window
	[w->window orderOut:w->window];
	removeConstraints(w);
	if (w->child != NULL) {
		uiControlSetParent(w->child, NULL);
		uiDarwinControlSetSuperview(uiDarwinControl(w->child), nil);
		uiControlDestroy(w->child);
	}
	[windowDelegate unregisterWindow:w];
	[w->window release];
	uiFreeControl(uiControl(w));
}

uiDarwinControlDefaultHandle(uiWindow, window)

uiControl *uiWindowParent(uiControl *c)
{
	return NULL;
}

void uiWindowSetParent(uiControl *c, uiControl *parent)
{
	uiUserBugCannotSetParentOnToplevel("uiWindow");
}

static int uiWindowToplevel(uiControl *c)
{
	return 1;
}

static int uiWindowVisible(uiControl *c)
{
	uiWindow *w = uiWindow(c);

	return [w->window isVisible];
}

static void uiWindowShow(uiControl *c)
{
	uiWindow *w = (uiWindow *) c;

	[w->window makeKeyAndOrderFront:w->window];
}

static void uiWindowHide(uiControl *c)
{
	uiWindow *w = (uiWindow *) c;

	[w->window orderOut:w->window];
}

uiDarwinControlDefaultEnabled(uiWindow, window)
uiDarwinControlDefaultEnable(uiWindow, window)
uiDarwinControlDefaultDisable(uiWindow, window)

static void uiWindowSyncEnableState(uiDarwinControl *c, int enabled)
{
	uiWindow *w = uiWindow(c);

	if (uiDarwinShouldStopSyncEnableState(uiDarwinControl(w), enabled))
		return;
	if (w->child != NULL)
		uiDarwinControlSyncEnableState(uiDarwinControl(w->child), enabled);
}

static void uiWindowSetSuperview(uiDarwinControl *c, NSView *superview)
{
	// TODO
}

static void windowRelayout(uiWindow *w)
{
	NSView *childView;
	NSView *contentView;

	removeConstraints(w);
	if (w->child == NULL)
		return;
	childView = (NSView *) uiControlHandle(w->child);
	contentView = [w->window contentView];
	singleChildConstraintsEstablish(&(w->constraints),
		contentView, childView,
		uiDarwinControlHugsTrailingEdge(uiDarwinControl(w->child)),
		uiDarwinControlHugsBottom(uiDarwinControl(w->child)),
		w->margined,
		@"uiWindow");
}

uiDarwinControlDefaultHugsTrailingEdge(uiWindow, window)
uiDarwinControlDefaultHugsBottom(uiWindow, window)

static void uiWindowChildEdgeHuggingChanged(uiDarwinControl *c)
{
	uiWindow *w = uiWindow(c);

	windowRelayout(w);
}

// TODO
uiDarwinControlDefaultHuggingPriority(uiWindow, window)
uiDarwinControlDefaultSetHuggingPriority(uiWindow, window)
// end TODO

char *uiWindowTitle(uiWindow *w)
{
	return uiDarwinNSStringToText([w->window title]);
}

void uiWindowSetTitle(uiWindow *w, const char *title)
{
	[w->window setTitle:toNSString(title)];
}

void uiWindowOnClosing(uiWindow *w, int (*f)(uiWindow *, void *), void *data)
{
	w->onClosing = f;
	w->onClosingData = data;
}

void uiWindowSetChild(uiWindow *w, uiControl *child)
{
	NSView *childView;

	if (w->child != NULL) {
		childView = (NSView *) uiControlHandle(w->child);
		[childView removeFromSuperview];
		uiControlSetParent(w->child, NULL);
	}
	w->child = child;
	if (w->child != NULL) {
		uiControlSetParent(w->child, uiControl(w));
		childView = (NSView *) uiControlHandle(w->child);
		uiDarwinControlSetSuperview(uiDarwinControl(w->child), [w->window contentView]);
		uiDarwinControlSyncEnableState(uiDarwinControl(w->child), uiControlEnabledToUser(uiControl(w)));
	}
	windowRelayout(w);
}

int uiWindowMargined(uiWindow *w)
{
	return w->margined;
}

void uiWindowSetMargined(uiWindow *w, int margined)
{
	w->margined = margined;
	singleChildConstraintsSetMargined(&(w->constraints), w->margined);
}

static int defaultOnClosing(uiWindow *w, void *data)
{
	return 0;
}

uiWindow *uiNewWindow(const char *title, int width, int height, int hasMenubar)
{
	uiWindow *w;

	finalizeMenus();

	uiDarwinNewControl(uiWindow, w);

	w->window = [[NSWindow alloc] initWithContentRect:NSMakeRect(0, 0, (CGFloat) width, (CGFloat) height)
		styleMask:(NSTitledWindowMask | NSClosableWindowMask | NSMiniaturizableWindowMask | NSResizableWindowMask)
		backing:NSBackingStoreBuffered
		defer:YES];
	[w->window setTitle:toNSString(title)];

	// do NOT release when closed
	// we manually do this in uiWindowDestroy() above
	[w->window setReleasedWhenClosed:NO];

	if (windowDelegate == nil) {
		windowDelegate = [[windowDelegateClass new] autorelease];
		[delegates addObject:windowDelegate];
	}
	[windowDelegate registerWindow:w];
	uiWindowOnClosing(w, defaultOnClosing, NULL);

	return w;
}

// utility function for menus
uiWindow *windowFromNSWindow(NSWindow *w)
{
	if (w == nil)
		return NULL;
	if (windowDelegate == nil)		// no windows were created yet; we're called with some OS X-provided window
		return NULL;
	return [windowDelegate lookupWindow:w];
}
