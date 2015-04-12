// 6 april 2015
#import "uipriv_darwin.h"

// TODO
// - free chilld containers properly

@interface uiWindowDelegate : NSObject <NSWindowDelegate>
@property (assign) NSWindow *w;
@property (assign) uiContainer *container;
@property int (*onClosing)(uiWindow *, void *);
@property void *onClosingData;
@property uiWindow *uiw;
@end

@implementation uiWindowDelegate

uiLogObjCClassAllocations

- (BOOL)windowShouldClose:(id)win
{
	// return exact constants to be safe
	if ((*(self.onClosing))(self.uiw, self.onClosingData))
		return YES;
	return NO;
}

// after this method returns we assume the window will be released (see below), so we can go too
- (void)windowWillClose:(NSNotification *)note
{
	[self.w setDelegate:nil];		// see http://stackoverflow.com/a/29523141/3408572

	// when we reach this point, we need to ensure that all the window's children are destroyed (for OS parity)
	// this may not happen under certain conditions
	// I'm not sure if calling uiQuit() in the close handler causes whatever causes our window to release its content view during deallocation to race with uiQuit()'s stopping of the run loop but that's one symptom I've noticed
	// so let's manually change the content view now
	// we'll set it to a new stock view; this should be enough to set our real container's superview to nil, triggering the destruction
	[self.w setContentView:[[NSView alloc] initWithFrame:NSZeroRect]];

	uiFree(self.uiw);
	[self release];
}

@end

struct uiWindow {
	uiWindowDelegate *d;
};

static int defaultOnClosing(uiWindow *w, void *data)
{
	return 1;
}

uiWindow *uiNewWindow(char *title, int width, int height)
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

	d.container = [[uiContainer alloc] initWithFrame:NSZeroRect];
	[d.w setContentView:d.container];

	d.onClosing = defaultOnClosing;
	[d.w setDelegate:d];

	d.uiw = uiNew(uiWindow);
	d.uiw->d = d;
	return d.uiw;
}

#define D w->d

void uiWindowDestroy(uiWindow *w)
{
	[D.w close];
}

uintptr_t uiWindowHandle(uiWindow *w)
{
	return (uintptr_t) (D.w);
}

char *uiWindowTitle(uiWindow *w)
{
	return uiDarwinNSStringToText([D.w title]);
}

void uiWindowSetTitle(uiWindow *w, const char *title)
{
	[D.w setTitle:toNSString(title)];
}

void uiWindowShow(uiWindow *w)
{
	[D.w makeKeyAndOrderFront:D.w];
}

void uiWindowHide(uiWindow *w)
{
	[D.w orderOut:D.w];
}

void uiWindowOnClosing(uiWindow *w, int (*f)(uiWindow *, void *), void *data)
{
	D.onClosing = f;
	D.onClosingData = data;
}

void uiWindowSetChild(uiWindow *w, uiControl *c)
{
	D.container.uiChild = c;
	uiControlSetParent(D.container.uiChild, (uintptr_t) (D.container));
}

int uiWindowMargined(uiWindow *w)
{
	if ([D.container uiMargined])
		return 1;
	return 0;
}

void uiWindowSetMargined(uiWindow *w, int margined)
{
	BOOL m;

	m = NO;
	if (margined)
		m = YES;
	[D.container uiSetMargined:m];
}
