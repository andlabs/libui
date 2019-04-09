// 3 june 2018
#import "uipriv_darwin.h"
#import "table.h"

// values from interface builder
#define textColumnLeading 2
#define textColumnTrailing 2
#define imageColumnLeading 3
#define imageTextColumnLeading 7
#define checkboxTextColumnLeading 0
// these aren't provided by IB; let's just choose one
#define checkboxColumnLeading imageColumnLeading
#define progressBarColumnLeading imageColumnLeading
#define progressBarColumnTrailing progressBarColumnLeading
#define buttonColumnLeading imageColumnLeading
#define buttonColumnTrailing buttonColumnLeading

@implementation uiprivTableCellView

- (void)uiprivUpdate:(NSInteger)row
{
	[self doesNotRecognizeSelector:_cmd];
}

@end

@implementation uiprivTableColumn

- (uiprivTableCellView *)uiprivMakeCellView
{
	[self doesNotRecognizeSelector:_cmd];
	return nil;			// appease compiler
}

@end

struct textColumnCreateParams {
	uiTable *t;
	uiTableModel *m;

	BOOL makeTextField;
	int textModelColumn;
	int textEditableModelColumn;
	uiTableTextColumnOptionalParams textParams;

	BOOL makeImageView;
	int imageModelColumn;

	BOOL makeCheckbox;
	int checkboxModelColumn;
	int checkboxEditableModelColumn;
};

@interface uiprivTextImageCheckboxTableCellView : uiprivTableCellView {
	uiTable *t;
	uiTableModel *m;

	NSTextField *tf;
	int textModelColumn;
	int textEditableModelColumn;
	uiTableTextColumnOptionalParams textParams;

	NSImageView *iv;
	int imageModelColumn;

	NSButton *cb;
	int checkboxModelColumn;
	int checkboxEditableModelColumn;
}
- (id)initWithFrame:(NSRect)r params:(struct textColumnCreateParams *)p;
- (IBAction)uiprivOnTextFieldAction:(id)sender;
- (IBAction)uiprivOnCheckboxAction:(id)sender;
@end

@implementation uiprivTextImageCheckboxTableCellView

