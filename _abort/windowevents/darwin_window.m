struct uiWindow {
	// constraints
	void (*onPositionChanged)(uiWindow *, void *);
	void *onPositionChangedData;
	BOOL suppressPositionChanged;
	// onContentSizeChanged
};

@interface windowDelegateClass : NSObject<NSWindowDelegate> {
// windowShouldClose:
- (void)windowDidMove:(NSNotification *)note;
// windowDidResize:
@end

@implementation windowDelegateClass

// - (BOOL)windowShouldClose:(id)sender

// TODO doesn't happen live
- (void)windowDidMove:(NSNotification *)note
{
	uiWindow *w;

	w = [self lookupWindow:((NSWindow *) [note object])];
	if (!w->suppressPositionChanged)
		(*(w->onPositionChanged))(w, w->onPositionChangedData);
}

// - (void)windowDidResize:(NSNotification *)note

// void uiWindowSetTitle(uiWindow *w, const char *title)

void uiWindowPosition(uiWindow *w, int *x, int *y)
{
	NSScreen *screen;
	NSRect r;

	r = [w->window frame];
	*x = r.origin.x;
	// this is the right screen to use; thanks mikeash in irc.freenode.net/#macdev
	// -mainScreen is useless for positioning (it's just the key window's screen)
	// and we use -frame, not -visibleFrame, for dealing with absolute positions
	screen = (NSScreen *) [[NSScreen screens] objectAtIndex:0];
	*y = ([screen frame].size.height - r.origin.y) - r.size.height;
}

void uiWindowSetPosition(uiWindow *w, int x, int y)
{
	// -[NSWindow setFrameTopLeftPoint:] is acting weird so...
	NSRect r;
	NSScreen *screen;

	// this fires windowDidMove:
	w->suppressPositionChanged = YES;
	r = [w->window frame];
	r.origin.x = x;
	screen = (NSScreen *) [[NSScreen screens] objectAtIndex:0];
	r.origin.y = [screen frame].size.height - (y + r.size.height);
	[w->window setFrameOrigin:r.origin];
	w->suppressPositionChanged = NO;
}

void uiWindowCenter(uiWindow *w)
{
	w->suppressPositionChanged = YES;
	[w->window center];
	w->suppressPositionChanged = NO;
}

void uiWindowOnPositionChanged(uiWindow *w, void (*f)(uiWindow *, void *), void *data)
{
	w->onPositionChanged = f;
	w->onPositionChangedData = data;
}

// void uiWindowContentSize(uiWindow *w, int *width, int *height)

// static int defaultOnClosing(uiWindow *w, void *data)

static void defaultOnPositionContentSizeChanged(uiWindow *w, void *data)
{
	// do nothing
}

uiWindow *uiNewWindow(const char *title, int width, int height, int hasMenubar)
{
//	uiWindowOnClosing(w, defaultOnClosing, NULL);
	uiWindowOnPositionChanged(w, defaultOnPositionContentSizeChanged, NULL);
//	uiWindowOnContentSizeChanged(w, defaultOnPositionContentSizeChanged, NULL);
}
