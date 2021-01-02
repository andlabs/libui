
#include "uipriv_qt5.hpp"

#include <QPushButton>

struct uiButton : public uiQt5Control {};

char *uiButtonText(uiButton *b)
{
	if (auto button = uiValidateAndCastObjTo<QPushButton>(b)) {
		return uiQt5StrdupQString(button->text());
	}
	return nullptr;
}

void uiButtonSetText(uiButton *b, const char *text)
{
	if (auto button = uiValidateAndCastObjTo<QPushButton>(b)) {
		button->setText(QString::fromUtf8(text));
	}
}

void uiButtonOnClicked(uiButton *b, void (*f)(uiButton *, void *), void *data)
{
	if (auto button = uiValidateAndCastObjTo<QPushButton>(b)) {
		QObject::connect(button, &QPushButton::clicked, button, [f,b,data]{
			f(b,data);
		}, Qt::UniqueConnection);
	}
}

uiButton *uiNewButton(const char *text)
{
	auto button = new QPushButton(QString::fromUtf8(text));

	// note styling is being set in main.cpp -> styleSheet

	return uiAllocQt5ControlType(uiButton,button,uiQt5Control::DeleteControlOnQObjectFree);
}
