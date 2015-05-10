// 28 april 2015
#import "uipriv_darwin.h"

int binHasOSParent(uiBin *b)
{
	NSView *v;

	// note that:
	// - the superview of a NSWindow content view is the window frame
	// - the superview of *the active NSTabView page* is the NSTabView itself (we don't have to worry about other pages because if there are pages, then at least one page will be active, so we will eventually get here)
	v = (NSView *) uiControlHandle(uiControl(b));
	return [v superview] != nil;
}

void binSetOSParent(uiBin *b, uintptr_t osParent)
{
	complain("TODO");
}

void binRemoveOSParent(uiBin *b)
{
	complain("TODO");
}

void binResizeRootAndUpdate(uiBin *b, intmax_t x, intmax_t y, intmax_t width, intmax_t height)
{
	// not used on OS X
}

void binTranslateMargins(uiBin *b, intmax_t *left, intmax_t *top, intmax_t *right, intmax_t *bottom, uiSizing *d)
{
	// not used on OS X
}
