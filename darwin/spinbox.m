// 14 august 2015
#import "uipriv_darwin.h"

@interface libui_spinbox : NSView<NSTextFieldDelegate> {
	NSTextField *tf;
	NSNumberFormatter *formatter;
	NSStepper *stepper;

	NSInteger value;
	NSInteger minimum;
	NSInteger maximum;

	uiSpinbox *spinbox;
}
- (id)initWithFrame:(NSRect)r spinbox:(uiSpinbox *)sb;
// see https://github.com/andlabs/ui/issues/82
- (NSInteger)libui_value;
- (void)libui_setValue:(NSInteger)val;
- (void)setMinimum:(NSInteger)min;
- (void)setMaximum:(NSInteger)max;
- (IBAction)stepperClicked:(id)sender;
- (void)controlTextDidChange:(NSNotification *)note;
@end

struct uiSpinbox {
	uiDarwinControl c;
	libui_spinbox *spinbox;
	void (*onChanged)(uiSpinbox *, void *);
	void *onChangedData;
};

@implementation libui_spinbox

- (id)initWithFrame:(NSRect)r spinbox:(uiSpinbox *)sb
{
	NSDictionary *views;

	self = [super initWithFrame:r];
	if (self) {
		self->tf = newEditableTextField();
		[self->tf setTranslatesAutoresizingMaskIntoConstraints:NO];

		self->formatter = [NSNumberFormatter new];
		[self->formatter setFormatterBehavior:NSNumberFormatterBehavior10_4];
		[self->formatter setLocalizesFormat:NO];
		[self->formatter setUsesGroupingSeparator:NO];
		[self->formatter setHasThousandSeparators:NO];
		[self->formatter setAllowsFloats:NO];
		[self->tf setFormatter:self->formatter];

		self->stepper = [[NSStepper alloc] initWithFrame:NSZeroRect];
		[self->stepper setIncrement:1];
		[self->stepper setValueWraps:NO];
		[self->stepper setAutorepeat:YES];              // hold mouse button to step repeatedly
		[self->stepper setTranslatesAutoresizingMaskIntoConstraints:NO];

		[self->tf setDelegate:self];
		[self->stepper setTarget:self];
		[self->stepper setAction:@selector(stepperClicked:)];

		[self addSubview:self->tf];
		[self addSubview:self->stepper];
		views = [NSDictionary
			dictionaryWithObjectsAndKeys:self->tf, @"textfield",
				self->stepper, @"stepper",
				nil];
		addConstraint(self, @"H:|[textfield]-[stepper]|", nil, views);
		addConstraint(self, @"V:|[textfield]|", nil, views);
		addConstraint(self, @"V:|[stepper]|", nil, views);

		self->spinbox = sb;
	}
	return self;
}

- (void)dealloc
{
	[self->tf setDelegate:nil];
	[self->tf removeFromSuperview];
	[self->tf release];
	[self->formatter release];
	[self->stepper setTarget:nil];
	[self->stepper removeFromSuperview];
	[self->stepper release];
	[super dealloc];
}

- (NSInteger)libui_value
{
	return self->value;
}

- (void)libui_setValue:(NSInteger)val
{
	self->value = val;
	if (self->value < self->minimum)
		self->value = self->minimum;
	if (self->value > self->maximum)
		self->value = self->maximum;
	[self->tf setIntegerValue:self->value];
	[self->stepper setIntegerValue:self->value];
}

- (void)setMinimum:(NSInteger)min
{
	self->minimum = min;
	[self->formatter setMinimum:[NSNumber numberWithInteger:self->minimum]];
	[self->stepper setMinValue:((double) (self->minimum))];
}

- (void)setMaximum:(NSInteger)max
{
	self->maximum = max;
	[self->formatter setMaximum:[NSNumber numberWithInteger:self->maximum]];
	[self->stepper setMaxValue:((double) (self->maximum))];
}

- (IBAction)stepperClicked:(id)sender
{
	[self libui_setValue:[self->stepper integerValue]];
	(*(self->spinbox->onChanged))(self->spinbox, self->spinbox->onChangedData);
}

- (void)controlTextDidChange:(NSNotification *)note
{
	[self libui_setValue:[self->tf integerValue]];
	(*(self->spinbox->onChanged))(self->spinbox, self->spinbox->onChangedData);
}

@end

uiDarwinDefineControl(
	uiSpinbox,							// type name
	spinbox								// handle
)

intmax_t uiSpinboxValue(uiSpinbox *s)
{
	return [s->spinbox libui_value];
}

void uiSpinboxSetValue(uiSpinbox *s, intmax_t value)
{
	[s->spinbox libui_setValue:value];
}

void uiSpinboxOnChanged(uiSpinbox *s, void (*f)(uiSpinbox *, void *), void *data)
{
	s->onChanged = f;
	s->onChangedData = data;
}

static void defaultOnChanged(uiSpinbox *s, void *data)
{
	// do nothing
}

uiSpinbox *uiNewSpinbox(intmax_t min, intmax_t max)
{
	uiSpinbox *s;

	if (min >= max)
		complain("error: min >= max in uiNewSpinbox()");

	s = (uiSpinbox *) uiNewControl(uiSpinbox);

	s->spinbox = [[libui_spinbox alloc] initWithFrame:NSZeroRect spinbox:s];
	[s->spinbox setMinimum:min];
	[s->spinbox setMaximum:max];
	[s->spinbox libui_setValue:min];

	uiSpinboxOnChanged(s, defaultOnChanged, NULL);

	uiDarwinFinishNewControl(s, uiSpinbox);

	return s;
}
