// 11 june 2015
#include "uipriv_darwin.h"

// TODO rewrite this whole file to take advantage of what we used to call bin functions

struct tab {
	uiTab t;
	NSTabView *tabview;
	NSMutableArray *pages;
	NSMutableArray *margined;
	void (*baseCommitDestroy)(uiControl *);
};

uiDefineControlType(uiTab, uiTypeTab, struct tab)

static void tabCommitDestroy(uiControl *c)
{
	struct tab *t = (struct tab *) c;

	// first destroy all tab pages so we can destroy all the bins
	while ([t->tabview numberOfTabViewItems] != 0)
		[t->tabview removeTabViewItem:[t->tabview tabViewItemAtIndex:0]];
	// then destroy all the bins, destroying children in the process
	// the above loop serves the purpose of binSetParent()
	[t->pages enumerateObjectsUsingBlock:^(id obj, NSUInteger index, BOOL *stop) {
		NSValue *v = (NSValue *) obj;
		uiControl *bin;

		bin = (uiControl *) [v pointerValue];
		binSetChild(bin, NULL);
		// TODO destroy the child
		uiControlDestroy(uiControl(bin));
	}];
	// and finally destroy ourselves
	[t->pages release];
	[t->margined release];
	(*(t->baseCommitDestroy))(uiControl(t));
}

static uintptr_t tabHandle(uiControl *c)
{
	struct tab *t = (struct tab *) c;

	return (uintptr_t) (t->tabview);
}

// the default new control implementation uses -sizeToFit, which we don't have with NSTabView
// fortunately, we do have -minimumSize
static void tabPreferredSize(uiControl *c, uiSizing *d, intmax_t *width, intmax_t *height)
{
	struct tab *t = (struct tab *) c;
	NSSize s;

	s = [t->tabview minimumSize];
	*width = (intmax_t) (s.width);
	*height = (intmax_t) (s.height);
}

static void tabContainerUpdate(uiControl *c)
{
	struct tab *t = (struct tab *) c;

	(*(t->baseEnable))(uiControl(t));
	// TODO enumerate over page CONTROLS instead
	[t->pages enumerateObjectsUsingBlock:^(id obj, NSUInteger index, BOOL *stop) {
		NSValue *v = (NSValue *) obj;
		uiControl *bin;

		bin = (uiControl *) [v pointerValue];
		// TODO get the right function
		uiContainerUpdate(uiControl(bin));
	}];
}

// TODO merge with InsertAt
static void tabAppend(uiTab *tt, const char *name, uiControl *child)
{
	struct tab *t = (struct tab *) tt;
	uiControl *page;
	NSTabViewItem *i;

	page = newBin();
	binSetChild(page, child);
	[t->pages addObject:[NSValue valueWithPointer:page]];
	[t->margined addObject:[NSNumber numberWithInt:0]];

	i = [[NSTabViewItem alloc] initWithIdentifier:nil];
	[i setLabel:toNSString(name)];
	[i setView:((NSView *) uiControlHandle(uiControl(page)))];
	[t->tabview addTabViewItem:i];
}

static void tabInsertAt(uiTab *tt, const char *name, uintmax_t n, uiControl *child)
{
	struct tab *t = (struct tab *) tt;
	uiControl *page;
	NSTabViewItem *i;

	page = newBin();
	binSetChild(page, child);
	[t->pages insertObject:[NSValue valueWithPointer:page] atIndex:n];
	[t->margined insertObject:[NSNumber numberWithInt:0] atIndex:n];

	i = [[NSTabViewItem alloc] initWithIdentifier:nil];
	[i setLabel:toNSString(name)];
	[i setView:((NSView *) uiControlHandle(uiControl(page)))];
	[t->tabview insertTabViewItem:i atIndex:n];
}

static void tabDelete(uiTab *tt, uintmax_t n)
{
	struct tab *t = (struct tab *) tt;
	NSValue *v;
	uiControl *page;
	NSTabViewItem *i;

	v = (NSValue *) [t->pages objectAtIndex:n];
	page = (uiBin *) [v pointerValue];
	[t->pages removeObjectAtIndex:n];
	[t->margined removeObjectAtIndex:n];

	// make sure the children of the tab aren't destroyed
	binSetChild(page, NULL);

	// remove the bin from the tab view
	// this serves the purpose of uiBinRemoveOSParent()
	i = [t->tabview tabViewItemAtIndex:n];
	[t->tabview removeTabViewItem:i];

	// then destroy the bin
	uiControlDestroy(uiControl(page));
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
	NSValue *pagev;
	uiBin *page;

	v = [NSNumber numberWithInt:margined];
	[t->margined replaceObjectAtIndex:n withObject:v];
	pagev = (NSValue *) [t->pages objectAtIndex:n];
	page = (uiBin *) [pagev pointerValue];
/* TODO
	if ([v intValue])
		uiBinSetMargins(page, tabLeftMargin, tabTopMargin, tabRightMargin, tabBottomMargin);
	else
		uiBinSetMargins(page, 0, 0, 0, 0);
*/
}

uiTab *uiNewTab(void)
{
	struct tab *t;

	t = (struct tab *) MAKE_CONTROL_INSTANCE(uiTypeTab());

	t->tabview = [[NSTabView alloc] initWithFrame:NSZeroRect];
	// also good for NSTabView (same selector and everything)
	uiDarwinMakeSingleViewControl(uiControl(t), t->tabview, YES);

	t->pages = [NSMutableArray new];
	t->margined = [NSMutableArray new];

	uiControl(t)->Handle = tabHandle;
	uiControl(t)->PreferredSize = tabPreferredSize;
	t->baseCommitDestroy = uiControl(t)->CommitDestroy;
	uiControl(t)->CommitDestroy = tabCommitDestroy;
	uiControl(t)->ContainerUpdate = tabContainerUpdate;

	uiTab(t)->Append = tabAppend;
	uiTab(t)->InsertAt = tabInsertAt;
	uiTab(t)->Delete = tabDelete;
	uiTab(t)->NumPages = tabNumPages;
	uiTab(t)->Margined = tabMargined;
	uiTab(t)->SetMargined = tabSetMargined;

	return uiTab(t);
}
