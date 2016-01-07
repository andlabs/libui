// 15 august 2015
#import "uipriv_darwin.h"

struct uiTab {
	uiDarwinControl c;
	NSTabView *tabview;
	// TODO either rename all uses of child to page or rename this to children
	NSMutableArray *pages;			// []NSValue<uiControl *>
	// the views that contain the children's views
	// these are the views that are assigned to each NSTabViewItem
	NSMutableArray *views;			// []NSView
	NSMutableArray *margined;		// []NSNumber
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
	// first remove all tab pages so we can destroy all the children
	while ([t->tabview numberOfTabViewItems] != 0)
		[t->tabview removeTabViewItem:[t->tabview tabViewItemAtIndex:0]];
	// then destroy all the children
	[t->pages enumerateObjectsUsingBlock:^(id obj, NSUInteger index, BOOL *stop) {
		NSValue *v = (NSValue *) obj;
		uiControl *c;

		c = (uiControl *) [v pointerValue];
		uiControlSetParent(c, NULL);
		uiControlDestroy(c);
	}];
	// and finally destroy ourselves
	[t->pages release];
	[t->views release];
	[t->margined release];
}

// TODO container update

static void tabRelayout(uiDarwinControl *c)
{
	uiTab *t = uiTab(c);
	NSUInteger i;

	if ([t->pages count] == 0)
		return;
	for (i = 0; i < [t->pages count]; i++) {
		NSValue *v;
		uiControl *child;
		uiDarwinControl *cc;
		NSView *view, *childView;
		NSNumber *margined;

		v = (NSValue *) [t->pages objectAtIndex:i];
		child = (uiControl *) [v pointerValue];
		view = (NSView *) [t->views objectAtIndex:i];
		childView = (NSView *) uiControlHandle(child);
		[view removeConstraints:[view constraints]];
		margined = (NSNumber *) [t->margined objectAtIndex:i];
		// first lay out the child
		cc = uiDarwinControl(child);
		(*(cc->Relayout))(cc);
		// then lay out the page
		layoutSingleView(view, childView, [margined intValue]);
	}
}

void uiTabAppend(uiTab *t, const char *name, uiControl *child)
{
	uiTabInsertAt(t, name, [t->pages count], child);
}

void uiTabInsertAt(uiTab *t, const char *name, uintmax_t n, uiControl *child)
{
	NSView *childView;
	NSView *view;
	NSTabViewItem *i;

	uiControlSetParent(child, uiControl(t));

	childView = (NSView *) uiControlHandle(child);
	view = [[NSView alloc] initWithFrame:NSZeroRect];
	// TODO if we turn off the autoresizing mask, nothing shows up; didn't this get documented somewhere?
	[view addSubview:childView];

	[t->pages insertObject:[NSValue valueWithPointer:child] atIndex:n];
	[t->views insertObject:view atIndex:n];
	[t->margined insertObject:[NSNumber numberWithInt:0] atIndex:n];

	i = [[NSTabViewItem alloc] initWithIdentifier:nil];
	[i setLabel:toNSString(name)];
	[i setView:view];
	[t->tabview insertTabViewItem:i atIndex:n];

	uiDarwinControlTriggerRelayout(uiDarwinControl(t));
}

void uiTabDelete(uiTab *t, uintmax_t n)
{
	NSValue *v;
	uiControl *child;
	NSView *childView;
	NSTabViewItem *i;

	v = (NSValue *) [t->pages objectAtIndex:n];
	child = (uiControl *) [v pointerValue];

	[t->pages removeObjectAtIndex:n];
	[t->views removeObjectAtIndex:n];
	[t->margined removeObjectAtIndex:n];

	childView = (NSView *) uiControlHandle(child);
	[childView removeFromSuperview];
	uiControlSetParent(child, NULL);

	i = [t->tabview tabViewItemAtIndex:n];
	[t->tabview removeTabViewItem:i];
}

uintmax_t uiTabNumPages(uiTab *t)
{
	return [t->pages count];
}

int uiTabMargined(uiTab *t, uintmax_t n)
{
	NSNumber *v;

	v = (NSNumber *) [t->margined objectAtIndex:n];
	return [v intValue];
}

void uiTabSetMargined(uiTab *t, uintmax_t n, int margined)
{
	NSNumber *v;

	v = [NSNumber numberWithInt:margined];
	[t->margined replaceObjectAtIndex:n withObject:v];
	uiDarwinControlTriggerRelayout(uiDarwinControl(t));
}

uiTab *uiNewTab(void)
{
	uiTab *t;

	t = (uiTab *) uiNewControl(uiTabType());

	t->tabview = [[NSTabView alloc] initWithFrame:NSZeroRect];
	// also good for NSTabView (same selector and everything)
	uiDarwinSetControlFont((NSControl *) (t->tabview), NSRegularControlSize);

	t->pages = [NSMutableArray new];
	t->views = [NSMutableArray new];
	t->margined = [NSMutableArray new];

	uiDarwinFinishNewControl(t, uiTab);
	uiDarwinControl(t)->Relayout = tabRelayout;

	return t;
}
