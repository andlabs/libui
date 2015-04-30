// 12 april 2015
#import "uipriv_darwin.h"

struct tab {
	uiTab t;
	NSTabView *tabview;
	NSMutableArray *pages;
	NSMutableArray *margined;
};

static void destroy(void *data)
{
	struct tab *t = (struct tab *) data;

	// first destroy all tab pages so we can destroy all the bins
	while ([t->tabview numberOfTabViewItems] != 0)
		[t->tabview removeTabViewItem:[t->tabview tabViewItemAtIndex:0]];
	// then destroy all the bins, destroying children in the process
	[t->pages enumerateObjectsUsingBlock:^(id obj, NSUInteger index, BOOL *stop) {
		NSValue *v = (NSValue *) obj;
		uiContainer *p;

		p = (uiContainer *) [v pointerValue];
		uiControlDestroy(uiControl(p));
	}];
	// and finally destroy ourselves
	[t->pages release];
	[t->margined release];
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
	binSetMainControl(page, child);
	[t->pages addObject:[NSValue valueWithPointer:page]];
	[t->margined addObject:[NSNumber numberWithInt:0]];

	i = [[NSTabViewItem alloc] initWithIdentifier:nil];
	[i setLabel:toNSString(name)];
	[i setView:((NSView *) uiControlHandle(uiControl(page)))];
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
	[t->margined removeObjectAtIndex:n];

	// make sure the children of the tab aren't destroyed
	binSetMainControl(p, NULL);

	// remove the bin from the tab view
	i = [t->tabview tabViewItemAtIndex:n];
	[t->tabview removeTabViewItem:i];

	// then destroy the bin
	uiControlDestroy(uiControl(p));
}

static uintmax_t tabNumPages(uiTab *tt)
{
	struct tab *t = (struct tab *) tt;

	return [t->pages count];
}

static int tabMargined(uiTab *tt, uintmax_t n)
{
	struct tab *t = (struct tab *) tt;
	NSNumber *v;

	v = (NSNumber *) [t->margined objectAtIndex:n];
	return [v intValue];
}

// These are based on measurements from Interface Builder.
// These seem to be based on Auto Layout constants, but I don't see an API that exposes these...
#define tabLeftMargin 17
#define tabTopMargin 3
#define tabRightMargin 17
#define tabBottomMargin 17

// notes:
// top margin of a tab to its parent should be 12, not 20
// our system doesn't allow this...

static void tabSetMargined(uiTab *tt, uintmax_t n, int margined)
{
	struct tab *t = (struct tab *) tt;
	NSNumber *v;
	NSValue *binv;
	uiContainer *bin;

	v = [NSNumber numberWithInt:margined];
	[t->margined replaceObjectAtIndex:n withObject:v];
	binv = (NSValue *) [t->pages objectAtIndex:n];
	bin = (uiContainer *) [binv pointerValue];
	if ([v intValue])
		binSetMargins(bin, tabLeftMargin, tabTopMargin, tabRightMargin, tabBottomMargin);
	else
		binSetMargins(bin, 0, 0, 0, 0);
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
	t->margined = [NSMutableArray new];

	uiControl(t)->PreferredSize = preferredSize;

	uiTab(t)->AppendPage = tabAppendPage;
	uiTab(t)->DeletePage = tabDeletePage;
	uiTab(t)->NumPages = tabNumPages;
	uiTab(t)->Margined = tabMargined;
	uiTab(t)->SetMargined = tabSetMargined;

	return uiTab(t);
}
