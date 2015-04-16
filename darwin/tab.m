// 12 april 2015
#import "uipriv_darwin.h"

// TODO
// - verify margins against extra space around the tab
// - free child containers properly

@interface uiNSTabView : NSTabView
@property uiTab *uiT;
@end

@implementation uiNSTabView

- (void)viewDidMoveToSuperview
{
	// TODO free all tabs explicitly
	if (uiDarwinControlFreeWhenAppropriate(uiControl(self.uiT), [self superview]))
		self.uiC = NULL;
	[super viewDidMoveToSuperview];
}

@end

// the default new control implementation uses -sizeToFit, which we don't have with NSTabView
// fortunately, we do have -minimumSize
static void preferredSize(uiControl *c, uiSizing *d, intmax_t *width, intmax_t *height)
{
	uiNSTabView *tv;
	NSSize s;

	tv = (uiNSTabView *) uiControlHandle(c);
	s = [tv minimumSize];
	*width = (intmax_t) (s.width);
	*height = (intmax_t) (s.height);
}

static void tabAddPage(uiTab *t, const char *name, uiControl *child)
{
	uiNSTabView *tv;
	uiParent *content;
	NSTabViewItem *i;

	content = uiNewParent(0);
	uiParentSetChild(content, child);

	i = [[NSTabViewItem alloc] initWithIdentifier:nil];
	[i setLabel:toNSString(name)];
	[i setView:((NSView *) uiParentHandle(content))];
	tv = (uiNSTabView *) uiControlHandle(uiControl(t));
	[tv addTabViewItem:i];
}

uiTab *uiNewTab(void)
{
	uiTab *t;
	uiNSTabView *tv;

	uiDarwinNewControl(uiControl(t), [uiNSTabView class], NO, NO);
	tv = (uiNSTabView *) uiControlHandle(c);

	// also good for NSTabView (same selector and everything)
	setStandardControlFont((NSControl *) tv);

	uiControl(t)->PreferredSize = preferredSize;

	uiTab(t)->AddPage = tabAddPage;

	tv.uiT = t;

	return tv.uiT;
}
