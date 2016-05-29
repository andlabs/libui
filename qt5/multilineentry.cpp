
#include "uipriv_qt5.hpp"

#include <QTextEdit>

struct uiMultilineEntry : public uiQt5Control {};

char *uiMultilineEntryText(uiMultilineEntry *e)
{
	if (auto textEdit = uiValidateAndCastObjTo<QTextEdit>(e)) {
		return uiQt5StrdupQString(textEdit->toPlainText());
	}
	return nullptr;
}

void uiMultilineEntrySetText(uiMultilineEntry *e, const char *text)
{
	if (auto textEdit = uiValidateAndCastObjTo<QTextEdit>(e)) {
		textEdit->setPlainText(QString::fromUtf8(text));
	}
}

void uiMultilineEntryAppend(uiMultilineEntry *e, const char *text)
{
	if (auto textEdit = uiValidateAndCastObjTo<QTextEdit>(e)) {
		// append is least interferring (keeps selection and cursor),
		// but will add newline by it self hence not exactly compatible

		// since the newline is implicitly inserted we need to remove
		// the explicitly inserted one if we want the examples to look
		// same, but this may cause other issues..
		auto oldBlockSignals = textEdit->blockSignals(true);
		textEdit->append(QString::fromUtf8(text).trimmed());
		textEdit->blockSignals(oldBlockSignals);
	}
}

void uiMultilineEntryOnChanged(uiMultilineEntry *e, void (*f)(uiMultilineEntry *e, void *data), void *data)
{
	if (auto textEdit = uiValidateAndCastObjTo<QTextEdit>(e)) {
		QObject::connect(textEdit, &QTextEdit::textChanged, textEdit, [f,e,data]{
			f(e,data);
		}, Qt::UniqueConnection);
	}
}

int uiMultilineEntryReadOnly(uiMultilineEntry *e)
{
	if (auto textEdit = uiValidateAndCastObjTo<QTextEdit>(e)) {
		textEdit->isReadOnly();
	}
	return 0;
}

void uiMultilineEntrySetReadOnly(uiMultilineEntry *e, int readonly)
{
	if (auto textEdit = uiValidateAndCastObjTo<QTextEdit>(e)) {
		textEdit->setReadOnly(readonly);
	}
}

uiMultilineEntry *uiNewMultilineEntry(void)
{
	auto textEdit = new QTextEdit;

	// note styling is being set in main.cpp -> styleSheet

	return uiAllocQt5ControlType(uiMultilineEntry,textEdit,uiQt5Control::DeleteControlOnQObjectFree);
}

uiMultilineEntry *uiNewNonWrappingMultilineEntry(void)
{
	auto textEdit = new QTextEdit;
	textEdit->setLineWrapMode(QTextEdit::NoWrap);

	// note styling is being set in main.cpp -> styleSheet

	return uiAllocQt5ControlType(uiMultilineEntry,textEdit,uiQt5Control::DeleteControlOnQObjectFree);
}
