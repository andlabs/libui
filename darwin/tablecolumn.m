// 3 june 2018
#import "uipriv_darwin.h"

// values from interface builder
#define textColumnLeading 2
#define textColumnTrailing 2
#define imageColumnLeading 3
#define imageTextColumnLeading 7

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
		layoutCellSubview(self, self->tf,
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

xx TODO somehow merge this with the above
@interface uiprivImageTextColumnCellView : uiprivColumnCellView {
	uiTable *t;
	uiTableModel *m;
	NSImageView *iv;
	int modelColumn;
	NSTextField *tf;
	int textModelColumn;
	int textEditableColumn;
	uiTableTextColumnOptionalParams params;
}
- (id)initWithFrame:(NSRect)r table:(uiTable *)table model:(uiTableModel *)model modelColumn:(int)mc textModelColumn:(int)tmc editableColumn:(int)ec params:(uiTableTextColumnOptionalParams *)p;
- (IBAction)uiprivOnAction:(id)sender;
@end

@implementation uiprivImageTextColumnCellView

- (id)initWithFrame:(NSRect)r table:(uiTable *)table model:(uiTableModel *)model modelColumn:(int)mc textModelColumn:(int)tmc editableColumn:(int)ec params:(uiTableTextColumnOptionalParams *)p
{
	self = [super initWithFrame:frame];
	if (self) {
		self->t = table;
		self->m = model;
		self->modelColumn = mc;
		self->textModelColumn = tmc;
		self->editableColumn = ec;
		if (p != NULL)
			params = *p;
		else
			params = defaultTextColumnOptionalParams;

		self->iv = [[NSImageView alloc] initWithFrame:NSZeroRect];
		[self->iv setImageFrameStyle:NSImageFrameNone];
		[self->iv setImageAlignment:NSImageAlignCenter];
		[self->iv setImageScaling:NSImageScaleProportionallyDown];
		[self->iv setAnimates:NO];
		[self->iv setEditable:NO];
		[self->iv addConstraint:uiprivMkConstraint(self->iv, NSLayoutAttributeWidth,
			NSLayoutRelationEqual,
			self->iv, NSLayoutAttributeHeight,
			1, 0,
			@"uiTable image squareness constraint")];
		[self addSubview:self->iv];

		self->tf = nil;
		if (self->textModelColumn != -1) {
			self->tf = uiprivNewLabel(@"");
			// TODO set wrap and ellipsize modes?
			[self->tf setTarget:self];
			[self->tf setAction:@selector(uiprivOnAction:)];
			[self addSubview:self->tf];
			layoutCellSubview(self, self->iv,
				self, imageColumnLeading,
				nil, 0,
				NO);
			layoutCellSubview(self, self->tf,
				self, imageTextColumnLeading,
				self, textColumnTrailing,
				YES);
		} else {
			layoutCellSubview(self, self->iv,
				nil, 0,
				nil, 0,
				NO);
			[self addConstraint:uiprivMkConstraint(self, NSLayoutAttributeCenterX,
				NSLayoutRelationEqual,
				self->iv, NSLayoutAttributeCenterX,
				1, 0,
				@"uiTable image centering constraint")];
		}

		// take advantage of NSTableCellView-provided accessibility features
		[self setImageView:self->iv];
		if (self->tf != nil)
			[self setTextField:self->tf];
	}
	return self;
}

- (void)dealloc
{
	if (self->tf != nil) {
		[self->tf release];
		self->tf = nil;
	}
	[self->iv release];
	self->iv = nil;
	[super dealloc];
}

- (void)uiprivUpdate:(NSInteger)row
{
	void *data;

	data = (*(self->m->mh->CellValue))(self->m->mh, self->m, row, self->modelColumn);
	[self->iv setImage:uiprivImageNSImage((uiImage *) data)];
	if (self->tf != nil)
		updateCellTextField(self->tf, row, self->m,
			self->textModelColumn, self->editableColumn, &(self->params));
}

- (IBAction)onAction:(id)sender
{
	NSInteger row;
	const void *data;

	row = [self->t->tv rowForView:self->tf];
	data = [[self->tf stringValue] UTF8String];
	(*(self->m->mh->SetCellValue))(self->m->mh, self->m,
		row, self->textModelColumn, data);
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
