// 15 august 2015
#import "uipriv_darwin.h"

#define defaultStyleMask (NSTitledWindowMask | NSClosableWindowMask | NSMiniaturizableWindowMask | NSResizableWindowMask)

struct windowImplData {
	NSWindow *window;
	char *title;
	uiControl *child;
#if 0
	int margined;
	int (*onClosing)(uiWindow *, void *);
	void *onClosingData;
	uiprivSingleChildConstraints constraints;
	void (*onContentSizeChanged)(uiWindow *, void *);
	void *onContentSizeChangedData;
	BOOL suppressSizeChanged;
	int fullscreen;
	int borderless;
#endif
};

#if 0
// skip {

@implementation uiprivNSWindow

- (void)uiprivDoMove:(NSEvent *)initialEvent
{
	uiprivDoManualMove(self, initialEvent);
}

- (void)uiprivDoResize:(NSEvent *)initialEvent on:(uiWindowResizeEdge)edge
{
	uiprivDoManualResize(self, initialEvent, edge);
}

@end

@interface windowDelegateClass : NSObject<NSWindowDelegate> {
	uiprivMap *windows;
}
- (BOOL)windowShouldClose:(id)sender;
- (void)windowDidResize:(NSNotification *)note;
- (void)windowDidEnterFullScreen:(NSNotification *)note;
- (void)windowDidExitFullScreen:(NSNotification *)note;
- (void)registerWindow:(uiWindow *)w;
- (void)unregisterWindow:(uiWindow *)w;
- (uiWindow *)lookupWindow:(NSWindow *)w;
@end

@implementation windowDelegateClass

- (id)init
{
	self = [super init];
	if (self)
		self->windows = uiprivNewMap();
	return self;
}

