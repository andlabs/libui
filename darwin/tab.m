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
		uiParent *p;

		p = (uiParent *) [v pointerValue];
		uiParentDestroy(p);
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

static void tabAddPage(uiTab *tt, const char *name, uiControl *child)
{
	struct tab *t = (struct tab *) tt;
	uiParent *content;
	NSTabViewItem *i;

	content = uiNewParent(0);
	uiParentSetMainControl(content, child);
	[t->pages addObject:[NSValue valueWithPointer:content]];

	i = [[NSTabViewItem alloc] initWithIdentifier:nil];
	[i setLabel:toNSString(name)];
	[i setView:((NSView *) uiParentHandle(content))];
	[t->tabview addTabViewItem:i];
}

static void tabDeletePage(uiTab *tt, uintmax_t n)
{
	struct tab *t = (struct tab *) tt;
	NSValue *v;
	uiParent *p;
	NSTabViewItem *i;

	v = (NSValue *) [t->pages objectAtIndex:n];
	p = (uiParent *) [v pointerValue];
	[t->pages removeObjectAtIndex:n];
	// make sure the children of the tab aren't destroyed
	uiParentSetMainControl(p, NULL);

	// TODO negotiate lifetimes better
	i = [t->tabview tabViewItemAtIndex:n];
	[t->tabview removeTabViewItem:i];
}

uiTab *uiNewTab(void)
{
	struct tab *t;

	t = uiNew(struct tab);

	uiDarwinNewControl(uiControl(t), [NSTabView class], NO, NO, destroy, t);

	t->tabview = (NSTabView *) VIEW(t);

	// also good for NSTabView (same selector and everything)
	setStandardControlFont((NSControl *) (t->tabview));

	t->pages = [NSMutableArray new];

	uiControl(t)->PreferredSize = preferredSize;

	uiTab(t)->AddPage = tabAddPage;
	uiTab(t)->DeletePage = tabDeletePage;

	return uiTab(t);
}
