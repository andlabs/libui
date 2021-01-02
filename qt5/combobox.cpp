
#include "uipriv_qt5.hpp"

#include <QComboBox>

struct uiCombobox : public uiQt5Control {};

void uiComboboxAppend(uiCombobox *c, const char *text)
{
	if (auto comboBox = uiValidateAndCastObjTo<QComboBox>(c)) {
		comboBox->addItem(QString::fromUtf8(text));
	}
}

intmax_t uiComboboxSelected(uiCombobox *c)
{
	if (auto comboBox = uiValidateAndCastObjTo<QComboBox>(c)) {
		return comboBox->currentIndex();
	}
	return -1;
}

void uiComboboxSetSelected(uiCombobox *c, intmax_t n)
{
	if (auto comboBox = uiValidateAndCastObjTo<QComboBox>(c)) {
		comboBox->setCurrentIndex(n);
	}
}

void uiComboboxOnSelected(uiCombobox *c, void (*f)(uiCombobox *c, void *data), void *data)
{
	if (auto comboBox = uiValidateAndCastObjTo<QComboBox>(c)) {
		// disambiguation of overloaded function
		void (QComboBox:: *currentIndexChanged)(int) = &QComboBox::currentIndexChanged;
		QObject::connect(comboBox, currentIndexChanged, comboBox, [f,c,data]{
			f(c,data);
		}, Qt::UniqueConnection);
	}
}

uiCombobox *uiNewCombobox(void)
{
	auto comboBox = new QComboBox;

	// note styling is being set in main.cpp -> styleSheet

	return uiAllocQt5ControlType(uiCombobox,comboBox,uiQt5Control::DeleteControlOnQObjectFree);
}
