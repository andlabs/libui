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
	xx TODO consider renaming this type to uiprivTableCellView
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

=================== TODOTODO

uiTableModel *uiNewTableModel(uiTableModelHandler *mh)
{
	uiTableModel *m;

	m = uiprivNew(uiTableModel);
	m->mh = mh;
	m->m = [[tableModel alloc] initWithModel:m];
	m->tables = [NSMutableArray new];
	return m;
}

void *uiTableModelGiveColor(double r, double g, double b, double a)
{
	return [[NSColor colorWithSRGBRed:r green:g blue:b alpha:a] retain];
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
	NSIndexSet *set, *cols;

	set = [NSIndexSet indexSetWithIndex:index];
	for (tv in m->tables) {
		cols = [[NSIndexSet alloc] initWithIndexesInRange:NSMakeRange(0, [[tv tableColumns] count])];
		[tv reloadDataForRowIndexes:set columnIndexes:cols];
		// TODO this isn't enough
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

void uiTableColumnAppendTextPart(uiTableColumn *c, int modelColumn, int expand)
{
	tablePart *part;

	part = [tablePart new];
	part.type = partText;
	part.textColumn = modelColumn;
	part.expand = expand;
	[c->parts addObject:part];
}

void uiTableColumnAppendImagePart(uiTableColumn *c, int modelColumn, int expand)
{
	tablePart *part;

	part = [tablePart new];
	part.type = partImage;
	part.imageColumn = modelColumn;
	part.expand = expand;
	[c->parts addObject:part];
}

void uiTableColumnAppendButtonPart(uiTableColumn *c, int modelColumn, int expand)
{
	tablePart *part;

	part = [tablePart new];
	part.type = partButton;
	part.textColumn = modelColumn;
	part.expand = expand;
	part.editable = 1;		// editable by default
	[c->parts addObject:part];
}

void uiTableColumnAppendCheckboxPart(uiTableColumn *c, int modelColumn, int expand)
{
	tablePart *part;

	part = [tablePart new];
	part.type = partCheckbox;
	part.valueColumn = modelColumn;
	part.expand = expand;
	part.editable = 1;		// editable by default
	[c->parts addObject:part];
}

void uiTableColumnAppendProgressBarPart(uiTableColumn *c, int modelColumn, int expand)
{
	tablePart *part;

	part = [tablePart new];
	part.type = partProgressBar;
	part.valueColumn = modelColumn;
	part.expand = expand;
	[c->parts addObject:part];
}

void uiTableColumnPartSetEditable(uiTableColumn *c, int part, int editable)
{
	tablePart *p;

	p = (tablePart *) [c->parts objectAtIndex:part];
	p.editable = editable;
}

void uiTableColumnPartSetTextColor(uiTableColumn *c, int part, int modelColumn)
{
	tablePart *p;

	p = (tablePart *) [c->parts objectAtIndex:part];
	p.textColorColumn = modelColumn;
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

	c = uiprivNew(uiTableColumn);
	c->c = [[tableColumn alloc] initWithIdentifier:@""];
	c->c.libui_col = c;
	// via Interface Builder
	[c->c setResizingMask:(NSTableColumnAutoresizingMask | NSTableColumnUserResizingMask)];
	// 10.10 adds -[NSTableColumn setTitle:]; before then we have to do this
	[[c->c headerCell] setStringValue:uiprivToNSString(name)];
	// TODO is this sufficient?
	[[c->c headerCell] setFont:[NSFont systemFontOfSize:[NSFont systemFontSizeForControlSize:NSSmallControlSize]]];
	c->parts = [NSMutableArray new];
	[t->tv addTableColumn:c->c];
	return c;
}

void uiTableSetRowBackgroundColorModelColumn(uiTable *t, int modelColumn)
{
	t->backgroundColumn = modelColumn;
}

uiTable *uiNewTable(uiTableModel *model)
{
	uiTable *t;
	uiprivScrollViewCreateParams p;

	uiDarwinNewControl(uiTable, t);

	t->tv = [[tableView alloc] initWithFrame:NSZeroRect];
	t->tv.libui_t = t;

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