- (id)initWithFrame:(NSRect)r params:(struct textColumnCreateParams *)p
{
	self = [super initWithFrame:r];
	if (self) {
		NSMutableArray *constraints;

		self->t = p->t;
		self->m = p->m;
		constraints = [NSMutableArray new];

		self->tf = nil;
		if (p->makeTextField) {
			self->textModelColumn = p->textModelColumn;
			self->textEditableModelColumn = p->textEditableModelColumn;
			self->textParams = p->textParams;

			self->tf = uiprivNewLabel(@"");
			// TODO set wrap and ellipsize modes?
			[self->tf setTarget:self];
			[self->tf setAction:@selector(uiprivOnTextFieldAction:)];
			[self->tf setTranslatesAutoresizingMaskIntoConstraints:NO];
			[self addSubview:self->tf];

			// TODO for all three controls: set hugging and compression resistance properly
			[constraints addObject:uiprivMkConstraint(self, NSLayoutAttributeLeading,
				NSLayoutRelationEqual,
				self->tf, NSLayoutAttributeLeading,
				1, -textColumnLeading,
				@"uiTable cell text leading constraint")];
			[constraints addObject:uiprivMkConstraint(self, NSLayoutAttributeTop,
				NSLayoutRelationEqual,
				self->tf, NSLayoutAttributeTop,
				1, 0,
				@"uiTable cell text top constraint")];
			[constraints addObject:uiprivMkConstraint(self, NSLayoutAttributeTrailing,
				NSLayoutRelationEqual,
				self->tf, NSLayoutAttributeTrailing,
				1, textColumnTrailing,
				@"uiTable cell text trailing constraint")];
			[constraints addObject:uiprivMkConstraint(self, NSLayoutAttributeBottom,
				NSLayoutRelationEqual,
				self->tf, NSLayoutAttributeBottom,
				1, 0,
				@"uiTable cell text bottom constraint")];
		}

		self->iv = nil;
		if (p->makeImageView) {
			self->imageModelColumn = p->imageModelColumn;

			self->iv = [[NSImageView alloc] initWithFrame:NSZeroRect];
			[self->iv setImageFrameStyle:NSImageFrameNone];
			[self->iv setImageAlignment:NSImageAlignCenter];
			[self->iv setImageScaling:NSImageScaleProportionallyDown];
			[self->iv setAnimates:NO];
			[self->iv setEditable:NO];
			[self->iv setTranslatesAutoresizingMaskIntoConstraints:NO];
			[self addSubview:self->iv];

			[constraints addObject:uiprivMkConstraint(self->iv, NSLayoutAttributeWidth,
				NSLayoutRelationEqual,
				self->iv, NSLayoutAttributeHeight,
				1, 0,
				@"uiTable image squareness constraint")];
			if (self->tf != nil) {
				[constraints addObject:uiprivMkConstraint(self, NSLayoutAttributeLeading,
					NSLayoutRelationEqual,
					self->iv, NSLayoutAttributeLeading,
					1, -imageColumnLeading,
					@"uiTable cell image leading constraint")];
				[constraints replaceObjectAtIndex:0
					withObject:uiprivMkConstraint(self->iv, NSLayoutAttributeTrailing,
						NSLayoutRelationEqual,
						self->tf, NSLayoutAttributeLeading,
						1, -imageTextColumnLeading,
						@"uiTable cell image-text spacing constraint")];
			} else
				[constraints addObject:uiprivMkConstraint(self, NSLayoutAttributeCenterX,
					NSLayoutRelationEqual,
					self->iv, NSLayoutAttributeCenterX,
					1, 0,
					@"uiTable cell image centering constraint")];
			[constraints addObject:uiprivMkConstraint(self, NSLayoutAttributeTop,
				NSLayoutRelationEqual,
				self->iv, NSLayoutAttributeTop,
				1, 0,
				@"uiTable cell image top constraint")];
			[constraints addObject:uiprivMkConstraint(self, NSLayoutAttributeBottom,
				NSLayoutRelationEqual,
				self->iv, NSLayoutAttributeBottom,
				1, 0,
				@"uiTable cell image bottom constraint")];
		}

		self->cb = nil;
		if (p->makeCheckbox) {
			self->checkboxModelColumn = p->checkboxModelColumn;
			self->checkboxEditableModelColumn = p->checkboxEditableModelColumn;

			self->cb = [[NSButton alloc] initWithFrame:NSZeroRect];
			[self->cb setTitle:@""];
			[self->cb setButtonType:NSSwitchButton];
			// doesn't seem to have an associated bezel style
			[self->cb setBordered:NO];
			[self->cb setTransparent:NO];
			uiDarwinSetControlFont(self->cb, NSRegularControlSize);
			[self->cb setTranslatesAutoresizingMaskIntoConstraints:NO];
			[self addSubview:self->cb];

			if (self->tf != nil) {
				[constraints addObject:uiprivMkConstraint(self, NSLayoutAttributeLeading,
					NSLayoutRelationEqual,
					self->cb, NSLayoutAttributeLeading,
					1, -imageColumnLeading,
					@"uiTable cell checkbox leading constraint")];
				[constraints replaceObjectAtIndex:0
					withObject:uiprivMkConstraint(self->cb, NSLayoutAttributeTrailing,
						NSLayoutRelationEqual,
						self->tf, NSLayoutAttributeLeading,
						1, -imageTextColumnLeading,
						@"uiTable cell checkbox-text spacing constraint")];
			} else
				[constraints addObject:uiprivMkConstraint(self, NSLayoutAttributeCenterX,
					NSLayoutRelationEqual,
					self->cb, NSLayoutAttributeCenterX,
					1, 0,
					@"uiTable cell checkbox centering constraint")];
			[constraints addObject:uiprivMkConstraint(self, NSLayoutAttributeTop,
				NSLayoutRelationEqual,
				self->cb, NSLayoutAttributeTop,
				1, 0,
				@"uiTable cell checkbox top constraint")];
			[constraints addObject:uiprivMkConstraint(self, NSLayoutAttributeBottom,
				NSLayoutRelationEqual,
				self->cb, NSLayoutAttributeBottom,
				1, 0,
				@"uiTable cell checkbox bottom constraint")];
		}

		[self addConstraints:constraints];

		// take advantage of NSTableCellView-provided accessibility features
		if (self->tf != nil)
			[self setTextField:self->tf];
		if (self->iv != nil)
			[self setImageView:self->iv];
	}
	return self;
}

