// 14 august 2015
#import "uipriv_darwin.h"

struct uiDateTimePicker {
	uiDarwinControl c;
	NSDatePicker *dp;
	void (*onChanged)(uiDateTimePicker *, void *);
	void *onChangedData;
};

@interface datePickerDelegateClass : NSObject <NSDatePickerCellDelegate> {
	struct mapTable *pickers;
}
- (void)datePickerCell:(NSDatePickerCell *)aDatePickerCell validateProposedDateValue:(NSDate **)proposedDateValue timeInterval:(NSTimeInterval *)proposedTimeInterval;
- (void)doTimer:(NSTimer *)timer;
- (void)registerPicker:(uiDateTimePicker *)b;
- (void)unregisterPicker:(uiDateTimePicker *)b;
@end

@implementation datePickerDelegateClass

- (id)init
{
	self = [super init];
	if (self)
		self->pickers = newMap();
	return self;
}

- (void)dealloc
{
	mapDestroy(self->pickers);
	[super dealloc];
}

- (void)datePickerCell:(NSDatePickerCell *)aDatePickerCell
	validateProposedDateValue:(NSDate **)proposedDateValue
	timeInterval:(NSTimeInterval *)proposedTimeInterval
{
	uiDateTimePicker *d;

	d = (uiDateTimePicker *) mapGet(self->pickers, aDatePickerCell);
	[NSTimer scheduledTimerWithTimeInterval:0
		target:self
		selector:@selector(doTimer:)
		userInfo:[NSValue valueWithPointer:d]
		repeats:NO];
}

- (void)doTimer:(NSTimer *)timer
{
	uiDateTimePicker *d;

	d = (uiDateTimePicker *) [((NSValue *)[timer userInfo]) pointerValue];
	(*(d->onChanged))(d, d->onChangedData);
}

- (void)registerPicker:(uiDateTimePicker *)d
{
	mapSet(self->pickers, d->dp.cell, d);
	[d->dp setDelegate:self];
}

- (void)unregisterPicker:(uiDateTimePicker *)d
{
	[d->dp setDelegate:nil];
	mapDelete(self->pickers, d->dp.cell);
}

@end

static datePickerDelegateClass *datePickerDelegate = nil;

uiDarwinControlAllDefaults(uiDateTimePicker, dp)

static void defaultOnChanged(uiDateTimePicker *d, void *data)
{
	// do nothing
}

void uiDateTimePickerTime(uiDateTimePicker *d, struct tm *time)
{
	time_t t;
	struct tm tmbuf;
	NSDate *date;

	date = [d->dp dateValue];
	t = (time_t) [date timeIntervalSince1970];

	tmbuf = *localtime(&t);
	memcpy(time, &tmbuf, sizeof(struct tm));
}

void uiDateTimePickerSetTime(uiDateTimePicker *d, const struct tm *time)
{
	time_t t;
	struct tm tmbuf;

	memcpy(&tmbuf, time, sizeof(struct tm));
	t = mktime(&tmbuf);

	[d->dp setDateValue:[NSDate dateWithTimeIntervalSince1970:t]];
}

void uiDateTimePickerOnChanged(uiDateTimePicker *d, void (*f)(uiDateTimePicker *, void *), void *data)
{
	d->onChanged = f;
	d->onChangedData = data;
}

static uiDateTimePicker *finishNewDateTimePicker(NSDatePickerElementFlags elements)
{
	uiDateTimePicker *d;

	uiDarwinNewControl(uiDateTimePicker, d);

	d->dp = [[NSDatePicker alloc] initWithFrame:NSZeroRect];
	[d->dp setDateValue:[NSDate date]];
	[d->dp setBordered:NO];
	[d->dp setBezeled:YES];
	[d->dp setDrawsBackground:YES];
	[d->dp setDatePickerStyle:NSTextFieldAndStepperDatePickerStyle];
	[d->dp setDatePickerElements:elements];
	[d->dp setDatePickerMode:NSSingleDateMode];
	uiDarwinSetControlFont(d->dp, NSRegularControlSize);

	if (datePickerDelegate == nil) {
		datePickerDelegate = [[datePickerDelegateClass new] autorelease];
		[delegates addObject:datePickerDelegate];
	}
	[datePickerDelegate registerPicker:d];
	uiDateTimePickerOnChanged(d, defaultOnChanged, NULL);

	return d;
}

uiDateTimePicker *uiNewDateTimePicker(void)
{
	return finishNewDateTimePicker(NSYearMonthDayDatePickerElementFlag | NSHourMinuteSecondDatePickerElementFlag);
}

uiDateTimePicker *uiNewDatePicker(void)
{
	return finishNewDateTimePicker(NSYearMonthDayDatePickerElementFlag);
}

uiDateTimePicker *uiNewTimePicker(void)
{
	return finishNewDateTimePicker(NSHourMinuteSecondDatePickerElementFlag);
}
