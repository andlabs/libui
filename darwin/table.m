// 21 june 2016
#import "uipriv_darwin.h"

// TODOs
// - initial state of table view is off
// - header cell seems off
// - background color shows up for a line or two below selection
// - editable NSTextFields have no intrinsic width
// - changing a part property does not refresh views
// - is the Y position of checkbox cells correct?
// - progressbars appear ABOVE the table header
// 	- threaded animation (which was known to have some issues: https://stackoverflow.com/questions/18142801/nstableview-nsprogressindicator-flickering-issues) is NOT the cause; happens regardless of setting (or does the setting not stick? TODO)

// LONGTERM
// - reuse row views instead of creating a new one each time

@interface tableModel : NSObject<NSTableViewDataSource, NSTableViewDelegate> {
	uiTableModel *libui_m;
}
- (id)initWithModel:(uiTableModel *)m;
- (IBAction)onAction:(id)sender;
@end

enum {
	partText,
	partImage,
	partButton,
	partCheckbox,
	partProgressBar,
};

@interface tablePart : NSObject
@property int type;
@property int textColumn;
@property int textColorColumn;
@property int imageColumn;
@property int valueColumn;
@property int expand;
@property int editable;
- (NSView *)mkView:(uiTableModel *)m row:(int)row;
@end

@interface tableColumn : NSTableColumn
@property uiTableColumn *libui_col;
@end

@interface tableView : NSTableView
@property uiTable *libui_t;
@end

struct uiTableModel {
	uiTableModelHandler *mh;
	tableModel *m;
	NSMutableArray *tables;
};

struct uiTableColumn {
	tableColumn *c;
	NSMutableArray *parts;
};

struct uiTable {
	uiDarwinControl c;
	NSScrollView *sv;
	tableView *tv;
	uiprivScrollViewData *d;
	int backgroundColumn;
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
			[v addConstraint:uiprivMkConstraint(v, NSLayoutAttributeLeading,
				NSLayoutRelationEqual,
				view, NSLayoutAttributeLeading,
				1, -xleft,
				@"uiTableColumn first part horizontal constraint")];
			prev = view;
			continue;
		}
		[v addConstraint:uiprivMkConstraint(prev, NSLayoutAttributeTrailing,
			NSLayoutRelationEqual,
			view, NSLayoutAttributeLeading,
			1, -xmiddle,
			@"uiTableColumn middle horizontal constraint")];
		prev = view;
	}
	[v addConstraint:uiprivMkConstraint(prev, NSLayoutAttributeTrailing,
		NSLayoutRelationEqual,
		v, NSLayoutAttributeTrailing,
		1, -xright,
		@"uiTableColumn last part horizontal constraint")];

	// and vertically
	for (view in views) {
		[v addConstraint:uiprivMkConstraint(view, NSLayoutAttributeCenterY,
			NSLayoutRelationEqual,
			v, NSLayoutAttributeCenterY,
			1, 0,
			@"uiTableColumn part vertical constraint")];
		// TODO avoid the need for this hack
		if ([view isKindOfClass:[NSImageView class]])
			[v addConstraint:uiprivMkConstraint(view, NSLayoutAttributeTop,
				NSLayoutRelationEqual,
				v, NSLayoutAttributeTop,
				1, 0,
				@"uiTableColumn part vertical top constraint")];
	}

done:
	[views release];
	[v setTranslatesAutoresizingMaskIntoConstraints:NO];
	// TODO autorelease?
	return v;
}

- (void)tableView:(NSTableView *)nstv didAddRowView:(NSTableRowView *)rv forRow:(NSInteger)row
{
	uiTableModel *m = self->libui_m;
	tableView *tv = (tableView *) nstv;
	uiTable *t = tv.libui_t;
	NSColor *color;

	if (t->backgroundColumn == -1)
		return;
	color = (NSColor *) ((*(m->mh->CellValue))(m->mh, m, row, t->backgroundColumn));
	if (color == nil)
		return;
	[rv setBackgroundColor:color];
	// TODO autorelease color? or release it?
}

- (IBAction)onAction:(id)sender
{
	uiTableModel *m = self->libui_m;
	NSView *view = (NSView *) sender;
	NSTableView *tv;
	NSInteger row;
	const void *data;

	row = -1;
	for (tv in m->tables) {
		row = [tv rowForView:view];
		if (row != -1)
			break;
	}
	if (row == -1)
		uiprivImplBug("table model action triggered on view with no associated table");

	if ([view isKindOfClass:[NSTextField class]])
		data = [[((NSTextField *) view) stringValue] UTF8String];
	else if ([view isKindOfClass:[NSButton class]]) {
		NSButton *b;

		b = (NSButton *) view;
//		if ([b buttonType] == NSSwitchButton)
			data = uiTableModelGiveInt([b state] == NSOnState);
// TODO there is no buttonType getter
if(1);		else
			data = NULL;
	} else
		uiprivImplBug("table model editing action triggered on non-editable view");

	// note the use of [view tag] — we need the model column, which we store in the view tag for relevant views below
	(*(m->mh->SetCellValue))(m->mh, m,
		row, [view tag],
		data);
	// always refresh the value in case the model rejected it
	// TODO only affect tv?
	uiTableModelRowChanged(m, row);
}