- (void)dealloc
{
	if (self->cb != nil) {
		[self->cb release];
		self->cb = nil;
	}
	if (self->iv != nil) {
		[self->iv release];
		self->iv = nil;
	}
	if (self->tf != nil) {
		[self->tf release];
		self->tf = nil;
	}
	[super dealloc];
}

- (void)uiprivUpdate:(NSInteger)row
{
	uiTableValue *value;

	if (self->tf != nil) {
		NSString *str;
		NSColor *color;
		double r, g, b, a;

		value = uiprivTableModelCellValue(self->m, row, self->textModelColumn);
		str = uiprivToNSString(uiTableValueString(value));
		uiFreeTableValue(value);
		[self->tf setStringValue:str];

		[self->tf setEditable:uiprivTableModelCellEditable(self->m, row, self->textEditableModelColumn)];

		color = [NSColor controlTextColor];
		if (uiprivTableModelColorIfProvided(self->m, row, self->textParams.ColorModelColumn, &r, &g, &b, &a))
			color = [NSColor colorWithSRGBRed:r green:g blue:b alpha:a];
		[self->tf setTextColor:color];
		// we don't own color in ether case; don't release
	}
	if (self->iv != nil) {
		uiImage *img;

		value = uiprivTableModelCellValue(self->m, row, self->imageModelColumn);
		img = uiTableValueImage(value);
		uiFreeTableValue(value);
		[self->iv setImage:uiprivImageNSImage(img)];
	}
	if (self->cb != nil) {
		value = uiprivTableModelCellValue(self->m, row, self->checkboxModelColumn);
		if (uiTableValueInt(value) != 0)
			[self->cb setState:NSOnState];
		else
			[self->cb setState:NSOffState];
		uiFreeTableValue(value);

		[self->cb setEnabled:uiprivTableModelCellEditable(self->m, row, self->checkboxEditableModelColumn)];
	}
}

- (IBAction)uiprivOnTextFieldAction:(id)sender
{
	NSInteger row;
	uiTableValue *value;

	row = [self->t->tv rowForView:self->tf];
	value = uiNewTableValueString([[self->tf stringValue] UTF8String]);
	uiprivTableModelSetCellValue(self->m, row, self->textModelColumn, value);
	uiFreeTableValue(value);
	// always refresh the value in case the model rejected it
	// TODO document that we do this, but not for the whole row (or decide to do both, or do neither...)
	[self uiprivUpdate:row];
}

- (IBAction)uiprivOnCheckboxAction:(id)sender
{
	NSInteger row;
	uiTableValue *value;

	row = [self->t->tv rowForView:self->cb];
	value = uiNewTableValueInt([self->cb state] != NSOffState);
	uiprivTableModelSetCellValue(self->m, row, self->checkboxModelColumn, value);
	uiFreeTableValue(value);
	// always refresh the value in case the model rejected it
	[self uiprivUpdate:row];
}

@end

@interface uiprivTextImageCheckboxTableColumn : uiprivTableColumn {
	struct textColumnCreateParams params;
}
- (id)initWithIdentifier:(NSString *)ident params:(struct textColumnCreateParams *)p;
@end

@implementation uiprivTextImageCheckboxTableColumn

- (id)initWithIdentifier:(NSString *)ident params:(struct textColumnCreateParams *)p
{
	self = [super initWithIdentifier:ident];
	if (self)
		self->params = *p;
	return self;
}

- (uiprivTableCellView *)uiprivMakeCellView
{
	uiprivTableCellView *cv;

	cv = [[uiprivTextImageCheckboxTableCellView alloc] initWithFrame:NSZeroRect params:&(self->params)];
	[cv setIdentifier:[self identifier]];
	return cv;
}

@end

@interface uiprivProgressBarTableCellView : uiprivTableCellView {
	uiTable *t;
	uiTableModel *m;
	NSProgressIndicator *p;
	int modelColumn;
}
- (id)initWithFrame:(NSRect)r table:(uiTable *)table model:(uiTableModel *)model modelColumn:(int)mc;
@end

@implementation uiprivProgressBarTableCellView

