// 14 august 2015
#import "uipriv_darwin.h"

struct uiDateTimePicker {
	uiDarwinControl c;
	NSDatePicker *dp;
};

uiDarwinDefineControl(
	uiDateTimePicker,						// type name
	uiDaateTimePickerType,					// type function
	dp									// handle
)

static uiDateTimePicker *finishNewDateTimePicker(NSDatePickerElementFlags elements)
{
	uiDateTimePicker *d;

	d = (uiDateTimePicker *) uiNewControl(uiDateTimePickerType());

	d->dp = [[NSDatePicker alloc] initWithFrame:NSZeroRect];
	// TODO text field stuff
	[d->dp setDatePickerStyle:NSTextFieldAndStepperDatePickerStyle];
	[d->dp setDatePickerElements:elements];
	[d->dp setDatePickerMode:NSSingleDateMode];
	// TODO get date picker font

	uiDarwinFinishNewControl(d, uiDateTimePicker);

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
