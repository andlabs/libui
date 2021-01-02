
#include "uipriv_qt5.hpp"

//#include <QCalendarWidget> // TODO: for dropdown

#include <QDateEdit>
#include <QTimeEdit>
#include <QDateTimeEdit>

struct uiDateTimePicker : public uiQt5Control {};

uiDateTimePicker *uiNewDateTimePicker(void)
{
	auto dateEdit = new QDateEdit(QDate::currentDate());

	// note styling is being set in main.cpp -> styleSheet

	return uiAllocQt5ControlType(uiDateTimePicker,dateEdit,uiQt5Control::DeleteControlOnQObjectFree);
}

uiDateTimePicker *uiNewDatePicker(void)
{
	auto timeEdit = new QTimeEdit(QTime::currentTime());

	// note styling is being set in main.cpp -> styleSheet

	return uiAllocQt5ControlType(uiDateTimePicker,timeEdit,uiQt5Control::DeleteControlOnQObjectFree);
}

uiDateTimePicker *uiNewTimePicker(void)
{
	auto dateTimeEdit = new QDateTimeEdit(QDateTime::currentDateTime());

	// note styling is being set in main.cpp -> styleSheet

	return uiAllocQt5ControlType(uiDateTimePicker,dateTimeEdit,uiQt5Control::DeleteControlOnQObjectFree);
}
