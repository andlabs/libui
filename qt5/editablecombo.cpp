
#include "uipriv_qt5.hpp"

#include <QComboBox>

struct uiEditableCombobox : public uiQt5Control {};


void uiEditableComboboxAppend(uiEditableCombobox *c, const char *text)
{
	if (auto comboBox = uiValidateAndCastObjTo<QComboBox>(c)) {
		comboBox->addItem(QString::fromUtf8(text));
	}
}

intmax_t uiEditableComboboxSelected(uiEditableCombobox *c)
{
	if (auto comboBox = uiValidateAndCastObjTo<QComboBox>(c)) {
		return comboBox->currentIndex();
	}
	return -1;
}

void uiEditableComboboxSetSelected(uiEditableCombobox *c, intmax_t n)
{
	if (auto comboBox = uiValidateAndCastObjTo<QComboBox>(c)) {
		return comboBox->setCurrentIndex(n);
	}
}

char *uiEditableComboboxText(uiEditableCombobox *c)
{
	if (auto comboBox = uiValidateAndCastObjTo<QComboBox>(c)) {
		return uiQt5StrdupQString(comboBox->currentText());
	}
	return nullptr;
}

void uiEditableComboboxSetText(uiEditableCombobox *c, const char *text)
{
	if (auto comboBox = uiValidateAndCastObjTo<QComboBox>(c)) {
		comboBox->setCurrentText(QString::fromUtf8(text));
	}
}

void uiEditableComboboxOnChanged(uiEditableCombobox *c, void (*f)(uiEditableCombobox *c, void *data), void *data)
{
	if (auto comboBox = uiValidateAndCastObjTo<QComboBox>(c)) {
		// disambiguation of overloaded function
		void (QComboBox:: *currentIndexChanged)(const QString &) = &QComboBox::currentIndexChanged;
		QObject::connect(comboBox, currentIndexChanged, comboBox, [f,c,data]{
			f(c,data);
		}, Qt::UniqueConnection);
	}
}

uiEditableCombobox *uiNewEditableCombobox(void)
{
	auto comboBox = new QComboBox;
	comboBox->setEditable(true);

	// note styling is being set in main.cpp -> styleSheet

	return uiAllocQt5ControlType(uiEditableCombobox,comboBox,uiQt5Control::DeleteControlOnQObjectFree);
}
