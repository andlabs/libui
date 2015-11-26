// 26 november 2015
#include "uipriv_wpf.hpp"

struct uiDateTimePicker {
	uiWindowsControl c;
	DUMMY dummy;
};

uiWindowsDefineControl(
	uiDateTimePicker,						// type name
	uiDateTimePickerType,					// type function
	dummy								// handle
)

static uiDateTimePicker *finishNewDateTimePicker(void)
{
	uiDateTimePicker *d;

	d = (uiDateTimePicker *) uiNewControl(uiDateTimePickerType());

	d->dummy = mkdummy(L"uiDateTimePicker");

	uiWindowsFinishNewControl(d, uiDateTimePicker, dummy);

	return d;
}

uiDateTimePicker *uiNewDateTimePicker(void)
{
	return finishNewDateTimePicker();
}

uiDateTimePicker *uiNewDatePicker(void)
{
	return finishNewDateTimePicker();
}

uiDateTimePicker *uiNewTimePicker(void)
{
	return finishNewDateTimePicker();
}
