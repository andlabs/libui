// 21 june 2016
#import "uipriv_darwin.h"

// TODOs
// - can't seem to grow the table view vertically beyond a certain height
// - header cell seems off

@interface tableModel : NSObject<NSTableViewDataSource, NSTableViewDelegate> {
	uiTableModel *libui_m;
}
- (id)initWithModel:(uiTableModel *)m;
@end

enum {
	partText,
};

@interface tablePart : NSObject
@property int type;
@property int mainColumn;
@property int expand;
- (NSView *)mkView:(uiTableModel *)m row:(int)row;
@end

@interface tableColumn : NSTableColumn
@property uiTableColumn *libui_col;
@end

struct uiTableModel {
	uiTableModelHandler *mh;
	tableModel *m;
	NSMutableArray *tables;
};

// TODO better memory management for this
// note how expand is part of this
struct uiTableCellPart {
	tablePart *part;
};

struct uiTableColumn {
	tableColumn *c;
	NSMutableArray *parts;
};

struct uiTable {
	uiDarwinControl c;
	NSScrollView *sv;
	NSTableView *tv;
	struct scrollViewData *d;
};

@implementation tableModel

- (id)initWithModel:(uiTableModel *)m
{
	self = [super init];
	if (self)
		self->libui_m = m;
	return self;
}

- (NSInteger)numberOfRowsInTableView:(NSTableView *)tv
{
	uiTableModelHandler *mh = self->libui_m->mh;

	return (*(mh->NumRows))(mh, self->libui_m);
}

// these are according to Interface Builder
#define xleft 2
#define xmiddle 7		/* between images and text, anyway; let's just use it for everything to be simpler */
#define xright 3

 - (NSView *)tableView:(NSTableView *)tv viewForTableColumn:(NSTableColumn *)cc row:(NSInteger)row
{
	NSTableCellView *v;
	tableColumn *c = (tableColumn *) cc;
	tablePart *part;
	NSMutableArray *views;
	NSView *view, *prev;

	v = [[NSTableCellView alloc] initWithFrame:NSZeroRect];

	views = [NSMutableArray new];
	for (part in c.libui_col->parts)
		[views addObject:[part mkView:self->libui_m row:row]];
	if ([views count] == 0)		// empty (TODO allow?)
		goto done;

	// add to v and arrange horizontally
	prev = nil;
	for (view in views) {
		[v addSubview:view];
		// TODO set [v imageView] and [v textField] as appropriate?
		if (prev == nil) {			// first view
			[v addConstraint:mkConstraint(v, NSLayoutAttributeLeading,
				NSLayoutRelationEqual,
				view, NSLayoutAttributeLeading,
				1, -xleft,
				@"uiTableColumn first part horizontal constraint")];
			prev = view;
			continue;
		}
		[v addConstraint:mkConstraint(prev, NSLayoutAttributeTrailing,
			NSLayoutRelationEqual,
			view, NSLayoutAttributeLeading,
			1, -xmiddle,
			@"uiTableColumn middle horizontal constraint")];
		prev = view;
	}
	[v addConstraint:mkConstraint(prev, NSLayoutAttributeTrailing,
		NSLayoutRelationEqual,
		v, NSLayoutAttributeTrailing,
		1, -xright,
		@"uiTableColumn last part horizontal constraint")];

	// and vertically
	for (view in views)
		[v addConstraint:mkConstraint(view, NSLayoutAttributeCenterY,
			NSLayoutRelationEqual,
			v, NSLayoutAttributeCenterY,
			1, 0,
			@"uiTableColumn part vertical constraint")];

done:
	[views release];
	[v setTranslatesAutoresizingMaskIntoConstraints:NO];
	// TODO autorelease?
	return v;
}

@end

@implementation tablePart

- (NSView *)mkView:(uiTableModel *)m row:(int)row
{
	void *data;
	NSString *str;
	NSView *view;
	NSTextField *tf;

	data = (*(m->mh->CellValue))(m->mh, m, row, self.mainColumn);
	switch (self.type) {
	case partText:
		str = toNSString((char *) data);
		uiFree(data);
		tf = newLabel(str);
		// TODO set wrap and ellipsize modes
		view = tf;
		break;
	}

	// if stretchy, don't hug, otherwise hug forcibly
	if (self.expand)
		[view setContentHuggingPriority:NSLayoutPriorityDefaultLow forOrientation:NSLayoutConstraintOrientationHorizontal];
	else
		[view setContentHuggingPriority:NSLayoutPriorityRequired forOrientation:NSLayoutConstraintOrientationHorizontal];
	[view setTranslatesAutoresizingMaskIntoConstraints:NO];
	// TODO autorelease?
	return view;
}

@end

@implementation tableColumn
@end

void *uiTableModelStrdup(const char *str)
{
	// TODO don't we have this already?
	char *dup;

	dup = (char *) uiAlloc((strlen(str) + 1) * sizeof (char), "char[]");
	strcpy(dup, str);
	return dup;
}

uiTableModel *uiNewTableModel(uiTableModelHandler *mh)
{
	uiTableModel *m;

	m = uiNew(uiTableModel);
	m->mh = mh;
	m->m = [[tableModel alloc] initWithModel:m];
	m->tables = [NSMutableArray new];
	return m;
}

