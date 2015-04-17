// 12 april 2015
#import "uipriv_darwin.h"

// TODO
// - verify margins against extra space around the tab
// - free child containers properly

struct tab {
	uiTab t;
	NSTabView *tabview;
};

static void destroy(void *data)
{
	struct tab *t = (struct tab *) data;

	uiFree(t);
}

// the default new control implementation uses -sizeToFit, which we don't have with NSTabView
// fortunately, we do have -minimumSize
static void preferredSize(uiControl *c, uiSizing *d, intmax_t *width, intmax_t *height)
{
	struct tab *t = (struct tab *) c;
	NSSize s;

	s = [t->tabview minimumSize];
	*width = (intmax_t) (s.width);
	*height = (intmax_t) (s.height);
}

static void tabAddPage(uiTab *tt, const char *name, uiControl *child)
{
	struct tab *t = (struct tab *) tt;
	uiParent *content;
	NSTabViewItem *i;

	content = uiNewParent(0);
	uiParentSetMainControl(content, child);

	i = [[NSTabViewItem alloc] initWithIdentifier:nil];
	[i setLabel:toNSString(name)];
	[i setView:((NSView *) uiParentHandle(content))];
	[t->tabview addTabViewItem:i];
}

uiTab *uiNewTab(void)
{
	struct tab *t;

	t = uiNew(struct tab);

	uiDarwinNewControl(uiControl(t), [NSTabView class], NO, NO, destroy, NULL);

	t->tabview = (NSTabView *) VIEW(t);

	// also good for NSTabView (same selector and everything)
	setStandardControlFont((NSControl *) (t->tabview));

	uiControl(t)->PreferredSize = preferredSize;

	uiTab(t)->AddPage = tabAddPage;

	return uiTab(t);
}
