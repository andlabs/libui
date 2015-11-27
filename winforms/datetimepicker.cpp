// 26 november 2015
#include "uipriv_winforms.hpp"

// TODO TODO TODO AVAILABLE SINCE 4.0 TODO TODO TODO

struct uiDateTimePicker {
	uiWindowsControl c;
//	gcroot<DatePicker ^> *datePicker;
	DUMMY dummy;
};

uiWindowsDefineControl(
	uiDateTimePicker,						// type name
	uiDateTimePickerType,					// type function
//	datePicker							// handle
	dummy								// handle
)

static uiDateTimePicker *finishNewDateTimePicker(/* TODO */)
{
	uiDateTimePicker *d;

	d = (uiDateTimePicker *) uiNewControl(uiDateTimePickerType());

/*	d->datePicker = new gcroot<DatePicker ^>();
	*(d->datePicker) = gcnew DatePicker();
	// TODO SelectedDateFormat
*/
	d->dummy = mkdummy(L"uiDatePicker");

	uiWindowsFinishNewControl(d, uiDateTimePicker, dummy);//datePicker);

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