- (id)initWithFrame:(NSRect)r table:(uiTable *)table model:(uiTableModel *)model modelColumn:(int)mc
{
	self = [super initWithFrame:r];
	if (self) {
		self->t = table;
		self->m = model;
		self->modelColumn = mc;

		self->p = [[NSProgressIndicator alloc] initWithFrame:NSZeroRect];
		[self->p setControlSize:NSRegularControlSize];
		[self->p setBezeled:YES];
		[self->p setStyle:NSProgressIndicatorBarStyle];
		[self->p setTranslatesAutoresizingMaskIntoConstraints:NO];
		[self addSubview:self->p];

		// TODO set hugging and compression resistance properly
		[self addConstraint:uiprivMkConstraint(self, NSLayoutAttributeLeading,
			NSLayoutRelationEqual,
			self->p, NSLayoutAttributeLeading,
			1, -progressBarColumnLeading,
			@"uiTable cell progressbar leading constraint")];
		[self addConstraint:uiprivMkConstraint(self, NSLayoutAttributeTop,
			NSLayoutRelationEqual,
			self->p, NSLayoutAttributeTop,
			1, 0,
			@"uiTable cell progressbar top constraint")];
		[self addConstraint:uiprivMkConstraint(self, NSLayoutAttributeTrailing,
			NSLayoutRelationEqual,
			self->p, NSLayoutAttributeTrailing,
			1, progressBarColumnTrailing,
			@"uiTable cell progressbar trailing constraint")];
		[self addConstraint:uiprivMkConstraint(self, NSLayoutAttributeBottom,
			NSLayoutRelationEqual,
			self->p, NSLayoutAttributeBottom,
			1, 0,
			@"uiTable cell progressbar bottom constraint")];
	}
	return self;
}

- (void)dealloc
{
	[self->p release];
	self->p = nil;
	[super dealloc];
}

- (void)uiprivUpdate:(NSInteger)row
{
	uiTableValue *value;
	int progress;

	value = uiprivTableModelCellValue(self->m, row, self->modelColumn);
	progress = uiTableValueInt(value);
	uiFreeTableValue(value);
	if (progress == -1) {
		[self->p setIndeterminate:YES];
		[self->p startAnimation:self->p];
	} else if (progress == 100) {
		[self->p setIndeterminate:NO];
		[self->p setMaxValue:101];
		[self->p setDoubleValue:101];
		[self->p setDoubleValue:100];
		[self->p setMaxValue:100];
	} else {
		[self->p setIndeterminate:NO];
		[self->p setDoubleValue:(progress + 1)];
		[self->p setDoubleValue:progress];
	}
}

@end

@interface uiprivProgressBarTableColumn : uiprivTableColumn {
	uiTable *t;
	// TODO remove the need for this given t (or make t not require m, one of the two)
	uiTableModel *m;
	int modelColumn;
}
- (id)initWithIdentifier:(NSString *)ident table:(uiTable *)table model:(uiTableModel *)model modelColumn:(int)mc;
@end

@implementation uiprivProgressBarTableColumn

- (id)initWithIdentifier:(NSString *)ident table:(uiTable *)table model:(uiTableModel *)model modelColumn:(int)mc
{
	self = [super initWithIdentifier:ident];
	if (self) {
		self->t = table;
		self->m = model;
		self->modelColumn = mc;
	}
	return self;
}

- (uiprivTableCellView *)uiprivMakeCellView
{
	uiprivTableCellView *cv;

	cv = [[uiprivProgressBarTableCellView alloc] initWithFrame:NSZeroRect table:self->t model:self->m modelColumn:self->modelColumn];
	[cv setIdentifier:[self identifier]];
	return cv;
}

@end

@interface uiprivButtonTableCellView : uiprivTableCellView {
	uiTable *t;
	uiTableModel *m;
	NSButton *b;
	int modelColumn;
	int editableColumn;
}
- (id)initWithFrame:(NSRect)r table:(uiTable *)table model:(uiTableModel *)model modelColumn:(int)mc editableColumn:(int)ec;
- (IBAction)uiprivOnClicked:(id)sender;
@end

@implementation uiprivButtonTableCellView

