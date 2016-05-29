
#include "uipriv_qt5.hpp"

#include <QCheckBox>

struct uiCheckbox : public uiQt5Control {};

char *uiCheckboxText(uiCheckbox *c)
{
	if (auto checkBox = uiValidateAndCastObjTo<QCheckBox>(c)) {
		return uiQt5StrdupQString(checkBox->text());
	}
	return nullptr;
}

void uiCheckboxSetText(uiCheckbox *c, const char *text)
{
	if (auto checkBox = uiValidateAndCastObjTo<QCheckBox>(c)) {
		checkBox->setText(QString::fromUtf8(text));
	}
}

void uiCheckboxOnToggled(uiCheckbox *c, void (*f)(uiCheckbox *, void *), void *data)
{
	if (auto checkBox = uiValidateAndCastObjTo<QCheckBox>(c)) {
		QObject::connect(checkBox, &QCheckBox::toggled, checkBox, [f,c,data]{
			f(c,data);
		}, Qt::UniqueConnection);
	}
}

int uiCheckboxChecked(uiCheckbox *c)
{
	if (auto checkBox = uiValidateAndCastObjTo<QCheckBox>(c)) {
		return checkBox->isChecked();
	}
	return 0;
}

void uiCheckboxSetChecked(uiCheckbox *c, int checked)
{
	if (auto checkBox = uiValidateAndCastObjTo<QCheckBox>(c)) {
		checkBox->setChecked(checked);
	}
}

uiCheckbox *uiNewCheckbox(const char *text)
{
	auto checkBox = new QCheckBox(QString::fromUtf8(text));

	// note styling is being set in main.cpp -> styleSheet

	return uiAllocQt5ControlType(uiCheckbox,checkBox,uiQt5Control::DeleteControlOnQObjectFree);
}