@end

@implementation tablePart

- (id)init
{
	self = [super init];
	if (self) {
		self.textColumn = -1;
		self.textColorColumn = -1;
	}
	return self;
}

- (NSView *)mkView:(uiTableModel *)m row:(int)row
{
	void *data;
	NSString *str;
	NSView *view;
	NSTextField *tf;
	NSImageView *iv;
	NSButton *b;
	NSProgressIndicator *p;
	int value;

	switch (self.type) {
	case partText:
		data = (*(m->mh->CellValue))(m->mh, m, row, self.textColumn);
		str = uiprivToNSString((char *) data);
		uiprivFree(data);
		tf = uiprivNewLabel(str);
		// TODO set wrap and ellipsize modes?
		if (self.textColorColumn != -1) {
			NSColor *color;

			color = (NSColor *) ((*(m->mh->CellValue))(m->mh, m, row, self.textColorColumn));
			if (color != nil)
				[tf setTextColor:color];
			// TODO release color
		}
		if (self.editable) {
			[tf setEditable:YES];
			[tf setTarget:m->m];
			[tf setAction:@selector(onAction:)];
		}
		[tf setTag:self.textColumn];
		view = tf;
		break;
	case partImage:
		data = (*(m->mh->CellValue))(m->mh, m, row, self.imageColumn);
		iv = [[NSImageView alloc] initWithFrame:NSZeroRect];
		[iv setImage:uiprivImageNSImage((uiImage *) data)];
		[iv setImageFrameStyle:NSImageFrameNone];
		[iv setImageAlignment:NSImageAlignCenter];
		[iv setImageScaling:NSImageScaleProportionallyDown];
		[iv setAnimates:NO];
		[iv setEditable:NO];
		[iv addConstraint:uiprivMkConstraint(iv, NSLayoutAttributeWidth,
			NSLayoutRelationEqual,
			iv, NSLayoutAttributeHeight,
			1, 0,
			@"uiTable image squareness constraint")];
		[iv setTag:self.imageColumn];
		view = iv;
		break;
	case partButton:
		// TODO buttons get clipped
		data = (*(m->mh->CellValue))(m->mh, m, row, self.textColumn);
		str = uiprivToNSString((char *) data);
		b = [[NSButton alloc] initWithFrame:NSZeroRect];
		[b setTitle:str];
		[b setButtonType:NSMomentaryPushInButton];
		[b setBordered:YES];
		[b setBezelStyle:NSRoundRectBezelStyle];
		uiDarwinSetControlFont(b, NSRegularControlSize);
		if (self.editable) {
			[b setTarget:m->m];
			[b setAction:@selector(onAction:)];
		} else
			[b setEnabled:NO];
		[b setTag:self.textColumn];
		view = b;
		break;
	case partCheckbox:
		data = (*(m->mh->CellValue))(m->mh, m, row, self.valueColumn);
		b = [[NSButton alloc] initWithFrame:NSZeroRect];
		[b setTitle:@""];
		[b setButtonType:NSSwitchButton];
		// doesn't seem to have an associated bezel style
		[b setBordered:NO];
		[b setTransparent:NO];
		uiDarwinSetControlFont(b, NSRegularControlSize);
		if (uiTableModelTakeInt(data) != 0)
			[b setState:NSOnState];
		else
			[b setState:NSOffState];
		if (self.editable) {
			[b setTarget:m->m];
			[b setAction:@selector(onAction:)];
		} else
			[b setEnabled:NO];
		[b setTag:self.valueColumn];
		view = b;
		break;
	case partProgressBar:
		data = (*(m->mh->CellValue))(m->mh, m, row, self.valueColumn);
		value = uiTableModelTakeInt(data);
		// TODO no intrinsic width
		p = [[NSProgressIndicator alloc] initWithFrame:NSZeroRect];
		[p setControlSize:NSRegularControlSize];
		[p setBezeled:YES];
		[p setStyle:NSProgressIndicatorBarStyle];
		if (value == -1) {
			[p setIndeterminate:YES];
			[p startAnimation:p];
		} else if (value == 100) {
			[p setIndeterminate:NO];
			[p setMaxValue:101];
			[p setDoubleValue:101];
			[p setDoubleValue:100];
			[p setMaxValue:100];
		} else {
			[p setIndeterminate:NO];
			[p setDoubleValue:(value + 1)];
			[p setDoubleValue:value];
		}
		view = p;
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

@implementation tableView
@end

void *uiTableModelStrdup(const char *str)
{
	// TODO don't we have this already?
	char *dup;

	dup = (char *) uiprivAlloc((strlen(str) + 1) * sizeof (char), "char[]");
	strcpy(dup, str);
	return dup;
}

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
