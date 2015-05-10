// 12 april 2015
#import "uipriv_darwin.h"

// TODO rewrite this whole file to take advantage of bin functions

struct tab {
	uiTab t;
	NSTabView *tabview;
	NSMutableArray *pages;
	NSMutableArray *margined;
	void (*baseEnable)(uiControl *);
	void (*baseDisable)(uiControl *);
	void (*baseSysFunc)(uiControl *, uiControlSysFuncParams *);
};

static void destroy(void *data)
{
	struct tab *t = (struct tab *) data;

	// first destroy all tab pages so we can destroy all the bins
	while ([t->tabview numberOfTabViewItems] != 0)
		[t->tabview removeTabViewItem:[t->tabview tabViewItemAtIndex:0]];
	// then destroy all the bins, destroying children in the process
	// the above loop serves the purpose of binSetParent()
	[t->pages enumerateObjectsUsingBlock:^(id obj, NSUInteger index, BOOL *stop) {
		NSValue *v = (NSValue *) obj;
		uiBin *p;

		p = (uiBin *) [v pointerValue];
		uiControlDestroy(uiControl(p));
	}];
	// and finally destroy ourselves
	[t->pages release];
	[t->margined release];
	uiFree(t);
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

static void tabEnable(uiControl *c)
{
	struct tab *t = (struct tab *) c;

	(*(t->baseEnable))(uiControl(t));
	[t->pages enumerateObjectsUsingBlock:^(id obj, NSUInteger index, BOOL *stop) {
		NSValue *v = (NSValue *) obj;
		uiBin *p;

		p = (uiBin *) [v pointerValue];
		uiControlEnable(uiControl(p));
	}];
}

static void tabDisable(uiControl *c)
{
	struct tab *t = (struct tab *) c;

	(*(t->baseDisable))(uiControl(t));
	[t->pages enumerateObjectsUsingBlock:^(id obj, NSUInteger index, BOOL *stop) {
		NSValue *v = (NSValue *) obj;
		uiBin *p;

		p = (uiBin *) [v pointerValue];
		uiControlDisable(uiControl(p));
	}];
}

static void tabSysFunc(uiControl *c, uiControlSysFuncParams *p)
{
	struct tab *t = (struct tab *) c;

	(*(t->baseSysFunc))(uiControl(t), p);
	[t->pages enumerateObjectsUsingBlock:^(id obj, NSUInteger index, BOOL *stop) {
		NSValue *v = (NSValue *) obj;
		uiBin *pp;

		pp = (uiBin *) [v pointerValue];
		uiControlSysFunc(uiControl(pp), p);
	}];
}

static void tabAppendPage(uiTab *tt, const char *name, uiControl *child)
{
	struct tab *t = (struct tab *) tt;
	uiBin *page;
	NSTabViewItem *i;

	page = newBin();
	uiBinSetMainControl(page, child);
	[t->pages addObject:[NSValue valueWithPointer:page]];
	[t->margined addObject:[NSNumber numberWithInt:0]];

	i = [[NSTabViewItem alloc] initWithIdentifier:nil];
	[i setLabel:toNSString(name)];
	[i setView:((NSView *) uiControlHandle(uiControl(page)))];
	[t->tabview addTabViewItem:i];
}

static void tabInsertPageBefore(uiTab *tt, const char *name, uintmax_t n, uiControl *child)
{
	struct tab *t = (struct tab *) tt;
	uiBin *page;
	NSTabViewItem *i;

	page = newBin();
	uiBinSetMainControl(page, child);
	[t->pages insertObject:[NSValue valueWithPointer:page] atIndex:n];
	[t->margined insertObject:[NSNumber numberWithInt:0] atIndex:n];

	i = [[NSTabViewItem alloc] initWithIdentifier:nil];
	[i setLabel:toNSString(name)];
	[i setView:((NSView *) uiControlHandle(uiControl(page)))];
	[t->tabview insertTabViewItem:i atIndex:n];
}

static void tabDeletePage(uiTab *tt, uintmax_t n)
{
	struct tab *t = (struct tab *) tt;
	NSValue *v;
	uiBin *p;
	NSTabViewItem *i;

	v = (NSValue *) [t->pages objectAtIndex:n];
	p = (uiBin *) [v pointerValue];
	[t->pages removeObjectAtIndex:n];
	[t->margined removeObjectAtIndex:n];

	// make sure the children of the tab aren't destroyed
	uiBinSetMainControl(p, NULL);

	// remove the bin from the tab view
	// this serves the purpose of uiBinRemoveOSParent()
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
	uiBin *bin;

	v = [NSNumber numberWithInt:margined];
	[t->margined replaceObjectAtIndex:n withObject:v];
	binv = (NSValue *) [t->pages objectAtIndex:n];
	bin = (uiBin *) [binv pointerValue];
	if ([v intValue])
		uiBinSetMargins(bin, tabLeftMargin, tabTopMargin, tabRightMargin, tabBottomMargin);
	else
		uiBinSetMargins(bin, 0, 0, 0, 0);
}

uiTab *uiNewTab(void)
{
	struct tab *t;

	t = uiNew(struct tab);

	uiDarwinMakeControl(uiControl(t), [NSTabView class], NO, NO, destroy, t);

	t->tabview = (NSTabView *) uiControlHandle(uiControl(t));

	// also good for NSTabView (same selector and everything)
	setStandardControlFont((NSControl *) (t->tabview));

	t->pages = [NSMutableArray new];
	t->margined = [NSMutableArray new];

	uiControl(t)->PreferredSize = tabPreferredSize;
	t->baseEnable = uiControl(t)->Enable;
	uiControl(t)->Enable = tabEnable;
	t->baseDisable = uiControl(t)->Disable;
	uiControl(t)->Disable = tabDisable;
	t->baseSysFunc = uiControl(t)->SysFunc;
	uiControl(t)->SysFunc = tabSysFunc;

	uiTab(t)->AppendPage = tabAppendPage;
	uiTab(t)->InsertPageBefore = tabInsertPageBefore;
	uiTab(t)->DeletePage = tabDeletePage;
	uiTab(t)->NumPages = tabNumPages;
	uiTab(t)->Margined = tabMargined;
	uiTab(t)->SetMargined = tabSetMargined;

	return uiTab(t);
}
