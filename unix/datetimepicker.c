// 11 june 2015
#include "uipriv_unix.h"

struct uiDateTimePicker {
	uiUnixControl c;
	GtkWidget *widget;
};

uiUnixDefineControl(
	uiDateTimePicker,						// type name
	uiDateTimePickerType					// type function
)

// TODO
typedef int OSTHING;
enum {
	OSARGDATETIME,
	OSARGDATEONLY,
	OSARGTIMEONLY,
};

uiDateTimePicker *finishNewDateTimePicker(OSTHING OSARG)
{
	uiDateTimePicker *d;

	d = (uiDateTimePicker *) uiNewControl(uiDateTimePickerType());

	d->widget = gtk_label_new("TODO uiDateTimePicker not implemented");

	uiUnixFinishNewControl(d, uiDateTimePicker);

	return d;
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
