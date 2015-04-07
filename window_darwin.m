// 6 april 2015
#include "ui_darwin.h"

@interface uiWindowDelegate : NSObject <NSWindowDelegate>
@property uiWindow *w;
@property int (*onClosing)(uiWindow *, void *);
@property void *onClosingData;
@end

@implementation uiWindowDelegate

// TODO will this *destroy* the window?
- (BOOL)windowShouldClose:(id)win
{
	// return exact constants to be safe
	if ((*(self.onClosing))(self.w, self.onClosingData))
		return YES;
	return NO;
}

@end

struct uiWindow {
	NSWindow *w;
	uiWindowDelegate *d;
};

static int defaultOnClosing(uiWindow *w, void *data)
{
	return 1;
}

uiWindow *uiNewWindow(char *title, int width, int height)
{
	uiWindow *w;

	w = (uiWindow *) uiAlloc(sizeof (uiWindow));

	w->w = [[NSWindow alloc] initWithContentRect:NSMakeRect(0, 0, (CGFloat) width, (CGFloat) height)
		styleMask:(NSTitledWindowMask | NSClosableWindowMask | NSMiniaturizableWindowMask | NSResizableWindowMask)
		backing:NSBackingStoreBuffered
		defer:YES];
	[w->w setTitle:toNSString(title)];
	// TODO substitutions

	w->d = [uiWindowDelegate new];
	w->d.w = w;
	w->d.onClosing = defaultOnClosing;
	[w->w setDelegate:w->d];

	return w;
}

void uiWindowDestroy(uiWindow *w)
{
	// TODO
	// TODO will w->d be destroyed?
}

uintptr_t uiWindowHandle(uiWindow *w)
{
	return (uintptr_t) (w->w);
}

// TODO titles

void uiWindowShow(uiWindow *w)
{
	[w->w makeKeyAndOrderFront:w->w];
}

void uiWindowHide(uiWindow *w)
{
	[w->w orderOut:w->w];
}

void uiWindowOnClosing(uiWindow *w, int (*f)(uiWindow *, void *), void *data)
{
	w->d.onClosing = f;
	w->d.onClosingData = data;
}
