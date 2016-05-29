
#include "uipriv_qt5.hpp"

#include <QProgressBar>

struct uiProgressBar : public uiQt5Control {};

void uiProgressBarSetValue(uiProgressBar *p, int value)
{
	if (auto progressBar = uiValidateAndCastObjTo<QProgressBar>(p)) {
		return progressBar->setValue(value);
	}
}

uiProgressBar *uiNewProgressBar(void)
{
	auto progressBar = new QProgressBar;

	// note styling is being set in main.cpp -> styleSheet

	return uiAllocQt5ControlType(uiProgressBar,progressBar,uiQt5Control::DeleteControlOnQObjectFree);
}
