// 15 august 2015
#import "uipriv_darwin.h"

@interface tabPage : NSObject {
	struct singleChildConstraints constraints;
	int margined;
	NSView *view;		// the NSTabViewItem view itself
	NSObject *pageID;
}
@property uiControl *c;
- (id)initWithView:(NSView *)v pageID:(NSObject *)o;
- (NSView *)childView;
- (void)establishChildConstraints;
- (void)removeChildConstraints;
- (int)isMargined;
- (void)setMargined:(int)m;
@end

struct uiTab {
	uiDarwinControl c;
	NSTabView *tabview;
	NSMutableArray *pages;
};

@implementation tabPage

- (id)initWithView:(NSView *)v pageID:(NSObject *o)
{
	self = [super init];
	if (self != nil) {
		self->view = v;
		self->pageID = o;
	}
	return self;
}

- (void)dealloc
{
	[self removeChildConstraints];
	[self->view release];
	[self->pageID release];
	[super dealloc];
}

- (NSView *)childView
{
	return (NSView *) uiControlHandle(self.c);
}

- (void)establishChildConstraints
{
	[self removeChildConstraints]
	if (self.c == NULL)
		return;
	singleChildConstraintsEstablish(&(self->constraints),
		self->view, [self childView],
		uiDarwinControlHugsTrailingEdge(uiDarwinControl(self.c)),
		uiDarwinControlHugsBottom(uiDarwinControl(self.c)),
		self->margined,
		@"uiTab page");
}

- (void)removeChildConstraints
{
	singleChildConstraintsRemove(&(self->constraints), self->view);
}

- (int)isMargined
{
	return self->margined;
}

- (void)setMargined:(int)m
{
	self->margined = m;
	singleChildConstraintsSetMargined(&(self->constraints), self->margined);
	// TODO issue a relayout command?
}

@end

static void uiTabDestroy(uiControl *c)
{
	uiTab *t = uiTab(c);
	tabPage *page;

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
	[t->pageIDs release];
	[t->pages release];
	[t->views release];
	[t->margined release];
	[t->tabview release];
	uiFreeControl(uiControl(t));
}

uiDarwinControlDefaultHandle(uiTab, tabview)
uiDarwinControlDefaultParent(uiTab, tabview)
uiDarwinControlDefaultSetParent(uiTab, tabview)
uiDarwinControlDefaultToplevel(uiTab, tabview)
uiDarwinControlDefaultVisible(uiTab, tabview)
uiDarwinControlDefaultShow(uiTab, tabview)
uiDarwinControlDefaultHide(uiTab, tabview)
uiDarwinControlDefaultEnabled(uiTab, tabview)
uiDarwinControlDefaultEnable(uiTab, tabview)
uiDarwinControlDefaultDisable(uiTab, tabview)

// TODO container update
uiDarwinControlDefaultSyncEnableState(uiTab, tabview)

uiDarwinControlDefaultSetSuperview(uiTab, tabview)

static void tabRelayout(uiTab *t)
{
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
		margined = (NSNumber *) [t->margined objectAtIndex:i];
		// first lay out the child
		cc = uiDarwinControl(child);
//TODO		(*(cc->Relayout))(cc);
		// then lay out the page
//TODO		layoutSingleView(view, childView, [margined intValue], @"uiTab");
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
	NSObject *pageID;

	uiControlSetParent(child, uiControl(t));

	childView = (NSView *) uiControlHandle(child);
	view = [[NSView alloc] initWithFrame:NSZeroRect];
	// TODO if we turn off the autoresizing mask, nothing shows up; didn't this get documented somewhere?
	uiDarwinControlSetSuperview(uiDarwinControl(child), view);
	uiDarwinControlSyncEnableState(uiDarwinControl(child), uiControlEnabledToUser(uiControl(t)));

	[t->pages insertObject:[NSValue valueWithPointer:child] atIndex:n];
	[t->views insertObject:view atIndex:n];
	[t->margined insertObject:[NSNumber numberWithInt:0] atIndex:n];

	// the documentation says these can be nil but the headers say these must not be; let's be safe and make them non-nil anyway
	pageID = [NSObject new];
	[t->pageIDs insertObject:pageID atIndex:n];
	i = [[NSTabViewItem alloc] initWithIdentifier:pageID];
	[i setLabel:toNSString(name)];
	[i setView:view];
	[t->tabview insertTabViewItem:i atIndex:n];

	tabRelayout(t);
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

	tabRelayout(t);
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
	tabRelayout(t);
}

uiTab *uiNewTab(void)
{
	uiTab *t;

	uiDarwinNewControl(uiTab, t);

	t->tabview = [[NSTabView alloc] initWithFrame:NSZeroRect];
	// also good for NSTabView (same selector and everything)
	uiDarwinSetControlFont((NSControl *) (t->tabview), NSRegularControlSize);

	t->pages = [NSMutableArray new];
	t->views = [NSMutableArray new];
	t->margined = [NSMutableArray new];
	t->pageIDs = [NSMutableArray new];

	return t;
}