- (id)initWithFrame:(NSRect)r table:(uiTable *)table model:(uiTableModel *)model modelColumn:(int)mc editableColumn:(int)ec
{
	self = [super initWithFrame:r];
	if (self) {
		self->t = table;
		self->m = model;
		self->modelColumn = mc;
		self->editableColumn = ec;

		self->b = [[NSButton alloc] initWithFrame:NSZeroRect];
		[self->b setButtonType:NSMomentaryPushInButton];
		[self->b setBordered:YES];
		[self->b setBezelStyle:NSRoundRectBezelStyle];
		uiDarwinSetControlFont(self->b, NSRegularControlSize);
		[self->b setTarget:self];
		[self->b setAction:@selector(uiprivOnClicked:)];
		[self->b setTranslatesAutoresizingMaskIntoConstraints:NO];
		[self addSubview:self->b];

		// TODO set hugging and compression resistance properly
		[self addConstraint:uiprivMkConstraint(self, NSLayoutAttributeLeading,
			NSLayoutRelationEqual,
			self->b, NSLayoutAttributeLeading,
			1, -buttonColumnLeading,
			@"uiTable cell button leading constraint")];
		[self addConstraint:uiprivMkConstraint(self, NSLayoutAttributeTop,
			NSLayoutRelationEqual,
			self->b, NSLayoutAttributeTop,
			1, 0,
			@"uiTable cell button top constraint")];
		[self addConstraint:uiprivMkConstraint(self, NSLayoutAttributeTrailing,
			NSLayoutRelationEqual,
			self->b, NSLayoutAttributeTrailing,
			1, buttonColumnTrailing,
			@"uiTable cell button trailing constraint")];
		[self addConstraint:uiprivMkConstraint(self, NSLayoutAttributeBottom,
			NSLayoutRelationEqual,
			self->b, NSLayoutAttributeBottom,
			1, 0,
			@"uiTable cell button bottom constraint")];
	}
	return self;
}

- (void)dealloc
{
	[self->b release];
	self->b = nil;
	[super dealloc];
}

- (void)uiprivUpdate:(NSInteger)row
{
	uiTableValue *value;
	NSString *str;

	value = uiprivTableModelCellValue(self->m, row, self->modelColumn);
	str = uiprivToNSString(uiTableValueString(value));
	uiFreeTableValue(value);
	[self->b setTitle:str];

	[self->b setEnabled:uiprivTableModelCellEditable(self->m, row, self->editableColumn)];
}

- (IBAction)uiprivOnClicked:(id)sender
{
	NSInteger row;

	row = [self->t->tv rowForView:self->b];
	uiprivTableModelSetCellValue(self->m, row, self->modelColumn, NULL);
	// TODO document we DON'T update the cell after doing this
	// TODO or decide what to do instead
}

@end

@interface uiprivButtonTableColumn : uiprivTableColumn {
	uiTable *t;
	uiTableModel *m;
	int modelColumn;
	int editableColumn;
}
- (id)initWithIdentifier:(NSString *)ident table:(uiTable *)table model:(uiTableModel *)model modelColumn:(int)mc editableColumn:(int)ec;
@end

@implementation uiprivButtonTableColumn

- (id)initWithIdentifier:(NSString *)ident table:(uiTable *)table model:(uiTableModel *)model modelColumn:(int)mc editableColumn:(int)ec
{
	self = [super initWithIdentifier:ident];
	if (self) {
		self->t = table;
		self->m = model;
		self->modelColumn = mc;
		self->editableColumn = ec;
	}
	return self;
}

- (uiprivTableCellView *)uiprivMakeCellView
{
	uiprivTableCellView *cv;

	cv = [[uiprivButtonTableCellView alloc] initWithFrame:NSZeroRect table:self->t model:self->m modelColumn:self->modelColumn editableColumn:self->editableColumn];
	[cv setIdentifier:[self identifier]];
	return cv;
}

@end

void uiTableAppendTextColumn(uiTable *t, const char *name, int textModelColumn, int textEditableModelColumn, uiTableTextColumnOptionalParams *textParams)
{
	struct textColumnCreateParams p;
	uiprivTableColumn *col;
	NSString *str;

	memset(&p, 0, sizeof (struct textColumnCreateParams));
	p.t = t;
	p.m = t->m;

	p.makeTextField = YES;
	p.textModelColumn = textModelColumn;
	p.textEditableModelColumn = textEditableModelColumn;
	if (textParams != NULL)
		p.textParams = *textParams;
	else
		p.textParams = uiprivDefaultTextColumnOptionalParams;

	str = [NSString stringWithUTF8String:name];
	col = [[uiprivTextImageCheckboxTableColumn alloc] initWithIdentifier:str params:&p];
	[col setTitle:str];
	[t->tv addTableColumn:col];
}

