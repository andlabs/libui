
#include "uipriv_qt5.hpp"

#include <QSpinBox>

struct uiSpinbox : public uiQt5Control {};

intmax_t uiSpinboxValue(uiSpinbox *s)
{
	if (auto spinBox = uiValidateAndCastObjTo<QSpinBox>(s)) {
		return spinBox->value();
	}
	return 0;
}

void uiSpinboxSetValue(uiSpinbox *s, intmax_t value)
{
	if (auto spinBox = uiValidateAndCastObjTo<QSpinBox>(s)) {
		spinBox->setValue(value);
	}
}

void uiSpinboxOnChanged(uiSpinbox *s, void (*f)(uiSpinbox *, void *), void *data)
{
	if (auto spinBox = uiValidateAndCastObjTo<QSpinBox>(s)) {
		QObject::connect(spinBox, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), spinBox, [f,s,data]{
			f(s,data);
		}, Qt::UniqueConnection);
	}
}

uiSpinbox *uiNewSpinbox(intmax_t min, intmax_t max)
{
	auto spinBox = new QSpinBox;
	spinBox->setRange(qMin(min,max),qMax(min,max));

	// note styling is being set in main.cpp -> styleSheet

	return uiAllocQt5ControlType(uiSpinbox,spinBox,uiQt5Control::DeleteControlOnQObjectFree);
}
