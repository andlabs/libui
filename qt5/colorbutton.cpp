#include "uipriv_qt5.hpp"

#include <QColorDialog>
#include <QPushButton>

struct uiColorButton : public uiQt5Control {};

static QColorDialog *findColorDialog(uiColorButton *b)
{
	if (auto pushButton = uiValidateAndCastObjTo<QPushButton>(b)) {
		if (auto colorDialog = pushButton->findChild<QColorDialog*>()) {
			return colorDialog;
		}
	}
	qWarning("colorDialog not found?!");
	return nullptr;
}

void uiColorButtonColor(uiColorButton *b, double *r, double *g, double *bl, double *a)
{
	if (auto colorDialog = findColorDialog(b)) {
		auto color = colorDialog->currentColor();
		*r = color.redF();
		*g = color.greenF();
		*bl = color.blueF();
		*a = color.alphaF();
	}
}

void uiColorButtonSetColor(uiColorButton *b, double r, double g, double bl, double a)
{
	if (auto colorDialog = findColorDialog(b)) {
		auto color = QColor::fromRgbF(r,g,bl,a);
		colorDialog->setCurrentColor(color);
		emit colorDialog->colorSelected(color); // hacky?
	}
}

void uiColorButtonOnChanged(uiColorButton *b, void (*f)(uiColorButton *, void *), void *data)
{
	if (auto colorDialog = findColorDialog(b)) {
		QObject::connect(colorDialog, &QColorDialog::colorSelected, colorDialog, [f,b,data]{
			f(b,data);
		}, Qt::UniqueConnection);
	}
}

uiColorButton *uiNewColorButton(void)
{
	auto pushButton = new QPushButton;
	pushButton->setObjectName("colorButton"); // so we can target just this button with stylesheet

	// persisting this dialog in the background simplies things, but not a good partice
	auto colorDialog = new QColorDialog(pushButton);

	auto updateColor = [pushButton](const QColor &color) {
		// quick and dirty, probably not ideal, doesn't show transperency in anyway
		auto qssColorString = QStringLiteral("rgb(%1,%2,%3)").arg(color.red()).arg(color.green()).arg(color.blue());
		pushButton->setStyleSheet(QStringLiteral("#colorButton {background-color: %1;}").arg(qssColorString));
	};
	QObject::connect(colorDialog, &QColorDialog::colorSelected, pushButton, updateColor);
	updateColor({}); // set initial color (black)

	QObject::connect(pushButton, &QPushButton::clicked, colorDialog, &QColorDialog::show);

	// note styling is being set in main.cpp -> styleSheet

	return uiAllocQt5ControlType(uiColorButton,pushButton,uiQt5Control::DeleteControlOnQObjectFree);
}
