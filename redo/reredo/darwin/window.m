// 15 august 2015
#import "uipriv_darwin.h"

struct uiWindow {
	uiDarwinControl c;
	NSWindow *window;
	uiControl *child;
	int margined;
	int (*onClosing)(uiWindow *, void *);
	void *onClosingData;
};

@interface windowDelegateClass : NSObject<NSWindowDelegate> {
	NSMapTable *windows;
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

// TODO have this called
- (void)dealloc
{
	if ([self->windows count] != 0)
		complain("attempt to destroy shared window delegate but windows are still registered to it");
	[self->windows release];
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
	mapSet(self->windows w->window, w);
	[w->window setDelegate:self];
}

- (void)unregisterWindow:(uiWindow *)w
{
	[w->window setDelegate:nil];
	[self->windows removeObjectForKey:w->window];
}

- (uiWindow *)lookupWindow:(NSWindow *)w
{
	NSValue *v;

	v = (NSValue *) [self->windows objectForKey:w];
	if (v == nil)		// just in case we're called with some OS X-provided window as the key window
		return NULL;
	return (uiWindow *) [v pointerValue];
}

@end

static windowDelegateClass *windowDelegate = nil;

static void onDestroy(uiWindow *);

uiDarwinDefineControlWithOnDestroy(
	uiWindow,							// type name
	uiWindowType,							// type function
	window,								// handle
	onDestroy(this);						// on destroy
)

static void onDestroy(uiWindow *w)
{
	NSView *childView;

	// hide the window
	[w->window orderOut:w->window];
	if (w->child != NULL) {
		childView = (NSView *) uiControlHandle(w->child);
		[childView removeFromSuperview];
		uiControlSetParent(w->child, NULL);
		uiControlDestroy(w->child);
	}
	[windowDelegate unregisterWindow:w];
}

// TODO forbid setting a parent

static void windowCommitShow(uiControl *c)
{
	uiWindow *w = (uiWindow *) c;

	[w->window makeKeyAndOrderFront:w->window];
}

static void windowCommitHide(uiControl *c)
{
	uiWindow *w = (uiWindow *) c;

	[w->window orderOut:w->window];
}

// TODO container update state

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
		[[w->window contentView] addSubview:childView];
		layoutSingleView([w->window contentView], childView, w->margined);
	}
}

int uiWindowMargined(uiWindow *w)
{
	return w->margined;
}

void uiWindowSetMargined(uiWindow *w, int margined)
{
	NSView *childView;

	w->margined = margined;
	if (w->child != NULL) {
		childView = (NSView *) uiControlHandle(w->child);
		[[w->window contentView] addSubview:childView];
		layoutSingleView([w->window contentView], childView, w->margined);
	}
}

static int defaultOnClosing(uiWindow *w, void *data)
{
	return 0;
}

uiWindow *uiNewWindow(const char *title, int width, int height, int hasMenubar)
{
	uiWindow *w;

	finalizeMenus();

	w = (uiWindow *) uiNewControl(uiWindowType());

	w->window = [[NSWindow alloc] initWithContentRect:NSMakeRect(0, 0, (CGFloat) width, (CGFloat) height)
		styleMask:(NSTitledWindowMask | NSClosableWindowMask | NSMiniaturizableWindowMask | NSResizableWindowMask)
		backing:NSBackingStoreBuffered
		defer:YES];
	[w->window setTitle:toNSString(title)];

	// explicitly release when closed
	// the only thing that closes the window is us anyway
	[w->window setReleasedWhenClosed:YES];

	if (windowDelegate == nil)
		windowDelegate = [windowDelegateClass new];
	[windowDelegate registerWindow:w];
	uiWindowOnClosing(w, defaultOnClosing, NULL);

	uiDarwinFinishNewControl(w, uiWindow);
//TODO	uiControl(w)->CommitShow = windowCommitShow;
//TODO	uiControl(w)->CommitHide = windowCommitHide;

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
