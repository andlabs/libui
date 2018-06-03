// 3 june 2018
#import "uipriv_darwin.h"

// values from interface builder
#define textColumnLeading 2
#define textColumnTrailing 2
#define imageColumnLeading 3
#define imageTextColumnLeading 7
#define checkboxTextColumnLeading 0
// these aren't provided by IB; let's just choose one
#define checkboxColumnLeading imageColumnLeading

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

static BOOL isCellEditable(uiTableModel *m, NSInteger row, int modelColumn)
{
	void *data;

	switch (modelColumn) {
	case uiTableModelColumnNeverEditable:
		return NO;
	case uiTableModelColumnAlwaysEditable:
		return YES;
	}
	data = (*(m->mh->CellValue))(m->mh, m, row, modelColumn);
	return uiTableModelTakeInt(data) != 0;
	// TODO free data
}

static uiTableTextColumnOptionalParams defaultTextColumnOptionalParams = {
	.ColorModelColumn = -1,
};

struct textColumnCreateParams {
	uiTable *t;
	uiTableModel *m;

	BOOL makeTextField;
	int textModelColumn;
	int textEditableColumn;
	uiTableTextColumnOptionalParams textParams;

	BOOL makeImage;
	int imageModelColumn;

	BOOL makeCheckbox;
	int checkboxModelColumn;
	int checkboxEditableColumn;
};

@interface uiprivTextImageCheckboxColumnCellView : uiprivColumnCellView {
	uiTable *t;
	uiTableModel *m;

	NSTextField *tf;
	int textModelColumn;
	int textEditableColumn;
	uiTableTextColumnOptionalParams textParams;

	NSImageView *iv;
	int imageModelColumn;

	NSButton *cb;
	int checkboxModelColumn;
	int checkboxEditableColumn;
}
- (id)initWithFrame:(NSRect)r params:(struct textColumnCreateParams *)p;
- (IBAction)uiprivOnTextFieldAction:(id)sender;
- (IBAction)uiprivOnCheckboxAction:(id)sender;
@end

@implementation uiprivTextColumnCellView

- (id)initWithFrame:(NSRect)r params:(struct textColumnCreateParams *)p
{
	self = [super initWithFrame:frame];
	if (self) {
		NSView *left;
		CGFloat leftConstant;
		CGFloat leftTextConstant;

		self->t = p->t;
		self->m = p->m;

		self->tf = nil;
		if (p->makeTextField) {
			self->textModelColumn = p->textModelColumn;
			self->textEditableColumn = p->textEditableColumn;
			self->textParams = p->textParams;

			self->tf = uiprivNewLabel(@"");
			// TODO set wrap and ellipsize modes?
			[self->tf setTarget:self];
			[self->tf setAction:@selector(uiprivOnTextFieldAction:)];
			[self addSubview:self->tf];
		}

		left = nil;

		self->iv = nil;
		if (p->makeImageView) {
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
			left = self->iv;
			leftConstant = imageColumnLeading;
			leftTextConstant = imageTextColumnLeading;
		}

		self->cb = nil;
		if (p->makeCheckbox) {
			self->cb = [[NSButton alloc] initWithFrame:NSZeroRect];
			[self->cb setTitle:@""];
			[self->cb setButtonType:NSSwitchButton];
			// doesn't seem to have an associated bezel style
			[self->cb setBordered:NO];
			[self->cb setTransparent:NO];
			uiDarwinSetControlFont(self->cb, NSRegularControlSize);
			[self addSubview:self->cb];
			left = self->cb;
			leftConstant = checkboxColumnLeading;
			leftTextConstant = checkboxTextColumnLeading;
		}

		if (self->tf != nil && left == nil)
			layoutCellSubview(self, self->tf,
				self, textColumnLeading,
				self, textColumnTrailing,
				YES);
		else if (self->tf != nil) {
			layoutCellSubview(self, left,
				self, leftConstant,
				nil, 0,
				NO);
			layoutCellSubview(self, self->tf,
				left, leftTextConstant,
				self, textColumnTrailing,
				YES);
		} else {
			layoutCellSubview(self, left,
				nil, 0,
				nil, 0,
				NO);
			[self addConstraint:uiprivMkConstraint(self, NSLayoutAttributeCenterX,
				NSLayoutRelationEqual,
				left, NSLayoutAttributeCenterX,
				1, 0,
				@"uiTable image/checkbox centering constraint")];
		}

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
	void *data;
	BOOL editable;

	if (self->tv != nil) {
		NSString *str;
		BOOL editable;

		data = (*(self->m->mh->CellValue))(self->m->mh, self->m, row, self->textModelColumn);
		str = uiprivToNSString((char *) data);
		uiprivFree(data);
		[self->tf setStringValue:str];

		[self->tf setEditable:isCellEditable(self->m, row, self->textEditableColumn)];

		color = nil;
		if (self->textParams.ColorModelColumn != -1)
			color = (NSColor *) ((*(self->m->mh->CellValue))(self->m->mh, self->m, row, self->textParams.ColorModelColumn));
		if (color == nil)
			color = [NSColor controlTextColor];
		[self->tf setColor:color];
		// TODO release color
	}
	if (self->iv != nil) {
		data = (*(self->m->mh->CellValue))(self->m->mh, self->m, row, self->imageModelColumn);
		[self->iv setImage:uiprivImageNSImage((uiImage *) data)];
	}
	if (self->cb != nil) {
		data = (*(self->m->mh->CellValue))(self->m->mh, self->m, row, self->imageModelColumn);
		if (TODO(data))
			[self->cb setState:NSOnState];
		else
			[self->cb setState:NSOffState];

		[self->cb setEditable:isCellEditable(self->m, row, self->checkboxEditableColumn)];
	}
}

- (IBAction)uiprivOnTextFieldAction:(id)sender
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

- (IBAction)uiprivOnCheckboxAction:(id)sender
{
	NSInteger row;
	int val;
	void *data;

	row = [self->t->tv rowForView:self->cb];
	val = [self->cb state] != NSOffState;
	data = TODO(val);
	(*(self->m->mh->SetCellValue))(self->m->mh, self->m,
		row, self->checkboxModelColumn, data);
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
