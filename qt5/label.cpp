
#include "uipriv_qt5.hpp"

#include <QLabel>

struct uiLabel : public uiQt5Control {};

char *uiLabelText(uiLabel *l)
{
	if (auto label = uiValidateAndCastObjTo<QLabel>(l)) {
		return uiQt5StrdupQString(label->text());
	}
	return nullptr;
}

void uiLabelSetText(uiLabel *l, const char *text)
{
	if (auto label = uiValidateAndCastObjTo<QLabel>(l)) {
		label->setText(QString::fromUtf8(text));
	}
}

uiLabel *uiNewLabel(const char *text)
{
	auto label = new QLabel(QString::fromUtf8(text));

	// note styling is being set in main.cpp -> styleSheet

	return uiAllocQt5ControlType(uiLabel,label,uiQt5Control::DeleteControlOnQObjectFree);
}
