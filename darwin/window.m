// 28 april 2015
#import "uipriv_darwin.h"

@interface windowDelegate : NSObject <NSWindowDelegate> {
	uiWindow *w;
	int (*onClosing)(uiWindow *, void *);
	void *onClosingData;
}
- (void)setuiWindow:(uiWindow *)ww;
- (void)setOnClosing:(int (*)(uiWindow *, void *))f data:(void *)data;
@end

@implementation windowDelegate

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
	uiContainer *bin;
	int hidden;
	int margined;
};

static int defaultOnClosing(uiWindow *w, void *data)
{
	return 1;
}

static void windowDestroy(uiControl *c)
{
	struct window *w = (struct window *) c;

	// first hide ourselves
	[w->window orderOut:w->window];
	// now destroy the bin
	// the bin has no parent, so we can just call uiControlDestroy()
	// we also don't have to worry about the NSWindow's reference to the content view; that'll be released too
	uiControlDestroy(uiControl(w->bin));
	// now destroy the delegate
	[w->window setDelegate:nil];
	[w->delegate release];
	// now destroy ourselves
	[w->window close];			// see below about releasing when closed
	uiFree(w);
}

static uintptr_t windowHandle(uiControl *c)
{
	struct window *w = (struct window *) c;

	return (uintptr_t) (w->window);
}

static void windowSetParent(uiControl *c, uiContainer *parent)
{
	complain("attempt to give the uiWindow at %p a parent", c);
}

static void windowPreferredSize(uiControl *c, uiSizing *d, intmax_t *width, intmax_t *height)
{
	complain("attempt to get the preferred size of the uiWindow at %p", c);
}

static void windowResize(uiControl *c, intmax_t x, intmax_t y, intmax_t width, intmax_t height, uiSizing *d)
{
	complain("attempt to resize the uiWindow at %p", c);
}

static int windowVisible(uiControl *c)
{
	struct window *w = (struct window *) c;

	return !w->hidden;
}

static void windowShow(uiControl *c)
{
	struct window *w = (struct window *) c;

	// don't make it key (TODO really? first time only?)
	// unfortunately there's no orderIn: or something that says "make the window visible but preserve the z-order" so
	[w->window orderFront:w->window];
	w->hidden = 0;
}

static void windowHide(uiControl *c)
{
	struct window *w = (struct window *) c;

	[w->window orderOut:w->window];
	w->hidden = 1;
}

static void windowEnable(uiControl *c)
{
	struct window *w = (struct window *) c;

	// TODO
}

static void windowDisable(uiControl *c)
{
	struct window *w = (struct window *) c;

	// TODO
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

	binSetMainControl(w->bin, child);
}

static int windowMargined(uiWindow *ww)
{
	struct window *w = (struct window *) ww;

	return w->margined;
}

static void windowSetMargined(uiWindow *ww, int margined)
{
	struct window *w = (struct window *) ww;

	w->margined = margined;
	if (w->margined)
		binSetMargins(w->bin, macXMargin, macYMargin, macXMargin, macYMargin);
	else
		binSetMargins(w->bin, 0, 0, 0, 0);
}

uiWindow *uiNewWindow(const char *title, int width, int height, int hasMenubar)
{
	struct window *w;
	NSView *binView;

	w = uiNew(struct window);

	w->window = [[NSWindow alloc] initWithContentRect:NSMakeRect(0, 0, (CGFloat) width, (CGFloat) height)
		styleMask:(NSTitledWindowMask | NSClosableWindowMask | NSMiniaturizableWindowMask | NSResizableWindowMask)
		backing:NSBackingStoreBuffered
		defer:YES];
	[w->window setTitle:toNSString(title)];

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

	uiControl(w)->Destroy = windowDestroy;
	uiControl(w)->Handle = windowHandle;
	uiControl(w)->SetParent = windowSetParent;
	uiControl(w)->PreferredSize = windowPreferredSize;
	uiControl(w)->Resize = windowResize;
	uiControl(w)->Visible = windowVisible;
	uiControl(w)->Show = windowShow;
	uiControl(w)->Hide = windowHide;
	uiControl(w)->Enable = windowEnable;
	uiControl(w)->Disable = windowDisable;

	uiWindow(w)->Title = windowTitle;
	uiWindow(w)->SetTitle = windowSetTitle;
	uiWindow(w)->OnClosing = windowOnClosing;
	uiWindow(w)->SetChild = windowSetChild;
	uiWindow(w)->Margined = windowMargined;
	uiWindow(w)->SetMargined = windowSetMargined;

	return uiWindow(w);
}
