// 11 june 2015
#include "uipriv_darwin.h"

struct datetimepicker {
	uiDateTimePicker d;
	NSDatePicker *dp;
};

uiDefineControlType(uiDateTimePicker, uiTypeDateTimePicker, struct datetimepicker)

static uintptr_t datetimepickerHandle(uiControl *c)
{
	struct datetimepicker *d = (struct datetimepicker *) c;

	return (uintptr_t) (d->dp);
}

uiDateTimePicker *finishNewDateTimePicker(NSDatePickerElementFlags elements)
{
	struct datetimepicker *d;

	d = (struct datetimepicker *) uiNewControl(uiTypeDateTimePicker());

	d->dp = [[NSDatePicker alloc] initWithFrame:NSZeroRect];
	// TODO text field stuff
	[d->dp setDatePickerStyle:NSTextFieldAndStepperDatePickerStyle];
	[d->dp setDatePickerElements:elements];
	[d->dp setDatePickerMode:NSSingleDateMode];
	// TODO get date picker font

	uiControl(d)->Handle = datetimepickerHandle;

	return uiDateTimePicker(d);
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
