// 28 april 2015
#import "uipriv_darwin.h"

@interface windowDelegate : NSObject <NSWindowDelegate> {
	uiWindow *w;
	int (*onClosing)(uiWindow *, void *);
	void *onClosingData;
}
- (uiWindow *)getuiWindow;
- (void)setuiWindow:(uiWindow *)ww;
- (void)setOnClosing:(int (*)(uiWindow *, void *))f data:(void *)data;
@end

@implementation windowDelegate

- (uiWindow *)getuiWindow
{
	return self->w;
}

- (void)setuiWindow:(uiWindow *)ww
{
	self->w = ww;
}

- (void)setOnClosing:(int (*)(uiWindow *, void *))f data:(void *)data
{
	self->onClosing = f;
	self->onClosingData = data;
}

- (BOOL)windowShouldClose:(id)sender
{
	if ((*(self->onClosing))(self->w, self->onClosingData))
		uiControlDestroy(uiControl(self->w));
	return NO;
}

@end

struct window {
	uiWindow w;
	NSWindow *window;
	windowDelegate *delegate;
	uiControl *bin;
};

static int defaultOnClosing(uiWindow *w, void *data)
{
	return 0;
}

static void windowDestroy(uiControl *c)
{
	struct window *w = (struct window *) c;

	// first hide ourselves
	[w->window orderOut:w->window];
	// now destroy the child
	binSetChild(w->bin, NULL);
	uiControlDestroy(w->child);
	// now destroy the bin
	// we do this by changing the content view to a dummy view
	// the window will release its reference on the bin now, then it will release its reference on the dummy view when the window itself is finally released
	[w->window setContentView:[[NSView alloc] initWithFrame:NSZeroRect]];
	uiControlDestroy(w->bin);
	// now destroy the delegate
	[w->window setDelegate:nil];
	[w->delegate release];
	// now destroy ourselves
	// don't call the base; we use a different method
	[w->window close];			// see below about releasing when closed
}

static uintptr_t windowHandle(uiControl *c)
{
	struct window *w = (struct window *) c;

	return (uintptr_t) (w->window);
}

static void windowCommitShow(uiControl *c)
{
	struct window *w = (struct window *) c;

	[w->window makeKeyAndOrderFront:w->window];
}

static void windowCommitHide(uiControl *c)
{
	struct window *w = (struct window *) c;

	[w->window orderOut:w->window];
}

static void windowContainerUpdateState(uiControl *c)
{
	struct window *w = (struct window *) c;

	if (w->child != NULL)
		uiControlUpdateState(w->child);
}

static char *windowTitle(uiWindow *ww)
{
	struct window *w = (struct window *) ww;

	return uiDarwinNSStringToText([w->window title]);
}

static void windowSetTitle(uiWindow *ww, const char *title)
{
	struct window *w = (struct window *) ww;

	[w->window setTitle:toNSString(title)];
}

static void windowOnClosing(uiWindow *ww, int (*f)(uiWindow *, void *), void *data)
{
	struct window *w = (struct window *) ww;

	[w->delegate setOnClosing:f data:data];
}

static void windowSetChild(uiWindow *ww, uiControl *child)
{
	struct window *w = (struct window *) ww;

	binSetChild(w->bin, child);
}

static int windowMargined(uiWindow *ww)
{
	struct window *w = (struct window *) ww;

	return binMargined(w->bin);
}

static void windowSetMargined(uiWindow *ww, int margined)
{
	struct window *w = (struct window *) ww;

	binSetMargined(w->bin, margined);
	uiContainerUpdate(uiContainer(w->bin));
}

static void windowResizeChild(uiWindow *ww)
{
	complain("uiWindowResizeChild() meaningless on OS X");
}

uiWindow *uiNewWindow(const char *title, int width, int height, int hasMenubar)
{
	struct window *w;
	NSView *binView;

	finalizeMenus();

	w = (struct window *) uiNewControl(uiTypeWindow());

	w->window = [[NSWindow alloc] initWithContentRect:NSMakeRect(0, 0, (CGFloat) width, (CGFloat) height)
		styleMask:(NSTitledWindowMask | NSClosableWindowMask | NSMiniaturizableWindowMask | NSResizableWindowMask)
		backing:NSBackingStoreBuffered
		defer:YES];
	[w->window setTitle:toNSString(title)];

	// a NSWindow is not a NSView, but nothing we're doing in this function is view-specific
	uiDarwinMakeSingleViewControl(uiControl(w), (NSView *) (w->window));

	// explicitly release when closed
	// the only thing that closes the window is us anyway
	[w->window setReleasedWhenClosed:YES];

	w->delegate = [windowDelegate new];
	[w->delegate setuiWindow:uiWindow(w)];
	[w->window setDelegate:w->delegate];

	w->bin = newBin();
	binView = (NSView *) uiControlHandle(uiControl(w->bin));
	[w->window setContentView:binView];

	[w->delegate setOnClosing:defaultOnClosing data:NULL];

	uiControl(w)->Handle = windowHandle;
	uiControl(w)->CommitDestroy = windowCommitDestroy;
	uiControl(w)->CommitShow = windowCommitShow;
	uiControl(w)->CommitHide = windowCommitHide;
	uiControl(w)->ContainerUpdateState = windowContainerUpdateState;

	uiWindow(w)->Title = windowTitle;
	uiWindow(w)->SetTitle = windowSetTitle;
	uiWindow(w)->OnClosing = windowOnClosing;
	uiWindow(w)->SetChild = windowSetChild;
	uiWindow(w)->Margined = windowMargined;
	uiWindow(w)->SetMargined = windowSetMargined;
	uiWindow(w)->ResizeChild = windowResizeChild;

	return uiWindow(w);
}

uiWindow *windowFromNSWindow(NSWindow *w)
{
	windowDelegate *d;

	if (w == nil)
		return NULL;
	d = (windowDelegate *) [w delegate];
	return [d getuiWindow];
}
