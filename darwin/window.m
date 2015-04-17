// 6 april 2015
#import "uipriv_darwin.h"

// TODO
// - free chilld containers properly

@interface uiWindowDelegate : NSObject <NSWindowDelegate>
@property (assign) NSWindow *w;
@property uiParent *content;
@property int (*onClosing)(uiWindow *, void *);
@property void *onClosingData;
@property struct window *uiw;
@end

@implementation uiWindowDelegate

uiLogObjCClassAllocations

- (BOOL)windowShouldClose:(id)win
{
	// return exact constants to be safe
	if ((*(self.onClosing))(uiWindow(self.uiw), self.onClosingData))
		return YES;
	return NO;
}

// after this method returns we assume the window will be released (see below), so we can go too
- (void)windowWillClose:(NSNotification *)note
{
	[self.w setDelegate:nil];		// see http://stackoverflow.com/a/29523141/3408572

	// when we reach this point, we need to ensure that all the window's children are destroyed (for OS parity)
	// because we need to set the content view's superview to the destroyed controls view to trigger deletion, we need to do this manually
	// first, replace the current content view...
	[self.w setContentView:[[NSView alloc] initWithFrame:NSZeroRect]];
	// ...then, trigger the deletion
	[destroyedControlsView addSubview:((NSView *) uiParentHandle(self.content))];

	uiFree(self.uiw);
	[self release];
}

@end

struct window {
	uiWindow w;
	uiWindowDelegate *d;
	int margined;
};

static int defaultOnClosing(uiWindow *w, void *data)
{
	return 1;
}

#define D (((struct window *) w)->d)

static void windowDestroy(uiWindow *w)
{
	[D.w close];
}

static uintptr_t windowHandle(uiWindow *w)
{
	return (uintptr_t) (D.w);
}

static char *windowTitle(uiWindow *w)
{
	return uiDarwinNSStringToText([D.w title]);
}

static void windowSetTitle(uiWindow *w, const char *title)
{
	[D.w setTitle:toNSString(title)];
}

static void windowShow(uiWindow *w)
{
	[D.w makeKeyAndOrderFront:D.w];
}

static void windowHide(uiWindow *w)
{
	[D.w orderOut:D.w];
}

static void windowOnClosing(uiWindow *w, int (*f)(uiWindow *, void *), void *data)
{
	D.onClosing = f;
	D.onClosingData = data;
}

static void windowSetChild(uiWindow *w, uiControl *c)
{
	uiParentSetMainControl(D.content, c);
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
		uiParentSetMargins(D.content, macXMargin, macYMargin, macXMargin, macYMargin);
	else
		uiParentSetMargins(D.content, 0, 0, 0, 0);
	uiParentUpdate(D.content);
}

uiWindow *uiNewWindow(const char *title, int width, int height)
{
	uiWindowDelegate *d;

	d = [uiWindowDelegate new];

	d.w = [[NSWindow alloc] initWithContentRect:NSMakeRect(0, 0, (CGFloat) width, (CGFloat) height)
		styleMask:(NSTitledWindowMask | NSClosableWindowMask | NSMiniaturizableWindowMask | NSResizableWindowMask)
		backing:NSBackingStoreBuffered
		defer:YES];
	[d.w setTitle:toNSString(title)];

	// we do not want substitutions
	// text fields, labels, etc. take their smart quotes and other autocorrect settings from their parent window, which provides a shared "field editor"
	// so we have to turn them off here
	// thanks akempgen in irc.freenode.net/#macdev
	// for some reason, this selector returns NSText but is documented to return NSTextView...
	// NOTE: if you disagree with me about disabling substitutions, start a github issue with why and I'll be happy to consider it
	disableAutocorrect((NSTextView *) [d.w fieldEditor:YES forObject:nil]);

	// this is what will destroy the window on close
	[d.w setReleasedWhenClosed:YES];

	d.content = uiNewParent(0);
	[d.w setContentView:((NSView *) uiParentHandle(d.content))];

	d.onClosing = defaultOnClosing;
	[d.w setDelegate:d];

	d.uiw = uiNew(struct window);
	d.uiw->d = d;

	uiWindow(d.uiw)->Destroy = windowDestroy;
	uiWindow(d.uiw)->Handle = windowHandle;
	uiWindow(d.uiw)->Title = windowTitle;
	uiWindow(d.uiw)->SetTitle = windowSetTitle;
	uiWindow(d.uiw)->Show = windowShow;
	uiWindow(d.uiw)->Hide = windowHide;
	uiWindow(d.uiw)->OnClosing = windowOnClosing;
	uiWindow(d.uiw)->SetChild = windowSetChild;
	uiWindow(d.uiw)->Margined = windowMargined;
	uiWindow(d.uiw)->SetMargined = windowSetMargined;

	return uiWindow(d.uiw);
}
