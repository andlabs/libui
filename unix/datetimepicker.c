// 11 june 2015
#include "uipriv_unix.h"

struct uiDateTimePicker {
	uiUnixControl c;
	GtkWidget *widget;
};

uiUnixControlAllDefaults(uiDateTimePicker)

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

	uiUnixNewControl(uiDateTimePicker, d);

	d->widget = gtk_label_new("TODO uiDateTimePicker not implemented");

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
