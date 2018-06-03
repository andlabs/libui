// 3 june 2018
#import "uipriv_darwin.h"

// values from interface builder
#define textColumnLeading 2
#define textColumnTrailing 2

static void layoutCellSubview(NSView *superview, NSView *subview, NSView *leading, CGFloat leadingConstant, NSView *trailing, CGFloat trailingConstant, BOOL stretchy)
{
	[subview setTranslatesAutoresizingMaskIntoConstraints:NO];
	if (stretchy)
		[subview setContentHuggingPriority:NSLayoutPriorityDefaultLow forOrientation:NSLayoutConstraintOrientationHorizontal];
	else
		[subview setContentHuggingPriority:NSLayoutPriorityRequired forOrientation:NSLayoutConstraintOrientationHorizontal];
	if (leading != nil)
		[superview addConstraint:uiprivMkConstraint(leading, NSLayoutAttributeLeading,
			NSLayoutRelationEqual,
			subview, NSLayoutAttributeLeading,
			1, -leadingConstant,
			@"uiTable cell subview leading constraint")];
	[superview addConstraint:uiprivMkConstraint(superview, NSLayoutAttributeTop,
		NSLayoutRelationEqual,
		subview, NSLayoutAttributeTop,
		1, 0,
		@"uiTable cell subview top constraint")];
	if (trailing != nil)
		[superview addConstraint:uiprivMkConstraint(trailing, NSLayoutAttributeTrailing,
			NSLayoutRelationEqual,
			subview, NSLayoutAttributeLeading,
			1, trailingConstant,
			@"uiTable cell subview trailing constraint")];
	[superview addConstraint:uiprivMkConstraint(superview, NSLayoutAttributeBottom,
		NSLayoutRelationEqual,
		subview, NSLayoutAttributeBottom,
		1, 0,
		@"uiTable cell subview bottom constraint")];
}

@interface uiprivColumnCellView : NSTableCellView
- (void)uiprivUpdate:(NSInteger)row;
@end

@implementation uiprivColumnCellView

- (void)uiprivUpdate:(NSInteger)row
{
	[self doesNotRecognizeSelector:_cmd];
}

@end

static uiTableTextColumnOptionalParams defaultTextColumnOptionalParams = {
	.ColorModelColumn = -1,
};

static void updateCellTextField(NSTextField *tf, NSInteger row, uiTableModel *m, int modelColumn, int editableColumn, uiTableTextColumnOptionalParams *params)
{
	void *data;
	NSString *str;
	BOOL editable;

	data = (*(m->mh->CellValue))(m->mh, m, row, modelColumn);
	str = uiprivToNSString((char *) data);
	uiprivFree(data);
	[tf setStringValue:str];

	switch (editableColumn) {
	case uiTableModelColumnNeverEditable:
		editable = NO;
		break;
	case uiTableModelColumnAlwaysEditable:
		editable = YES;
		break;
	default:
		data = (*(m->mh->CellValue))(m->mh, m, row, editableColumn);
		editable = uiTableModelTakeInt(data) != 0;
		// TODO free data
	}
	[tf setEditable:editable];

	color = nil;
	if (params->ColorModelColumn != -1)
		color = (NSColor *) ((*(m->mh->CellValue))(m->mh, m, row, params->ColorModelColumn));
	if (color == nil)
		color = [NSColor controlTextColor];
	[tf setColor:color];
	// TODO release color
}

@interface uiprivTextColumnCellView : uiprivColumnCellView {
	uiTable *t;
	uiTableModel *m;
	NSTextField *tf;
	int modelColumn;
	int editableColumn;
	uiTableTextColumnOptionalParams params;
}
- (id)initWithFrame:(NSRect)r table:(uiTable *)table model:(uiTableModel *)model modelColumn:(int)mc editableColumn:(int)ec params:(uiTableTextColumnOptionalParams *)p;
- (IBAction)uiprivOnAction:(id)sender;
@end

@implementation uiprivTextColumnCellView

- (id)initWithFrame:(NSRect)r ModelColumn:(int)mc editableColumn:(int)ec params:(uiTableTextColumnOptionalParams *)p
{
	self = [super initWithFrame:frame];
	if (self) {
		self->t = table;
		self->m = model;
		self->modelColumn = mc;
		self->editableColumn = ec;
		if (p != NULL)
			params = *p;
		else
			params = defaultTextColumnOptionalParams;

		self->tf = uiprivNewLabel(@"");
		// TODO set wrap and ellipsize modes?
		[self->tf setTarget:self];
		[self->tf setAction:@selector(uiprivOnAction:)];
		[self addSubview:self->tf];
		layoutCellSubview(self, self->tv,
			self, textColumnLeading,
			self, textColumnTrailing,
			YES);

		// take advantage of NSTableCellView-provided accessibility features
		[self setTextField:self->tf];
	}
	return self;
}

- (void)dealloc
{
	[self->tf release];
	self->tf = nil;
	[super dealloc];
}

- (void)uiprivUpdate:(NSInteger)row
{
	updateCellTextField(self->tf, row, self->m,
		self->modelColumn, self->editableColumn, &(self->params));
}

- (IBAction)onAction:(id)sender
{
	NSInteger row;
	const void *data;

	row = [self->t->tv rowForView:self->tf];
	data = [[self->tf stringValue] UTF8String];
	(*(self->m->mh->SetCellValue))(self->m->mh, self->m,
		row, self->modelColumn, data);
	// always refresh the value in case the model rejected it
	[self uiprivUpdate:row];
}

@end

void uiTableAppendTextColumn(uiTable *t,
	const char *name,
	int textModelColumn,
	int textEditableModelColumn,
	uiTableTextColumnOptionalParams *params);

void uiTableAppendImageColumn(uiTable *t,
	const char *name,
	int imageModelColumn);

void uiTableAppendImageTextColumn(uiTable *t,
	const char *name,
	int imageModelColumn,
	int textModelColumn,
	int textEditableModelColumn,
	uiTableTextColumnOptionalParams *textParams);

void uiTableAppendCheckboxColumn(uiTable *t,
	const char *name,
	int checkboxModelColumn,
	int checkboxEditableModelColumn);

void uiTableAppendCheckboxTextColumn(uiTable *t,
	const char *name,
	int checkboxModelColumn,
	int checkboxEditableModelColumn,
	int textModelColumn,
	int textEditableModelColumn,
	uiTableTextColumnOptionalParams *textParams);

void uiTableAppendProgressBarColumn(uiTable *t,
	const char *name,
	int progressModelColumn);

void uiTableAppendButtonColumn(uiTable *t,
	const char *name,
	int buttonTextModelColumn,
	int buttonClickableModelColumn);
