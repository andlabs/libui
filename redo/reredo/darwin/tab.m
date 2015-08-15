// 15 august 2015
#import "uipriv_darwin.h"

struct uiTab {
	uiDarwinControl c;
	NSTabView *tabview;
	NSMutableArray *pages;
	NSMutableArray *margined;
};

static void onDestroy(uiTab *);

uiDarwinDefineControlWithOnDestroy(
	uiTab,								// type name
	uiTabType,							// type function
	tabview,								// handle
	onDestroy(this);						// on destroy
)

static void onDestroy(uiTab *t)
{
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
}

// TODO container update

// TODO merge with InsertAt
void uiTabAppend(uiTab *t, const char *name, uiControl *child)
{
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

void uiTabInsertAt(uiTab *t, const char *name, uintmax_t n, uiControl *child)
{
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

void uiTabDelete(uiTab *t, uintmax_t n)
{
	NSValue *v;
	uiControl *page;
	NSTabViewItem *i;

	v = (NSValue *) [t->pages objectAtIndex:n];
	page = (uiControl *) [v pointerValue];
	[t->pages removeObjectAtIndex:n];
	[t->margined removeObjectAtIndex:n];

	// make sure the children of the tab aren't destroyed
	binSetChild(page, NULL);

	// remove the bin from the tab view
	// this serves the purpose of uiControlSetOSParent(bin, NULL)
	i = [t->tabview tabViewItemAtIndex:n];
	[t->tabview removeTabViewItem:i];

	// then destroy the bin
	uiControlDestroy(uiControl(page));
}

uintmax_t uITabNumPages(uiTab *t)
{
	return [t->pages count];
}

int uiTabMargined(uiTab *t, uintmax_t n)
{
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

void uiTabSetMargined(uiTab *t, uintmax_t n, int margined)
{
	NSNumber *v;
	NSValue *pagev;
	uiControl *page;

	v = [NSNumber numberWithInt:margined];
	[t->margined replaceObjectAtIndex:n withObject:v];
	pagev = (NSValue *) [t->pages objectAtIndex:n];
	page = (uiControl *) [pagev pointerValue];
/* TODO
	if ([v intValue])
		uiBinSetMargins(page, tabLeftMargin, tabTopMargin, tabRightMargin, tabBottomMargin);
	else
		uiBinSetMargins(page, 0, 0, 0, 0);
*/
}

uiTab *uiNewTab(void)
{
	uiTab *t;

	t = (uiTab *) uiNewControl(uiTabType());

	t->tabview = [[NSTabView alloc] initWithFrame:NSZeroRect];
	// also good for NSTabView (same selector and everything)
	uiDarwinSetControlFont((NSControl *) (t->tabview), NSRegularControlSize);

	t->pages = [NSMutableArray new];
	t->margined = [NSMutableArray new];

	uiDarwinFinishNewControl(t, uiTab);

	return t;
}
