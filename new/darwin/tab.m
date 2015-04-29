// 12 april 2015
#import "uipriv_darwin.h"

// TODO
// - verify margins against extra space around the tab

struct tab {
	uiTab t;
	NSTabView *tabview;
	NSMutableArray *pages;
};

static void destroy(void *data)
{
	struct tab *t = (struct tab *) data;

	[t->pages enumerateObjectsUsingBlock:^(id obj, NSUInteger index, BOOL *stop) {
		NSValue *v = (NSValue *) obj;
		uiContainer *p;

		// TODO this is definitely wrong but
		p = (uiContainer *) [v pointerValue];
		uiControlDestroy(uiControl(p));
	}];
	[t->pages release];
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

static void tabAppendPage(uiTab *tt, const char *name, uiControl *child)
{
	struct tab *t = (struct tab *) tt;
	uiContainer *page;
	NSTabViewItem *i;

	page = newBin();
	binSetMainConotrol(page, child);
	[t->pages addObject:[NSValue valueWithPointer:page]];

	i = [[NSTabViewItem alloc] initWithIdentifier:nil];
	[i setLabel:toNSString(name)];
	[i setView:((NSView *) uiContainerHandle(content))];
	[t->tabview addTabViewItem:i];
}

static void tabDeletePage(uiTab *tt, uintmax_t n)
{
	struct tab *t = (struct tab *) tt;
	NSValue *v;
	uiContainer *p;
	NSTabViewItem *i;

	v = (NSValue *) [t->pages objectAtIndex:n];
	p = (uiContainer *) [v pointerValue];
	[t->pages removeObjectAtIndex:n];

	// make sure the children of the tab aren't destroyed
	binSetMainControl(p, NULL);

	// TODO negotiate lifetimes better
	i = [t->tabview tabViewItemAtIndex:n];
	[t->tabview removeTabViewItem:i];
}

uiTab *uiNewTab(void)
{
	struct tab *t;

	t = uiNew(struct tab);

	uiDarwinNewControl(uiControl(t), [NSTabView class], NO, NO, destroy, t);

	t->tabview = (NSTabView *) uiControlHandle(uiControl(t));

	// also good for NSTabView (same selector and everything)
	setStandardControlFont((NSControl *) (t->tabview));

	t->pages = [NSMutableArray new];

	uiControl(t)->PreferredSize = preferredSize;

	uiTab(t)->AppendPage = tabAppendPage;
	uiTab(t)->DeletePage = tabDeletePage;

	return uiTab(t);
}