void uiFreeTableModel(uiTableModel *m)
{
	if ([m->tables count] != 0)
		userbug("You cannot free a uiTableModel while uiTables are using it.");
	[m->tables release];
	[m->m release];
	uiFree(m);
}

void uiTableModelRowInserted(uiTableModel *m, int newIndex)
{
	NSTableView *tv;
	NSIndexSet *set;

	set = [NSIndexSet indexSetWithIndex:newIndex];
	for (tv in m->tables)
		[tv insertRowsAtIndexes:set withAnimation:NSTableViewAnimationEffectNone];
	// set is autoreleased
}

void uiTableModelRowChanged(uiTableModel *m, int index)
{
	NSTableView *tv;
	NSIndexSet *set, *cols;

	set = [NSIndexSet indexSetWithIndex:index];
	for (tv in m->tables) {
		cols = [[NSIndexSet alloc] initWithIndexesInRange:NSMakeRange(0, [[tv tableColumns] count])];
		[tv reloadDataForRowIndexes:set columnIndexes:cols];
		[cols release];
	}
	// set is autoreleased
}

void uiTableModelRowDeleted(uiTableModel *m, int oldIndex)
{
	NSTableView *tv;
	NSIndexSet *set;

	set = [NSIndexSet indexSetWithIndex:oldIndex];
	for (tv in m->tables)
		[tv removeRowsAtIndexes:set withAnimation:NSTableViewAnimationEffectNone];
	// set is autoreleased
}

void uiTableColumnAppend(uiTableColumn *c, uiTableCellPart *part, int expand)
{
	part->part.expand = expand;
	[c->parts addObject:part->part];
}

uiTableCellPart *uiNewTableTextPart(int modelColumn)
{
	uiTableCellPart *p;

	p = uiNew(uiTableCellPart);
	p->part = [tablePart new];
	p->part.type = partText;
	p->part.mainColumn = modelColumn;
	return p;
}

void uiFreeTableCellPart(uiTableCellPart *p)
{
	// TODO disallow if in use
	[p->part release];
	uiFree(p);
}

uiDarwinControlAllDefaultsExceptDestroy(uiTable, sv)

static void uiTableDestroy(uiControl *c)
{
	uiTable *t = uiTable(c);

	// TODO
	[t->sv release];
	uiFreeControl(uiControl(t));
}

uiTableColumn *uiTableAppendColumn(uiTable *t, const char *name)
{
	uiTableColumn *c;

	c = uiNew(uiTableColumn);
	c->c = [[tableColumn alloc] initWithIdentifier:@""];
	c->c.libui_col = c;
	// via Interface Builder
	[c->c setResizingMask:(NSTableColumnAutoresizingMask | NSTableColumnUserResizingMask)];
	[c->c setTitle:toNSString(name)];
	// TODO is this sufficient?
	[[c->c headerCell] setFont:[NSFont systemFontOfSize:[NSFont systemFontSizeForControlSize:NSSmallControlSize]]];
	c->parts = [NSMutableArray new];
	[t->tv addTableColumn:c->c];
	return c;
}

uiTable *uiNewTable(uiTableModel *model)
{
	uiTable *t;
	struct scrollViewCreateParams p;

	uiDarwinNewControl(uiTable, t);

	t->tv = [[NSTableView alloc] initWithFrame:NSZeroRect];

	[t->tv setDataSource:model->m];
	[t->tv setDelegate:model->m];
	[t->tv reloadData];
	[model->tables addObject:t->tv];

	// TODO is this sufficient?
	[t->tv setAllowsColumnReordering:NO];
	[t->tv setAllowsColumnResizing:YES];
	[t->tv setAllowsMultipleSelection:NO];
	[t->tv setAllowsEmptySelection:YES];
	[t->tv setAllowsColumnSelection:NO];
	[t->tv setUsesAlternatingRowBackgroundColors:YES];
	[t->tv setSelectionHighlightStyle:NSTableViewSelectionHighlightStyleRegular];
	[t->tv setGridStyleMask:NSTableViewGridNone];
	[t->tv setAllowsTypeSelect:YES];
	// TODO floatsGroupRows — do we even allow group rows?

	memset(&p, 0, sizeof (struct scrollViewCreateParams));
	p.DocumentView = t->tv;
	// this is what Interface Builder sets it to
	// TODO verify
	p.BackgroundColor = [NSColor colorWithCalibratedWhite:1.0 alpha:1.0];
	p.DrawsBackground = YES;
	p.Bordered = YES;
	p.HScroll = YES;
	p.VScroll = YES;
	t->sv = mkScrollView(&p, &(t->d));

	dispatch_after(
	dispatch_time(DISPATCH_TIME_NOW,3*NSEC_PER_SEC),
	dispatch_get_main_queue(),
	^{
	for(NSView *v in t->sv.subviews){
	NSLog(@"%@ %p %@", [v class], v, NSStringFromRect([v frame]));
	for(NSView *v2 in v.subviews)
	NSLog(@"    %@ %p %@", [v2 class], v2, NSStringFromRect([v2 frame]));
	}
	});

	return t;
}
