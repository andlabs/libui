// 3 june 2018
#import "uipriv_darwin.h"
#import "table.h"

@interface uiprivTableModel : NSObject<NSTableViewDataSource, NSTableViewDelegate> {
	uiTableModel *m;
}
- (id)initWithModel:(uiTableModel *)model;
@end

@implementation uiprivTableModel

- (id)initWithModel:(uiTableModel *)m
{
	self = [super init];
	if (self)
		self->libui_m = m;
	return self;
}

- (NSInteger)numberOfRowsInTableView:(NSTableView *)tv
{
	return (*(self->m->mh->NumRows))(self->m->mh, self->m);
}

 - (NSView *)tableView:(NSTableView *)tv viewForTableColumn:(NSTableColumn *)cc row:(NSInteger)row
{
	uiprivTableColumn *c = (uiprivTableColumn *) cc;
	// TODO consider renaming this type to uiprivTableCellView
	uiprivColumnCellView *cv;

	cv = (uiprivColumnCellView *) [tv makeViewWithIdentifier:[c identifier] owner:self];
	if (cv == nil)
		cv = [c uiprivMakeCellView];
	[cv uiprivUpdate:row];
	return cv;
}

- (void)tableView:(NSTableView *)nstv didAddRowView:(NSTableRowView *)rv forRow:(NSInteger)row
{
	xx TODO set background color
}

@end

uiTableModel *uiNewTableModel(uiTableModelHandler *mh)
{
	uiTableModel *m;

	m = uiprivNew(uiTableModel);
	m->mh = mh;
	m->m = [[uiprivTableModel alloc] initWithModel:m];
	m->tables = [NSMutableArray new];
	return m;
}

void uiFreeTableModel(uiTableModel *m)
{
	if ([m->tables count] != 0)
		uiprivUserBug("You cannot free a uiTableModel while uiTables are using it.");
	[m->tables release];
	[m->m release];
	uiprivFree(m);
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
	NSTableRowView *rv;
	NSUInteger i, n;
	uiprivTableColumnView *cv;

	for (tv in m->tables) {
		rv = [tv rowViewForRow:index makeIfNecessary:NO];
		if (rv != nil) {
			xx TODO update colors
		}
		n = [[tv tableColumns] count];
		for (i = 0; i < n; i++) {
			cv = (uiprivTableCellView *) [tv viewForColumn:i row:index makeIfNecessary:NO];
			if (cv != nil)
				[cv uiprivUpdate:index];
		}
	}
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

uiDarwinControlAllDefaultsExceptDestroy(uiTable, sv)

static void uiTableDestroy(uiControl *c)
{
	uiTable *t = uiTable(c);

	// TODO
	[t->sv release];
	uiFreeControl(uiControl(t));
}

void uiTableSetRowBackgroundColorModelColumn(uiTable *t, int modelColumn)
{
	t->backgroundColumn = modelColumn;
	// TODO update all rows
}

uiTable *uiNewTable(uiTableModel *model)
{
	uiTable *t;
	uiprivScrollViewCreateParams p;

	uiDarwinNewControl(uiTable, t);
	t->m = model;

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

	memset(&p, 0, sizeof (uiprivScrollViewCreateParams));
	p.DocumentView = t->tv;
	// this is what Interface Builder sets it to
	// TODO verify
	p.BackgroundColor = [NSColor colorWithCalibratedWhite:1.0 alpha:1.0];
	p.DrawsBackground = YES;
	p.Bordered = YES;
	p.HScroll = YES;
	p.VScroll = YES;
	t->sv = uiprivMkScrollView(&p, &(t->d));

	t->backgroundColumn = -1;

	return t;
}
