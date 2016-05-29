
#include "uipriv_qt5.hpp"

#include <QFrame>

struct uiSeparator : public uiQt5Control {};

uiSeparator *uiNewHorizontalSeparator(void)
{
	auto frame = new QFrame;
	frame->setFrameStyle(QFrame::HLine | QFrame::Sunken);

	// note styling is being set in main.cpp -> styleSheet

	return uiAllocQt5ControlType(uiSeparator,frame,uiQt5Control::DeleteControlOnQObjectFree);
}
