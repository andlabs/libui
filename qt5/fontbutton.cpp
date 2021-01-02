
#include "uipriv_qt5.hpp"

#include "draw.hpp"

// another way would be to return a QFontComboBox + QSpinBox (for font size),
// but this is more true the control name
#include <QFontDialog>
#include <QPushButton>

struct uiFontButton : public uiQt5Control {};

uiDrawTextFont *uiFontButtonFont(uiFontButton *b)
{
	if (auto pushButton = uiValidateAndCastObjTo<QPushButton>(b)) {
		if (auto fontDialog = pushButton->findChild<QFontDialog*>()) {
			return new uiDrawTextFont(fontDialog->currentFont());
		}
	}
	return nullptr;
}

void uiFontButtonOnChanged(uiFontButton *b, void (*f)(uiFontButton *, void *), void *data)
{
	if (auto pushButton = uiValidateAndCastObjTo<QPushButton>(b)) {
		if (auto fontDialog = pushButton->findChild<QFontDialog*>()) {
			QObject::connect(fontDialog, &QFontDialog::fontSelected, fontDialog, [f,b,data]{
				f(b,data);
			}, Qt::UniqueConnection);
		}
	}
}

uiFontButton *uiNewFontButton(void)
{
	auto pushButton = new QPushButton;

	// persisting this dialog in the background simplies things, but not a good partice
	auto fontDialog = new QFontDialog(pushButton);

	auto updateFont = [pushButton](const QFont &font) {
		auto family = font.family();
		auto pointSize = font.pointSize(); // or pixelSize..?
		pushButton->setText(QStringLiteral("%1  %2").arg(family).arg(pointSize));
	};
	QObject::connect(fontDialog, &QFontDialog::fontSelected, pushButton, updateFont);
	QObject::connect(pushButton, &QPushButton::clicked, fontDialog, &QFontDialog::show);

	QFont font; font.setPointSize(12);
	updateFont(font);
	fontDialog->setCurrentFont(font); // set initial font (system font)

	// note styling is being set in main.cpp -> styleSheet

	return uiAllocQt5ControlType(uiFontButton,pushButton,uiQt5Control::DeleteControlOnQObjectFree);
}
