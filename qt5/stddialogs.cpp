
#include "uipriv_qt5.hpp"

#include <QFileDialog>
#include <QMessageBox>

char *uiOpenFile(uiWindow *parent)
{
	return uiQt5StrdupQString(QFileDialog::getOpenFileName(uiValidateAndCastObjTo<QWidget>(parent)));
}

char *uiSaveFile(uiWindow *parent)
{
	return uiQt5StrdupQString(QFileDialog::getSaveFileName(uiValidateAndCastObjTo<QWidget>(parent)));
}

void uiMsgBox(uiWindow *parent, const char *title, const char *description)
{
	QMessageBox::information(uiValidateAndCastObjTo<QWidget>(parent),
							 QString::fromUtf8(title),
							 QString::fromUtf8(description));
}

void uiMsgBoxError(uiWindow *parent, const char *title, const char *description)
{
	QMessageBox::warning(uiValidateAndCastObjTo<QWidget>(parent),
						 QString::fromUtf8(title),
						 QString::fromUtf8(description));
}