void uiTableAppendImageColumn(uiTable *t, const char *name, int imageModelColumn)
{
	struct textColumnCreateParams p;
	uiprivTableColumn *col;
	NSString *str;

	memset(&p, 0, sizeof (struct textColumnCreateParams));
	p.t = t;
	p.m = t->m;

	p.makeImageView = YES;
	p.imageModelColumn = imageModelColumn;

	str = [NSString stringWithUTF8String:name];
	col = [[uiprivTextImageCheckboxTableColumn alloc] initWithIdentifier:str params:&p];
	[col setTitle:str];
	[t->tv addTableColumn:col];
}

void uiTableAppendImageTextColumn(uiTable *t, const char *name, int imageModelColumn, int textModelColumn, int textEditableModelColumn, uiTableTextColumnOptionalParams *textParams)
{
	struct textColumnCreateParams p;
	uiprivTableColumn *col;
	NSString *str;

	memset(&p, 0, sizeof (struct textColumnCreateParams));
	p.t = t;
	p.m = t->m;

	p.makeTextField = YES;
	p.textModelColumn = textModelColumn;
	p.textEditableModelColumn = textEditableModelColumn;
	if (textParams != NULL)
		p.textParams = *textParams;
	else
		p.textParams = uiprivDefaultTextColumnOptionalParams;

	p.makeImageView = YES;
	p.imageModelColumn = imageModelColumn;

	str = [NSString stringWithUTF8String:name];
	col = [[uiprivTextImageCheckboxTableColumn alloc] initWithIdentifier:str params:&p];
	[col setTitle:str];
	[t->tv addTableColumn:col];
}

void uiTableAppendCheckboxColumn(uiTable *t, const char *name, int checkboxModelColumn, int checkboxEditableModelColumn)
{
	struct textColumnCreateParams p;
	uiprivTableColumn *col;
	NSString *str;

	memset(&p, 0, sizeof (struct textColumnCreateParams));
	p.t = t;
	p.m = t->m;

	p.makeCheckbox = YES;
	p.checkboxModelColumn = checkboxModelColumn;
	p.checkboxEditableModelColumn = checkboxEditableModelColumn;

	str = [NSString stringWithUTF8String:name];
	col = [[uiprivTextImageCheckboxTableColumn alloc] initWithIdentifier:str params:&p];
	[col setTitle:str];
	[t->tv addTableColumn:col];
}

void uiTableAppendCheckboxTextColumn(uiTable *t, const char *name, int checkboxModelColumn, int checkboxEditableModelColumn, int textModelColumn, int textEditableModelColumn, uiTableTextColumnOptionalParams *textParams)
{
	struct textColumnCreateParams p;
	uiprivTableColumn *col;
	NSString *str;

	memset(&p, 0, sizeof (struct textColumnCreateParams));
	p.t = t;
	p.m = t->m;

	p.makeTextField = YES;
	p.textModelColumn = textModelColumn;
	p.textEditableModelColumn = textEditableModelColumn;
	if (textParams != NULL)
		p.textParams = *textParams;
	else
		p.textParams = uiprivDefaultTextColumnOptionalParams;

	p.makeCheckbox = YES;
	p.checkboxModelColumn = checkboxModelColumn;
	p.checkboxEditableModelColumn = checkboxEditableModelColumn;

	str = [NSString stringWithUTF8String:name];
	col = [[uiprivTextImageCheckboxTableColumn alloc] initWithIdentifier:str params:&p];
	[col setTitle:str];
	[t->tv addTableColumn:col];
}

void uiTableAppendProgressBarColumn(uiTable *t, const char *name, int progressModelColumn)
{
	uiprivTableColumn *col;
	NSString *str;

	str = [NSString stringWithUTF8String:name];
	col = [[uiprivProgressBarTableColumn alloc] initWithIdentifier:str table:t model:t->m modelColumn:progressModelColumn];
	[col setTitle:str];
	[t->tv addTableColumn:col];
}

void uiTableAppendButtonColumn(uiTable *t, const char *name, int buttonModelColumn, int buttonClickableModelColumn)
{
	uiprivTableColumn *col;
	NSString *str;

	str = [NSString stringWithUTF8String:name];
	col = [[uiprivButtonTableColumn alloc] initWithIdentifier:str table:t model:t->m modelColumn:buttonModelColumn editableColumn:buttonClickableModelColumn];
	[col setTitle:str];
	[t->tv addTableColumn:col];
}
