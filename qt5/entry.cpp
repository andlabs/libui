
#include "uipriv_qt5.hpp"

#include <QLineEdit>

struct uiEntry : uiQt5Control {};

char *uiEntryText(uiEntry *e)
{
	if (auto lineEdit = uiValidateAndCastObjTo<QLineEdit>(e)) {
		return uiQt5StrdupQString(lineEdit->text());
	}
	return nullptr;
}

void uiEntrySetText(uiEntry *e, const char *text)
{
	if (auto lineEdit = uiValidateAndCastObjTo<QLineEdit>(e)) {
		lineEdit->setText(QString::fromUtf8(text));
	}
}

void uiEntryOnChanged(uiEntry *e, void (*f)(uiEntry *, void *), void *data)
{
	if (auto lineEdit = uiValidateAndCastObjTo<QLineEdit>(e)) {
		// Unlike textChanged(), this signal is not emitted when the text is changed programmatically
		QObject::connect(lineEdit, &QLineEdit::textChanged, lineEdit, [f,e,data]{
			f(e,data);
		}, Qt::UniqueConnection);
	}
}

int uiEntryReadOnly(uiEntry *e)
{
	if (auto lineEdit = uiValidateAndCastObjTo<QLineEdit>(e)) {
		return lineEdit->isReadOnly();
	}
	return false;
}

void uiEntrySetReadOnly(uiEntry *e, int readonly)
{
	if (auto lineEdit = uiValidateAndCastObjTo<QLineEdit>(e)) {
		lineEdit->setReadOnly(readonly);
	}
}

uiEntry *uiNewEntry(void)
{
	auto lineEdit = new QLineEdit;

	// note styling is being set in main.cpp -> styleSheet

	return uiAllocQt5ControlType(uiEntry,lineEdit,uiQt5Control::DeleteControlOnQObjectFree);
}