- (void)dealloc
{
	uiprivMapDestroy(self->windows);
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

- (void)windowDidResize:(NSNotification *)note
{
	uiWindow *w;

	w = [self lookupWindow:((NSWindow *) [note object])];
	if (!w->suppressSizeChanged)
		(*(w->onContentSizeChanged))(w, w->onContentSizeChangedData);
}

- (void)windowDidEnterFullScreen:(NSNotification *)note
{
	uiWindow *w;

	w = [self lookupWindow:((NSWindow *) [note object])];
	if (!w->suppressSizeChanged)
		w->fullscreen = 1;
}

- (void)windowDidExitFullScreen:(NSNotification *)note
{
	uiWindow *w;

	w = [self lookupWindow:((NSWindow *) [note object])];
	if (!w->suppressSizeChanged)
		w->fullscreen = 0;
}

- (void)registerWindow:(uiWindow *)w
{
	uiprivMapSet(self->windows, w->window, w);
	[w->window setDelegate:self];
}

- (void)unregisterWindow:(uiWindow *)w
{
	[w->window setDelegate:nil];
	uiprivMapDelete(self->windows, w->window);
}

- (uiWindow *)lookupWindow:(NSWindow *)w
{
	uiWindow *v;

	v = uiWindow(uiprivMapGet(self->windows, w));
	// this CAN (and IS ALLOWED TO) return NULL, just in case we're called with some OS X-provided window as the key window
	return v;
}

@end

static windowDelegateClass *windowDelegate = nil;

static void removeConstraints(uiWindow *w)
{
	NSView *cv;

	cv = [w->window contentView];
	uiprivSingleChildConstraintsRemove(&(w->constraints), cv);
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
	uiprivSingleChildConstraintsEstablish(&(w->constraints),
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

static void uiWindowChildVisibilityChanged(uiDarwinControl *c)
{
	uiWindow *w = uiWindow(c);

	windowRelayout(w);
}

void uiWindowContentSize(uiWindow *w, int *width, int *height)
{
	NSRect r;

	r = [w->window contentRectForFrameRect:[w->window frame]];
	*width = r.size.width;
	*height = r.size.height;
}

void uiWindowSetContentSize(uiWindow *w, int width, int height)
{
	w->suppressSizeChanged = YES;
	[w->window setContentSize:NSMakeSize(width, height)];
	w->suppressSizeChanged = NO;
}

int uiWindowFullscreen(uiWindow *w)
{
	return w->fullscreen;
}

void uiWindowSetFullscreen(uiWindow *w, int fullscreen)
{
	if (w->fullscreen && fullscreen)
		return;
	if (!w->fullscreen && !fullscreen)
		return;
	w->fullscreen = fullscreen;
	if (w->fullscreen && w->borderless)		// borderless doesn't play nice with fullscreen; don't toggle while borderless
		return;
	w->suppressSizeChanged = YES;
	[w->window toggleFullScreen:w->window];
	w->suppressSizeChanged = NO;
	if (!w->fullscreen && w->borderless)		// borderless doesn't play nice with fullscreen; restore borderless after removing
		[w->window setStyleMask:NSBorderlessWindowMask];
}

void uiWindowOnContentSizeChanged(uiWindow *w, void (*f)(uiWindow *, void *), void *data)
{
	w->onContentSizeChanged = f;
	w->onContentSizeChangedData = data;
}

void uiWindowOnClosing(uiWindow *w, int (*f)(uiWindow *, void *), void *data)
{
	w->onClosing = f;
	w->onClosingData = data;
}

int uiWindowBorderless(uiWindow *w)
{
	return w->borderless;
}

void uiWindowSetBorderless(uiWindow *w, int borderless)
{
	w->borderless = borderless;
	if (w->borderless) {
		// borderless doesn't play nice with fullscreen; wait for later
		if (!w->fullscreen)
			[w->window setStyleMask:NSBorderlessWindowMask];
	} else {
		[w->window setStyleMask:defaultStyleMask];
		// borderless doesn't play nice with fullscreen; restore state
		if (w->fullscreen) {
			w->suppressSizeChanged = YES;
			[w->window toggleFullScreen:w->window];
			w->suppressSizeChanged = NO;
		}
	}
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
	uiprivSingleChildConstraintsSetMargined(&(w->constraints), w->margined);
}

static int defaultOnClosing(uiWindow *w, void *data)
{
	return 0;
}

static void defaultOnPositionContentSizeChanged(uiWindow *w, void *data)
{
	// do nothing
}

// } skip
#endif

static bool windowInit(uiControl *c, void *implData, void *initData)
{
	struct windowImplData *wi = (struct windowImplData *) implData;

	// TODO grab the exact values from Interface Builder
	wi->window = [[NSWindow alloc] initWithContentRect:NSMakeRect(0, 0, 100, 100)
		styleMask:defaultStyleMask
		backing:NSBackingStoreBuffered
		defer:YES];

	wi->title = NULL;
	[wi->window setTitle:@""];

	// do NOT release when closed
	// we manually do this in Free() below
	[wi->window setReleasedWhenClosed:NO];

#if 0
	if (windowDelegate == nil) {
		windowDelegate = [[windowDelegateClass new] autorelease];
		[uiprivDelegates addObject:windowDelegate];
	}
	[windowDelegate registerWindow:w];
	uiWindowOnClosing(w, defaultOnClosing, NULL);
	uiWindowOnContentSizeChanged(w, defaultOnPositionContentSizeChanged, NULL);
#endif

	return true;
}

static void connectChild(struct windowImplData *wi)
{
	// TODO
}

static void disconnectChild(struct windowImplData *wi)
{
	// TODO
}

static void windowFree(uiControl *c, void *implData)
{
	struct windowImplData *wi = (struct windowImplData *) implData;

	if (wi->child != NULL) {
		disconnectChild(wi);
		uiControlFree(wi->child);
		wi->child = NULL;
	}
	if (wi->title != NULL) {
		uiprivFreeUTF8(wi->title);
		wi->title = NULL;
	}
	[wi->window release];
	wi->window = nil;
}

static void windowParentChanging(uiControl *c, void *implData, uiControl *oldParent)
{
	uiprivProgrammerErrorCannotHaveWindowsAsChildren();
}

static void windowParentChanged(uiControl *c, void *implData, uiControl *newParent)
{
	uiprivProgrammerErrorCannotHaveWindowsAsChildren();
}

static id windowHandle(uiControl *c, void *implData)
{
	struct windowImplData *wi = (struct windowImplData *) implData;

	return wi->window;
}

static const uiControlVtable windowVtable = {
	.Size = sizeof (uiControlVtable),
	.Init = windowInit,
	.Free = windowFree,
	.ParentChanging = windowParentChanging,
	.ParentChanged = windowParentChanged,
};

static const uiControlOSVtable windowOSVtable = {
	.Size = sizeof (uiControlOSVtable),
	.Handle = windowHandle,
};

static uint32_t windowType = 0;

uint32_t uiprivSysWindowType(void)
{
	if (windowType == 0)
		windowType = uiRegisterControlType("uiWindow", &windowVtable, &windowOSVtable, sizeof (struct windowImplData));
	return windowType;
}

uiWindow *uiprivSysNewWindow(void)
{
	return (uiWindow *) uiNewControl(uiWindowType(), NULL);
}

const char *uiprivSysWindowTitle(uiWindow *w)
{
	struct windowImplData *wi = (struct windowImplData *) uiControlImplData(uiControl(w));

	if (wi->title == NULL)
		// TODO replace this with a dedicated UTF-8 empty string object
		return "";
	return wi->title;
}

void uiprivSysWindowSetTitle(uiWindow *w, const char *title)
{
	struct windowImplData *wi = (struct windowImplData *) uiControlImplData(uiControl(w));

	if (wi->title != NULL)
		uiprivFreeUTF8(wi->title);
	wi->title = uiprivSanitizeUTF8(title);
	[wi->window setTitle:[NSString stringWithUTF8String:wi->title]];
}

uiControl *uiprivSysWindowChild(uiWindow *w)
{
	struct windowImplData *wi = (struct windowImplData *) uiControlImplData(uiControl(w));

	return wi->child;
}

void uiprivSysWindowSetChild(uiWindow *w, uiControl *child)
{
	struct windowImplData *wi = (struct windowImplData *) uiControlImplData(uiControl(w));

	if (wi->child != NULL)
		disconnectChild(wi);
	wi->child = child;
	if (wi->child != NULL)
		connectChild(wi);
}

#if 0

// utility function for menus
uiWindow *uiprivWindowFromNSWindow(NSWindow *w)
{
	if (w == nil)
		return NULL;
	if (windowDelegate == nil)		// no windows were created yet; we're called with some OS X-provided window
		return NULL;
	return [windowDelegate lookupWindow:w];
}

#endif
