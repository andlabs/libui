
#include "uipriv_qt5.hpp"

#include <QRadioButton>
#include <QVBoxLayout>

struct uiRadioButtons : public uiQt5Control {};

void uiRadioButtonsAppend(uiRadioButtons *r, const char *text)
{
	if (auto layout = uiValidateAndCastObjTo<QLayout>(r)) {
		auto radioButton = new QRadioButton(QString::fromUtf8(text));
		layout->addWidget(radioButton);
		if (layout->count() == 1) {
			radioButton->setChecked(true);
		}
	}
}

uiRadioButtons *uiNewRadioButtons(void)
{
	// TODO: check does this need a QButtonGroup or is the layout sufficent?
	auto layout = new QVBoxLayout;

	// note styling is being set in main.cpp -> styleSheet

	return uiAllocQt5ControlType(uiRadioButtons,layout,uiQt5Control::DeleteControlOnQObjectFree);
}
