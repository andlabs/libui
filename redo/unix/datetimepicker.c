// 11 june 2015
#include "uipriv_unix.h"

struct datetimepicker {
	uiDateTimePicker d;
	GtkWidget *widget;
};

uiDefineControlType(uiDateTimePicker, uiTypeDateTimePicker, struct datetimepicker)

static uintptr_t datetimepickerHandle(uiControl *c)
{
	struct datetimepicker *d = (struct datetimepicker *) c;

	return (uintptr_t) (d->widget);
}

uiDateTimePicker *finishNewDateTimePicker(OSTHING OSARG)
{
	struct datetimepicker *d;

	d = (struct datetimepicker *) MAKE_CONTROL_INSTANCE(uiTypeDateTimePicker());

	PUT_CODE_HERE;

	uiControl(d)->Handle = datetimepickerHandle;

	return uiDateTimePicker(d);
}

uiDateTimePicker *uiNewDateTimePicker(void)
{
	return finishNewDateTimePicker(OSARGDATETIME);
}

uiDateTimePicker *uiNewDatePicker(void)
{
	return finishNewDateTimePicker(OSARGDATEONLY);
}

uiDateTimePicker *uiNewTimePicker(void)
{
	return finishNewDateTimePicker(OSARGTIMEONLY);
}
