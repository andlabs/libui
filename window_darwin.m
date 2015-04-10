// 6 april 2015
#import "uipriv_darwin.h"

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
	// TODO substitutions

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
